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



File::File(File &&other) noexcept
	:
	name{std::move(other.name)},
	data{std::move(other.data)} {}


void File::extract_lines() {
	this->lines.clear();

	size_t pos = 0;
	size_t len = 0;
	for (size_t i = 0; i < this->data.size(); i++) {
		if (this->data[i] == '\n') {
			this->lines.push_back(line_info{pos, len});
			pos = i + 1;
			len = 0;
		} else {
			len++;
		}
	}
	this->lines.push_back(line_info{pos, len});
}


File &File::operator =(File &&other) noexcept {
	this->name = std::move(other.name);
	this->data = std::move(other.data);
	return *this;
}


const std::string &File::get_name() const {
	return this->name;
}


const std::string &File::get_content() const {
	return this->data;
}


std::string File::get_line(size_t n) const {
	line_info lineinfo = this->lines[n - 1];
	return this->data.substr(lineinfo.offset, lineinfo.len);
}


const char *File::c_str() const {
	return this->data.c_str();
}


size_t File::size() const {
	return this->data.size();
}


} // namespace nyan
