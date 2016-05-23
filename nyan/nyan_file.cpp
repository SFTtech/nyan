// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_file.h"

#include "nyan_error.h"
#include "nyan_util.h"


namespace nyan {


NyanFile::NyanFile(const std::string &virtual_name, std::string &&data)
	:
	name{virtual_name},
	data{data} {

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


NyanFile::NyanFile(const std::string &path)
	:
	NyanFile{path, util::read_file(path)} {}


NyanFile::NyanFile(NyanFile &&other)
	:
	name{std::move(other.name)},
	data{std::move(other.data)} {}


NyanFile &NyanFile::operator =(NyanFile &&other) {
	this->name = std::move(other.name);
	this->data = std::move(other.data);
	return *this;
}


const std::string &NyanFile::get_name() const {
	return this->name;
}


const std::string &NyanFile::get() const {
	return this->data;
}


std::string NyanFile::get_line(size_t n) const {
	line_info lineinfo = this->lines[n - 1];
	return this->data.substr(lineinfo.offset, lineinfo.len);
}


const char *NyanFile::c_str() const {
	return this->data.c_str();
}


size_t NyanFile::size() const {
	return this->data.size();
}


} // namespace nyan
