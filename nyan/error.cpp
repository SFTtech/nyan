// Copyright 2016-2025 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "error.h"

#include <memory>
#include <sstream>
#if defined(_WIN32) || defined(__CYGWIN__)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	// TODO - CaptureStackBackTrace doesn't report number of frames available
    // need to implement increase buffer size
	#define backtrace(buffer, buf_size) CaptureStackBackTrace(0, buf_size, buffer, NULL)
#else
	#include <execinfo.h>
#endif

#include "compiler.h"
#include "util.h"


namespace nyan {

using namespace std::string_literals;


void Backtrace::analyze() {
	std::vector<void *> buffer{32};

	// increase buffer size until it's enough
	while (true) {
		int elements = backtrace(buffer.data(), buffer.size());
		if (elements < static_cast<decltype(elements)>(buffer.size())) {
			buffer.resize(elements);
			break;
		}
		buffer.resize(buffer.size() * 2);
	}

	for (void *element : buffer) {
		this->stack_addrs.push_back(element);
	}
}


void Backtrace::get_symbols(std::function<void(const backtrace_symbol *)> cb, bool reversed) const {
	backtrace_symbol symbol;

	if (reversed) {
		for (size_t idx = this->stack_addrs.size(); idx-- > 0;) {
			void *pc = this->stack_addrs[idx];

			symbol.functionname = util::symbol_name(pc, false, true);
			symbol.pc = pc;

			cb(&symbol);
		}
	}
	else {
		for (void *pc : this->stack_addrs) {
			symbol.functionname = util::symbol_name(pc, false, true);
			symbol.pc = pc;

			cb(&symbol);
		}
	}
}


void Backtrace::trim_to_current_stack_frame() {
	Backtrace current;
	current.analyze();

	while (not current.stack_addrs.empty() and not this->stack_addrs.empty()) {
		if (this->stack_addrs.back() != current.stack_addrs.back()) {
			break;
		}

		this->stack_addrs.pop_back();
		current.stack_addrs.pop_back();
	}
}


constexpr const char *runtime_error_message = "polymorphic nyan error, catch by reference!";

bool Error::break_on_error = false;


Error::Error(const std::string &msg,
             bool generate_backtrace,
             bool store_cause) :
	std::runtime_error{runtime_error_message},
	backtrace{nullptr},
	msg{msg} {
	if (generate_backtrace) {
		this->backtrace = std::make_shared<Backtrace>();
		this->backtrace->analyze();
	}

	if (store_cause) {
		this->store_cause();
	}

	if (unlikely(this->break_on_error)) {
		BREAKPOINT;
	}
}


std::string Error::str() const {
	return this->msg;
}


const char *Error::what() const noexcept {
	this->what_cache = this->str();
	return this->what_cache.c_str();
}


void Error::store_cause() {
	if (not std::current_exception()) {
		return;
	}

	try {
		throw;
	}
	catch (Error &cause) {
		cause.trim_backtrace();
		this->cause = std::current_exception();
	}
	catch (...) {
		this->cause = std::current_exception();
	}
}


void Error::trim_backtrace() {
	if (this->backtrace) {
		this->backtrace->trim_to_current_stack_frame();
	}
}


void Error::rethrow_cause() const {
	if (this->cause) {
		std::rethrow_exception(this->cause);
	}
}


std::string Error::type_name() const {
	return util::demangle(typeid(*this).name());
}


Backtrace *Error::get_backtrace() const {
	return this->backtrace.get();
}


const std::string &Error::get_msg() const {
	return this->msg;
}


void Error::enable_break(bool enable) {
	Error::break_on_error = enable;
}


std::ostream &operator<<(std::ostream &os, const Error &e) {
	// output the exception cause
	bool had_a_cause = true;
	try {
		e.rethrow_cause();
		had_a_cause = false;
	}
	catch (Error &cause) {
		os << cause << std::endl;
	}
	catch (std::exception &cause) {
		os << util::demangle(typeid(cause).name()) << ": " << cause.what() << std::endl;
	}

	if (had_a_cause) {
		os << std::endl
		   << "The above exception was the direct cause "
			  "of the following exception:"
		   << std::endl
		   << std::endl;
	}

	// output the exception backtrace
	if (e.get_backtrace()) {
		os << *e.get_backtrace();
	}
	else {
		os << "origin:" << std::endl;
	}

	os << e.type_name() << ":" << std::endl;
	os << e.str();

	return os;
}


/**
 * Prints a backtrace_symbol object.
 */
std::ostream &operator<<(std::ostream &os, const backtrace_symbol &bt_sym) {
	// imitate the looks of a Python traceback.
	os << " -> ";

	if (bt_sym.functionname.empty()) {
		os << '?';
	}
	else {
		os << bt_sym.functionname;
	}

	if (bt_sym.pc != nullptr) {
		os << " [" << bt_sym.pc << "]";
	}

	return os;
}


/**
 * Prints an entire Backtrace object.
 */
std::ostream &operator<<(std::ostream &os, const Backtrace &bt) {
	// imitate the looks of a Python traceback.
	os << "Traceback (most recent call last):" << std::endl;

	bt.get_symbols([&os](const backtrace_symbol *symbol) {
		os << *symbol << std::endl;
	},
	               true);

	return os;
}


InternalError::InternalError(const std::string &msg) :
	Error{msg} {}


FileReadError::FileReadError(const std::string &msg) :
	Error{msg} {}

} // namespace nyan
