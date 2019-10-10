// Copyright 2019-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include "error.h"
#include "location.h"


namespace nyan {


/**
 * Exception class to capture problems with files,
 * for that, it stores line number and line offset.
 */
class LangError : public Error {
public:
	LangError(const Location &location, const std::string &msg,
	          std::vector<std::pair<Location, std::string>> &&reasons={});

	std::string str() const override;

	virtual std::string show_problem_origin() const;

protected:
	Location location;
	std::vector<std::pair<Location, std::string>> reasons;
};


/**
 * Thrown when encountering type problems in nyan code.
 */
class TypeError : public LangError {
public:
	TypeError(const Location &location, const std::string &msg);

	virtual ~TypeError() = default;
};


/**
 * Exception for name access problems.
 */
class NameError : public LangError {
public:
	NameError(const Location &location,
	          const std::string &msg, const std::string &name="");

	std::string str() const override;

protected:
	std::string name;
};


/**
 * Tokenize failure exception.
 */
class TokenizeError : public LangError {
public:
	TokenizeError(const Location &location,
	              const std::string &msg);
};

} // namespace nyan
