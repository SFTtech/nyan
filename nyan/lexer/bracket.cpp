// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "bracket.h"


namespace nyan::lexer {


Bracket::Bracket(token_type ttype, int indent)
    :
    indentation{indent},
    type{this->to_type(ttype)},
    hanging{true} {}


void Bracket::doesnt_hang(int new_indent) {
    this->hanging = false;
    this->indentation = new_indent;
}


bool Bracket::is_hanging() const {
    return this->hanging;
}


bool Bracket::matches(token_type type) const {
    return type == this->expected_match();
}


int Bracket::get_content_indent() const {
    if (this->is_hanging()) {
        // hanging brackets store their expected indent level
        return this->indentation;
    }
    else {
        // wrapped brackets store their base indent level
        return this->indentation + SPACES_PER_INDENT;
    }
}


bool Bracket::closing_indent_ok(int indent) const {
    if (this->is_hanging()) {
        // hanging indent requires the closing bracket to be
        // closed after the opening bracket column
        return this->indentation <= indent;
    }
    else {
        return this->indentation == indent;
    }
}


std::string Bracket::get_closing_indent() const {
    if (this->is_hanging()) {
        std::ostringstream builder;
        builder << "at least "
                << this->indentation;
        return builder.str();
    }
    else {
        return std::to_string(this->indentation);
    }
}


const char *Bracket::matching_type_str() const {
    return token_type_str(this->expected_match());
}


bracket_type Bracket::to_type(token_type token) {
    switch (token) {
    case token_type::LPAREN:
    case token_type::RPAREN:
        return bracket_type::PAREN;
    case token_type::LANGLE:
    case token_type::RANGLE:
        return bracket_type::ANGLE;
    case token_type::LBRACKET:
    case token_type::RBRACKET:
        return bracket_type::BRACKET;
    case token_type::LBRACE:
    case token_type::RBRACE:
        return bracket_type::BRACE;
    default:
        throw InternalError{"tried to convert non-bracket token to bracket"};
    }
}


token_type Bracket::expected_match() const {
    switch (this->type) {
    case bracket_type::PAREN:
        return token_type::RPAREN;
    case bracket_type::ANGLE:
        return token_type::RANGLE;
    case bracket_type::BRACKET:
        return token_type::RBRACKET;
    case bracket_type::BRACE:
        return token_type::RBRACE;
    default:
        throw InternalError{"unknown bracket type"};
    }
}


} // namespace nyan::lexer
