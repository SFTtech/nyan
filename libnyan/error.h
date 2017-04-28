// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_ERROR_H_
#define NYAN_NYAN_ERROR_H_

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "location.h"

namespace nyan {


/**
 * A single symbol, as determined from a program counter, and returned by
 * Backtrace::get_symbols().
 */
struct backtrace_symbol {
	std::string functionname;  // empty if unknown
	void *pc;                  // nullptr if unknown
};


/**
 * Provide execution backtrace information through get_symbols().
 */
class Backtrace {
public:
	Backtrace() = default;

	virtual ~Backtrace() = default;

	/**
	 * Analyzes the current stack, and stores the program counter values in
	 * this->stack_addrs.
	 */
	void analyze();

	/**
	 * Provide the names for all stack frames via the callback.
	 *
	 * The most recent call is returned last (alike Python).
	 *
	 * @param cb
	 *    is called for every symbol in the backtrace,
	 *    starting with the top-most frame.
	 * @param reversed
	 *    if true, the most recent call is given last.
	 */
	void get_symbols(std::function<void (const backtrace_symbol *)> cb,
	                 bool reversed=true) const;

	/**
	 * Removes all the lower frames that are also present
	 * in the current stack.
	 *
	 * Designed to be used in catch clauses,
	 * to simulate stack trace collection
	 * from throw to catch, instead of from throw to the process entry point.
	 */
	void trim_to_current_stack_frame();

protected:
	/**
	 * All program counters of this backtrace.
	 */
	std::vector<void *> stack_addrs;
};


/**
 * Base exception for every error that occurs in nyan.
 */
class Error : public std::runtime_error {
public:
	Error(const std::string &msg,
	          bool generate_backtrace=true,
	          bool store_cause=true);
	virtual ~Error() = default;

	/**
	 * String representation of this exception, as
	 * specialized by a child exception.
	 */
	virtual std::string str() const;

	/**
	 * Returns the message's content.
	 */
	const char *what() const noexcept override;

	/**
	 * Stores a pointer to the currently-handled exception in this->cause.
	 */
	void store_cause();

	/**
	 * Calls this->backtrace->trim_to_current_stack_frame(),
	 * if this->backtrace is not nullptr.
	 *
	 * Designed to be used in catch clauses, to strip away all those
	 * unneeded symbols from program init upwards.
	 */
	void trim_backtrace();

	/**
	 * Re-throws the exception cause, if the exception has one.
	 * Otherwise, does nothing.
	 *
	 * Use this when handling the exception, to handle the cause.
	 */
	void rethrow_cause() const;

	/**
	 * Get the type name of of the exception.
	 * Use it to display the name of a child exception.
	 */
	virtual std::string type_name() const;

	/**
	 * Return the backtrace where the exception was thrown.
	 * nullptr if no backtrace was collected.
	 */
	Backtrace *get_backtrace() const;

	/**
	 * Directly return the message stored in the exception.
	 */
	const std::string &get_msg() const;

protected:
	/**
	 * The (optional) backtrace where the exception came from.
	 */
	std::shared_ptr<Backtrace> backtrace;

	/**
	 * The error message text.
	 */
	std::string msg;

	/**
	 * Re-throw this with rethrow_cause().
	 */
	std::exception_ptr cause;
};


/**
 * Output stream concat for nyanerrors.
 */
std::ostream &operator <<(std::ostream &os, const Error &e);


/**
 * Output stream concat for backtrace symbols.
 */
std::ostream &operator <<(std::ostream &os, const backtrace_symbol &bt_sym);


/**
 * Output stream concat for backtraces.
 */
std::ostream &operator <<(std::ostream &os, const Backtrace &bt);


/**
 * Internal Error, thrown when some interal sanity check failed.
 */
class InternalError : public Error {
public:
	InternalError(const std::string &msg);
};


/**
 * Exception class to capture problems with files,
 * for that, it stores line number and line offset.
 */
class FileError : public Error {
public:
	FileError(const Location &location, const std::string &msg);

	std::string str() const override;

	virtual std::string show_problem_origin() const;

protected:
	Location location;
};



/**
 * Exception for name access problems.
 */
class NameError : public FileError {
public:
	NameError(const Location &location,
	          const std::string &msg, const std::string &name="");

	std::string str() const override;

protected:
	std::string name;
};

} // namespace nyan

#endif
