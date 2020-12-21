// Copyright 2019-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "lang_error.h"

#include <memory>
#include <sstream>

#include "util.h"


namespace nyan {

using namespace std::string_literals;

LangError::LangError(const Location &location,
                     const std::string &msg,
                     std::vector<std::pair<Location, std::string>> &&reasons)
    :
    Error{msg},
    location{location},
    reasons{std::move(reasons)} {}


std::string LangError::str() const {
    std::ostringstream builder;

    builder << "\x1b[1m";

    this->location.str(builder);

    builder << "\x1b[31;1merror:\x1b[39;49m " << this->msg
            << "\x1b[0m";

    return builder.str();
}


static void visualize_location(std::ostringstream &builder, const Location &location) {
    size_t offset = location.get_line_offset();
    size_t length = location.get_length();

    if (length > 0) {
        length -= 1;
    }
    else {
        length = 0;
    }

    builder << location.get_line_content() << std::endl
            << std::string(offset, ' ') << "\x1b[36;1m^"
            << std::string(length, '~') << "\x1b[m";
}


std::string LangError::show_problem_origin() const {
    std::ostringstream builder;

    if (this->location.is_builtin()) {
        builder << this->location.get_msg();
    }
    else {
        visualize_location(builder, this->location);
    }

    for (const auto &reason : this->reasons) {
        const Location &loc = reason.first;
        const std::string &msg = reason.second;

        builder << std::endl << "\x1b[1m";

        loc.str(builder);

        builder << "\x1b[30;1mnote:\x1b[39;49m "
                << msg
                << "\x1b[0m" << std::endl;

        visualize_location(builder, loc);
        builder << std::endl;
    }

    return builder.str();
}


TypeError::TypeError(const Location &location, const std::string &msg)
    :
    LangError{location, msg} {}


NameError::NameError(const Location &location,
                     const std::string &msg,
                     const std::string &name)
    :
    LangError{location, msg},
    name{name} {}


std::string NameError::str() const {
    std::ostringstream builder;
    builder << "\x1b[1m";

    this->location.str(builder);

    builder << "\x1b[31;1mname error:\x1b[39;49m ";
    builder << this->msg;

    if (not this->name.empty()) {
        builder << ": '" << this->name << "'";
    }

    builder << "\x1b[0m";

    return builder.str();
}


TokenizeError::TokenizeError(const Location &location,
                             const std::string &msg):
    LangError{location, msg} {}

} // namespace nyan
