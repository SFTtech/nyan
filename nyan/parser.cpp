// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "parser.h"

#include "ast.h"
#include "compiler.h"
#include "database.h"
#include "file.h"
#include "lexer/lexer.h"
#include "member.h"
#include "object.h"
#include "token.h"
#include "type.h"
#include "util.h"
#include "value/value.h"
#include "value/number.h"
#include "value/orderedset.h"
#include "value/set.h"
#include "value/text.h"


namespace nyan {

Parser::Parser() {}


AST Parser::parse(const std::shared_ptr<File> &file) {
	// If you are some parser junkie and I trigger your rage mode now,
	// feel free to rewrite the parser or use a tool like bison.

	// tokenize input
	std::vector<Token> tokens = this->tokenize(file);

	// create ast from tokens
	AST ast = this->create_ast(tokens);

	return ast;
}


std::vector<Token> Parser::tokenize(const std::shared_ptr<File> &file) const {
	Lexer lexer{file};

	std::vector<Token> ret;

	while (true) {
		Token token = lexer.get_next_token();
		bool end = (token.type == token_type::ENDFILE);

		ret.push_back(std::move(token));

		if (end) {
			break;
		}
	}

	return ret;
}


AST Parser::create_ast(const std::vector<Token> &tokens) const {
	TokenStream token_iter{tokens};
	AST root{token_iter};
	return root;
}

} // namespace nyan
