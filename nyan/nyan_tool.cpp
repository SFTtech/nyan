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

void test_parser(const std::string &base_path, const std::string &filename) {
	auto db = Database::create();

	db->load(
		filename,
		[&base_path] (const std::string &filename) {
			return std::make_shared<File>(base_path + "/" + filename);
		}
	);

	std::shared_ptr<View> root = db->new_view();

	Object second = root->get("test.Second");
	std::cout << "Second.member == " << second.get<Int>("member") << std::endl;
	Object first = root->get("test.First");
	std::cout << "First.test == " << first.get<Text>("test") << std::endl;

	Object patch = root->get("test.FirstPatch");
	Transaction tx = root->new_transaction();
	tx.add(patch);
	tx.commit();

	std::cout << "after change: Second.member == "
	          << second.get<Int>("member") << std::endl;
}


int run(flags_t flags, params_t params) {
	try {
		if (flags[option_flag::TEST_PARSER]) {
			const std::string &filename = params[option_param::FILE];

			std::vector<std::string> parts = util::split(filename, '/');

			// first file is assumed to be in the root.
			std::string first_file = parts[parts.size() - 1];

			// everything else is the base path
			parts.pop_back();
			std::string base_path = util::strjoin("/", parts);

			try {
				nyan::test_parser(base_path, first_file);
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
	          << "-b --break                 -- debug-break on error" << std::endl
	          << "   --test-parser           -- test the parser" << std::endl
	          << "   --echo                  -- print the ast" << std::endl
	          << "" << std::endl;
}


std::pair<flags_t, params_t> argparse(int argc, char** argv) {
	flags_t flags{
		{option_flag::ECHO, false},
		{option_flag::TEST_PARSER, false}
	};

	params_t params{
		{option_param::FILE, ""}
	};

	int option_index = 0;

	while (true) {
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"file", required_argument, 0, 'f'},
			{"break", no_argument, 0, 'b'},
			{"echo", no_argument, 0, 0},
			{"test-parser", no_argument, 0, 0},
			{0, 0, 0, 0}
		};

		int c = getopt_long(argc, argv,
		                    "hf:b",
		                    long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0: {
			const char *op_name = long_options[option_index].name;
			if (strcmp("test-parser", op_name) == 0) {
				flags[option_flag::TEST_PARSER] = true;
			}

			break;
		}
		case 'f':
			params[option_param::FILE] = optarg;
			break;

		case 'h': help(); exit(0); break;
		case 'b': Error::enable_break(true); break;
		case '?': break;

		default: printf("error in getopt config: "
		                "returned character code 0%o\n", c);
		}
	}

	// required positional args
	if (option_index < argc) {
		while (optind < argc) {
			std::cout << argv[option_index] << std::endl;
			option_index += 1;
		}
	}


	return std::make_pair(flags, params);
}

} // namespace nyan


int main(int argc, char **argv) {

	auto args = nyan::argparse(argc, argv);
	nyan::flags_t flags = args.first;
	nyan::params_t params = args.second;

#define NYAN_CATCHALL

#ifndef NYAN_CATCHALL
	try {
#endif
		return nyan::run(flags, params);
#ifndef NYAN_CATCHALL
	}
	catch (std::exception &exc) {
		std::cout << "\x1b[31;1mfatal error:\x1b[m "
		          << exc.what() << std::endl;
		return 1;
	}
#endif
}
