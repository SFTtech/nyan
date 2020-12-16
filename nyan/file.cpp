// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "file.h"

#include "error.h"
#include "util.h"


namespace nyan {


File::File(const std::string &virtual_name, std::string &&data)
    :
    name{virtual_name},
    data{std::move(data)} {

    this->extract_lines();
}


File::File(const std::string &path)
    :
    File{path, util::read_file(path)} {

    // util::read_file throws a FileReadError if unsuccessful.
}


void File::extract_lines() {
    this->line_ends = { std::string::npos };

    for (size_t i = 0; i < this->data.size(); i++) {
        if (this->data[i] == '\n') {
            this->line_ends.push_back(i);
        }
    }
    this->line_ends.push_back(data.size());
}


const std::string &File::get_name() const {
    return this->name;
}


const std::string &File::get_content() const {
    return this->data;
}


std::string File::get_line(size_t n) const {
    size_t begin = this->line_ends[n - 1] + 1;
    size_t len = this->line_ends[n] - begin;
    return this->data.substr(begin, len);
}


const char *File::c_str() const {
    return this->data.c_str();
}


size_t File::size() const {
    return this->data.size();
}


} // namespace nyan
