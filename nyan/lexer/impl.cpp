// Copyright 2017-2018 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "impl.h"

#define YY_NO_UNISTD_H
#include "flex.gen.h"

#include "../compiler.h"
#include "../file.h"

namespace nyan::lexer {

Impl::Impl(const std::shared_ptr<File> &file)
    :
    file{file},
    input{file->get_content()} {

    yylex_init_extra(this, &this->scanner);
}

Impl::~Impl() {
    if (this->scanner) {
        yylex_destroy(this->scanner);
    }
}

/*
 * Generate tokens until the queue has on available to return.
 * Return tokens from the queue until it's empty.
 */
Token Impl::generate_token() {
    if (this->tokens.empty()) {
        yylex(this->scanner);
    }

    if (not this->tokens.empty()) {
        auto ret = this->tokens.front();
        this->tokens.pop();
        return ret;
    }

    // if generate_token did not generate a token:
    throw this->error("internal error.");
}


/*
 * Fetch the current lexer state and throw an error.
 */
TokenizeError Impl::error(const std::string &msg) {
    return TokenizeError{
        Location{
            this->file,
            yyget_lineno(this->scanner),
            this->linepos - static_cast<int>(yyget_leng(this->scanner)),
            static_cast<int>(yyget_leng(this->scanner))
        },
        msg
    };
}

void Impl::advance_linepos() {
    this->linepos += yyget_leng(this->scanner);
}

int Impl::read_input(char *buffer, int max_size) {
    if (unlikely(max_size <= 0)) {
        return 0;
    }

    this->input.read(buffer, static_cast<std::streamsize>(max_size));
    return static_cast<int>(this->input.gcount());
}

void Impl::endline() {
    // ENDLINE is not an acceptable first token.
    // Optimize for consecutive ENDLINE tokens: keep only one.
    if (not tokens.empty() and tokens.back().type != token_type::ENDLINE) {
        this->token(token_type::ENDLINE);
    }
    // Reset the line position to the beginning.
    this->linepos = linepos_start;
}

/*
 * Fetch the current lexer state variables and create a token.
 */
void Impl::token(token_type type) {
    int length = yyget_leng(this->scanner);
    int token_start = this->linepos - length;
    int lineno = yyget_lineno(this->scanner);
    if (type == token_type::ENDLINE) {
        /* don't assign the `\n` for the next line */
        --lineno;
    }

    // to register open and close parenthesis
    // for correct line-wrap-indentation.
    this->track_brackets(type, token_start);

    if (token_needs_payload(type)) {
        this->tokens.push(Token{
            this->file,
            lineno,
            token_start,
            length,
            type,
            yyget_text(this->scanner)
        });
    }
    else {
        this->tokens.push(Token{
            this->file,
            lineno,
            token_start,
            length,
            type
        });
    }
}

/*
 * Remember where the current open bracket is
 * so that the indentation can check if the depth is correct.
 */
void Impl::track_brackets(token_type type, int token_start) {

    // opening brackets
    if (type == token_type::LPAREN or
        type == token_type::LANGLE or
        type == token_type::LBRACKET or
        type == token_type::LBRACE) {

        // Track bracket type and indentation.
        // The position after the ( is exactly the expected indent
        // for hanging brackets.
        this->brackets.emplace(
            type,
            token_start + 1
        );

        this->possibly_hanging = true;
        return;
    }
    // closing brackets
    else if (type == token_type::RPAREN or
             type == token_type::RANGLE or
             type == token_type::RBRACKET or
             type == token_type::RBRACE) {

        if (this->brackets.empty()) {
            throw this->error("unexpected closing bracket, "
                              "as no opening one is known");
        }

        Bracket &matching_open_bracket = this->brackets.top();

        // test if bracket actually matches
        if (not matching_open_bracket.matches(type)) {
            std::ostringstream builder;
            builder << "non-matching bracket: expected '"
                    << matching_open_bracket.matching_type_str()
                    << "' but got '" << token_type_str(type) << "'";
            throw this->error(builder.str());
        }

        if (not matching_open_bracket.closing_indent_ok(token_start)) {
            std::ostringstream builder;
            builder << "wrong indentation of bracket: expected "
                    << matching_open_bracket.get_closing_indent()
                    << " indentation spaces (it is currently at "
                    << token_start << " spaces)";
            throw this->error(builder.str());
        }

        this->bracketcloseindent_expected = false;
        this->brackets.pop();
    }
    // newline directly after opening bracket
    // means regular indentation has to follow
    // and the bracket pair doesn't hang.
    else if (not this->brackets.empty() and
             this->possibly_hanging and
             type == token_type::ENDLINE) {

        // the bracket is followed by a newline directly,
        // thus is not hanging.
        this->brackets.top().doesnt_hang(
            this->previous_indent
        );
    }
    else if (not this->brackets.empty() and
             this->bracketcloseindent_expected) {
        std::ostringstream builder;
        builder << ("expected closing bracket or content "
                    "at indentation with ")
                << this->brackets.top().get_content_indent()
                << " spaces (you start at " << token_start << " spaces)";
        throw this->error(builder.str());
    }

    this->possibly_hanging = false;
}

/*
 * measure the indentation of a line
 */
void Impl::handle_indent(int depth) {

    this->linepos -= yyget_leng(this->scanner) - depth;

    if (not this->brackets.empty()) {
        // we're in a pair of brackets,
        // there the indentation is way funnier.

        // check if the content indentation is correct.
        int expected = this->brackets.top().get_content_indent();
        if (depth != expected) {
            // if the expected depth is not correct,
            // then the only thing that is allowed is
            // the closing bracket.
            // the check will be done for the next token in
            // `track_brackets`.
            this->bracketcloseindent_expected = true;
        }

        // don't need to track the indent stack,
        // this is done in the bracket tracking now.
        return;
    }

    // regular indent is enforced when not in a bracket pair
    if ((depth % SPACES_PER_INDENT) > 0) {
        std::ostringstream builder;
        builder << "indentation requires exactly "
                << SPACES_PER_INDENT
                << " spaces per level";
        throw this->error(builder.str());
    }

    if (depth == this->previous_indent) {
        // same indent level, ignore
        return;
    }
    else if (depth < this->previous_indent) {
        // current line is further left than the previous one
        int delta = this->previous_indent - depth;
        while (delta > 0) {
            delta -= SPACES_PER_INDENT;
            this->token(token_type::DEDENT);
        }
    }
    else {
        // current line has more depth than the previous one
        int delta = depth - this->previous_indent;
        while (delta > 0) {
            delta -= SPACES_PER_INDENT;
            this->token(token_type::INDENT);
        }
    }
    this->previous_indent = depth;
}

} // namespace nyan::lexer
