// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_file.h"

#include "nyan_error.h"
#include "nyan_util.h"


namespace nyan {

NyanFile::NyanFile(const std::string &path)
	:
	name{path},
	data{util::read_file(path)} {}


NyanFile::NyanFile(const std::string &virtual_name, const std::string &data)
	:
	name{virtual_name},
	data{data} {}


NyanFile::NyanFile(const std::string &virtual_name, std::string &&data)
	:
	name{virtual_name},
	data{std::move(data)} {}


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


const char *NyanFile::c_str() const {
	return this->data.c_str();
}


size_t NyanFile::size() const {
	return this->data.size();
}


} // namespace nyan
