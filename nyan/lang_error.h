// Copyright 2019-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
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

    /**
     * String representation of this error.
     *
     * @return String representation of this error.
     */
    std::string str() const override;

    /**
     * Get a string that visualizes the error in the output using
     * the location and reasons for the error.
     *
     * @return String containing a pretty error message.
     */
    virtual std::string show_problem_origin() const;

protected:

    /**
     * Location of the error in a file.
     */
    Location location;

    /**
     * Map of reasons for the error by location in the file.
     */
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
 * Exception for name access problems and naming conflicts.
 */
class NameError : public LangError {
public:
    NameError(const Location &location,
              const std::string &msg, const std::string &name="");

    /**
     * String representation of this error.
     *
     * @return String representation of this error.
     */
    std::string str() const override;

protected:

    /**
     * Name that the entitycausing this error conflicts with.
     */
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
