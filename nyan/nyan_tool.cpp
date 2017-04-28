// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_tool.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

#include "../libnyan/nyan.h"

namespace nyan {

void test_parser(const File &file) {
	Database database;

	Parser parser(&database);
	std::vector<Object *> objs = parser.parse(file);

	size_t i = 0;
	for (auto &obj : objs) {
		std::cout << "object " << i << " :" << std::endl;
		std::cout << obj->str() << std::endl;
		i += 1;
	}
}


int run(flags_t flags, params_t params) {
	try {
		if (flags[option_flag::TEST_PARSER]) {
			const std::string &filename = params[option_param::FILE];
			File input{filename};

			try {
				nyan::test_parser(input);
			}
			catch (FileError &err) {
				std::cout << "\x1b[33;1mfile error:\x1b[m\n"
				          << err << std::endl
				          << err.show_problem_origin()
				          << std::endl << std::endl;
				return 1;
			}
		}
		else {
			std::cout << "no action selected" << std::endl << std::endl;
			help();
		}
	}
	catch (Error &err) {
		std::cout << "\x1b[31;1merror:\x1b[m\n"
		          << err << std::endl;
		return 1;
	}
	return 0;
}


void help() {
	std::cout << "\x1b[32;1mnyan\x1b[m - "
	             "\x1b[32;1my\x1b[met "
	             "\x1b[32;1ma\x1b[mnother "
	             "\x1b[32;1mn\x1b[motation "
	             "-- tool" << std::endl
	          << std::endl
	          << "usage:" << std::endl
	          << "-h --help                  -- show this" << std::endl
	          << "-f --file <filename>       -- file to load" << std::endl
	          << "   --test-parser           -- test the parser" << std::endl
	          << "   --echo                  -- print the ast" << std::endl
	          << "" << std::endl;
}

} // namespace nyan


int main(int argc, char **argv) {
	nyan::flags_t flags{
		{nyan::option_flag::ECHO, false},
		{nyan::option_flag::TEST_PARSER, false}
	};

	nyan::params_t params{
		{nyan::option_param::FILE, ""}
	};

	while (true) {
		int option_index = 0;

		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"file", required_argument, 0, 'f'},
			{"echo", no_argument, 0, 0},
			{"test-parser", no_argument, 0, 0},
			{0, 0, 0, 0}
		};

		int c = getopt_long(argc, argv,
		                    "hf:",
		                    long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0: {
			const char *op_name = long_options[option_index].name;
			if (strcmp("test-parser", op_name) == 0) {
				flags[nyan::option_flag::TEST_PARSER] = true;
			}

			break;
		}
		case 'f':
			params[nyan::option_param::FILE] = optarg;
			break;

		case 'h': nyan::help(); exit(0); break;
		case '?': break;

		default: printf("error in getopt config: "
		                "returned character code 0%o\n", c);
		}
	}

	try {
		return nyan::run(flags, params);
	}
	catch (std::exception &exc) {
		std::cout << "\x1b[31;1mfatal error:\x1b[m "
		          << exc.what() << std::endl;
		return 1;
	}
}
