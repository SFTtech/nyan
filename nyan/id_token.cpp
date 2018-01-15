// Copyright 2017-2018 the nyan authors, LGPLv3+. See copying.md for legal info.
#include "id_token.h"

#include <string>

#include "ast.h"
#include "compiler.h"
#include "error.h"
#include "location.h"

namespace nyan {


IDToken::IDToken(const Token &first,
                 TokenStream &tokens) {

	this->ids.push_back(first);

	auto token = tokens.next();
	while (token->type == token_type::DOT) {
		token = tokens.next();
		if (unlikely(token->type != token_type::ID)) {
			throw ASTError{"expected identifier after a dot, encountered", *token};
		}
		this->ids.push_back(*token);
		token = tokens.next();
	}

	tokens.reinsert(token);
}


std::string IDToken::str() const {
	return util::strjoin(
		".",
		this->ids,
		[] (const auto &in) {
			return in.get();
		}
	);
}


bool IDToken::exists() const {
	return this->ids.size() > 0;
}


token_type IDToken::get_type() const {
	if (unlikely(not this->exists())) {
		return token_type::INVALID;
	}
	else {
		return this->ids.at(0).type;
	}
}


const Location &IDToken::get_start_location() const {
	if (unlikely(not this->exists())) {
		throw InternalError{
			"this IDToken doesn't exist, but you queried its location"
		};
	}

	return this->ids.at(0).location;
}


size_t IDToken::get_length() const {
	if (not this->exists()) {
		return 0;
	}

	size_t len = 0;
	for (auto &tok : this->ids) {
		// there's separating . in between each id
		len += tok.location.get_length() + 1;
	}

	// there's no trailing . in an id
	len -= 1;

	return len;
}


const std::vector<Token> &IDToken::get_components() const {
	return this->ids;
}


const std::string &IDToken::get_first() const {
	if (unlikely(not this->exists())) {
		throw InternalError{"element of non-existing IDToken requested"};
	}

	return this->ids[0].get();
}

} // namespace nyan
