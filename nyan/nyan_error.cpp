// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_error.h"

#include <execinfo.h>
#include <memory>
#include <sstream>

#include "nyan_util.h"


namespace nyan {


void Backtrace::analyze() {
	std::vector<void *> buffer{32};

	// increase buffer size until it's enough
	while (true) {
		int elements = backtrace(buffer.data(), buffer.size());
		if (elements < (ssize_t) buffer.size()) {
			buffer.resize(elements);
			break;
		}
		buffer.resize(buffer.size() * 2);
	}

	for (void *element : buffer) {
		this->stack_addrs.push_back(element);
	}
}


void Backtrace::get_symbols(std::function<void (const backtrace_symbol *)> cb, bool reversed) const {
	backtrace_symbol symbol;

	if (reversed) {
		for (size_t idx = this->stack_addrs.size(); idx-- > 0;) {
			void *pc = this->stack_addrs[idx];

			symbol.functionname = util::symbol_name(pc, false, true);
			symbol.pc = pc;

			cb(&symbol);
		}
	} else {
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


NyanError::NyanError(const std::string &msg,
                     bool generate_backtrace,
                     bool store_cause)
	:
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
}


std::string NyanError::str() const {
	return this->msg;
}


const char *NyanError::what() const noexcept {
	return this->msg.c_str();
}


void NyanError::store_cause() {
	if (not std::current_exception()) {
		return;
	}

	try {
		throw;
	} catch (NyanError &cause) {
		cause.trim_backtrace();
		this->cause = std::current_exception();
	} catch (...) {
		this->cause = std::current_exception();
	}
}


void NyanError::trim_backtrace() {
	if (this->backtrace) {
		this->backtrace->trim_to_current_stack_frame();
	}
}


void NyanError::rethrow_cause() const {
	if (this->cause) {
		std::rethrow_exception(this->cause);
	}
}


std::string NyanError::type_name() const {
	return util::demangle(typeid(*this).name());
}


Backtrace *NyanError::get_backtrace() const {
	return this->backtrace.get();
}


const std::string &NyanError::get_msg() const {
	return this->msg;
}


NyanInternalError::NyanInternalError(const std::string &msg)
	:
	NyanError{msg} {}


NyanFileError::NyanFileError(const NyanLocation &location,
                             const std::string &msg)
	:
	NyanError{msg},
	location{location} {}


std::string NyanFileError::str() const {
	std::ostringstream builder;

	this->location.str(builder);

	builder << "\x1b[31;1merror:\x1b[m " << this->msg;
	return builder.str();
}


NameError::NameError(const NyanLocation &location,
                     const std::string &msg,
                     const std::string &name)
	:
	NyanFileError{location, msg},
	name{name} {}


std::string NameError::str() const {
	if (not this->name.empty()) {
		std::ostringstream builder;
		builder << this->msg << ": '" << this->name << "'";
		return builder.str();
	}
	else {
		return this->msg;
	}
}


std::ostream &operator <<(std::ostream &os, const NyanError &e) {
	// output the exception cause
	bool had_a_cause = true;
	try {
		e.rethrow_cause();
		had_a_cause = false;
	} catch (NyanError &cause) {
		os << cause << std::endl;
	} catch (std::exception &cause) {
		os << util::demangle(typeid(cause).name()) << ": " << cause.what() << std::endl;
	}

	if (had_a_cause) {
		os << std::endl
		   << "The above exception was the direct cause "
		      "of the following exception:"
		   << std::endl << std::endl;
	}

	// output the exception backtrace
	if (e.get_backtrace()) {
		os << *e.get_backtrace();
	} else {
		os << "origin:" << std::endl;
	}

	os << e.type_name() << ": ";
	os << e.str() << std::endl;

	return os;
}


/**
 * Prints a backtrace_symbol object.
 */
std::ostream &operator <<(std::ostream &os, const backtrace_symbol &bt_sym) {
	// imitate the looks of a Python traceback.
	os << " -> ";

	if (bt_sym.functionname.empty()) {
		os << '?';
	} else {
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
std::ostream &operator <<(std::ostream &os, const Backtrace &bt) {
	// imitate the looks of a Python traceback.
	os << "Traceback (most recent call last):" << std::endl;

	bt.get_symbols([&os](const backtrace_symbol *symbol) {
		os << *symbol << std::endl;
	}, true);

	return os;
}

} // namespace nyan
