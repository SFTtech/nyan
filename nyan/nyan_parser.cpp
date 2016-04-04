// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_parser.h"

#include <iostream>
#include <sstream>

#include "nyan_ast.h"
#include "nyan_lexer.h"
#include "nyan_flex_lexer.h"
#include "nyan_token.h"
#include "nyan_util.h"


namespace nyan {

NyanParser::NyanParser(NyanStore *store)
	:
	store{store} {}


std::vector<NyanObject> NyanParser::parse(const std::string &input) {
	// If you are some parser junkie and I trigger your rage mode now,
	// feel free to rewrite the parser or a tool like bison.

	// tokenize input
	std::vector<NyanToken> tokens = this->tokenize(input);

	// create ast from tokens
	NyanAST root = this->create_ast(tokens);

	// create objects from tokens
	std::vector<NyanObject> ret = this->create_objects(root);

	return ret;
}


std::vector<NyanToken> NyanParser::tokenize(const std::string &input) {
	std::stringstream reader{input};
	NyanLexer lexer{&reader};

	std::vector<NyanToken> ret;

	while (true) {
		NyanToken token = lexer.get_next_token();
		bool end = (token.type == token_type::ENDFILE);

		ret.push_back(std::move(token));

		if (end) {
			break;
		}
	}

	return ret;
}


NyanAST NyanParser::create_ast(const std::vector<NyanToken> &tokens) {
	util::Iterator<NyanToken> token_iter{tokens};
	NyanAST root{token_iter};
	return root;
}


std::vector<NyanObject> NyanParser::create_objects(const NyanAST &ast) {
	std::vector<NyanObject> ret;

	for (auto &obj : ast.get_objects()) {
		std::cout << obj.str() << std::endl;
	}

	return ret;
}


} // namespace nyan
