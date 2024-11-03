// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "config.h"


namespace nyan {

/**
 * A single symbol, as determined from a program counter, and returned by
 * Backtrace::get_symbols().
 */
struct backtrace_symbol {
	std::string functionname; // empty if unknown
	void *pc; // nullptr if unknown
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
	 *	  if true, the most recent call is given last.
	 */
	void get_symbols(std::function<void(const backtrace_symbol *)> cb,
	                 bool reversed = true) const;

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
	      bool generate_backtrace = true,
	      bool store_cause = true);

	virtual ~Error() = default;

	/**
	 * String representation of this exception, as
	 * specialized by a child exception.
	 *
	 * @return String representation of this exception.
	 */
	virtual std::string str() const;

	/**
	 * Returns the message's content.
	 *
	 * @return Char array containing the error message.
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
	 *
	 * @return String with the exception type.
	 */
	virtual std::string type_name() const;

	/**
	 * Return the backtrace where the exception was thrown.
	 * nullptr if no backtrace was collected.
	 *
	 * @return Backtrace to the exception's origin.
	 */
	Backtrace *get_backtrace() const;

	/**
	 * Directly return the message stored in the exception.
	 *
	 * @return String containing the error message.
	 */
	const std::string &get_msg() const;

	/**
	 * Enable invocation of software breakpoint
	 * when this Error is constructed.
	 *
	 * @param enable If true, enable a breakpoint, else disable it.
	 */
	static void enable_break(bool enable);

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
	 * Cached error message text for returning C string via what().
	 */
	mutable std::string what_cache;

	/**
	 * Re-throw this with rethrow_cause().
	 */
	std::exception_ptr cause;

	/**
	 * Issue software breakpoint when an error is constructed.
	 */
	static bool break_on_error;
};


/**
 * Output stream concat for nyanerrors.
 *
 * @param os Output stream the error is appended to.
 * @param e Error whose message is appended to the output stream.
 */
std::ostream &operator<<(std::ostream &os, const Error &e);


/**
 * Output stream concat for backtrace symbols.
 *
 * @param os Output stream the backtrace symbol is appended to.
 * @param bt_sym Backtrace symbol which is appended to the output stream.
 */
std::ostream &operator<<(std::ostream &os, const backtrace_symbol &bt_sym);


/**
 * Output stream concat for backtraces.
 *
 * @param os Output stream the backtrace is appended to.
 * @param bt Backtrace which is appended to the output stream.
 */
std::ostream &operator<<(std::ostream &os, const Backtrace &bt);


/**
 * Internal Error, thrown when some interal sanity check failed.
 */
class InternalError : public Error {
public:
	InternalError(const std::string &msg);
};


/**
 * Error thrown when reading nyan files fails.
 * This is a "low level" error, i.e. file not found,
 * permission problems etc.
 */
class FileReadError : public Error {
public:
	FileReadError(const std::string &msg);
};

} // namespace nyan
