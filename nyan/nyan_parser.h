// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_PARSER_H_
#define NYAN_PARSER_H_

#include <vector>

#include "nyan_ast.h"
#include "nyan_object.h"
#include "nyan_token.h"

namespace nyan {

class NyanStore;

/**
 * The parser for nyan.
 */
class NyanParser {
public:
	NyanParser(NyanStore *store);
	virtual ~NyanParser() = default;

	std::vector<NyanObject> parse(const std::string &input);

protected:
	std::vector<NyanToken> tokenize(const std::string &input);
	NyanAST create_ast(const std::vector<NyanToken> &tokens);
	std::vector<NyanObject> create_objects(const NyanAST &ast);

	NyanStore *store;
};

} // namespace nyan

#endif
