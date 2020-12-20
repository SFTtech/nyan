// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_tool.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "nyan.h"


namespace nyan {

int test_parser(const std::string &base_path, const std::string &filename) {
    int ret = 0;
    auto db = Database::create();

    db->load(
        filename,
        [&base_path] (const std::string &filename) {
            return std::make_shared<File>(base_path + "/" + filename);
        }
    );

    std::shared_ptr<View> root = db->new_view();

    Object second = root->get_object("test.Second");
    Object first = root->get_object("test.First");

    std::cout << "after change: First.member == "
              << *root->get_object("test.First").get<Int>("member")
              << std::endl;

    Object patch = root->get_object("test.FirstPatch");
    for (int i = 0; i < 3; i++) {
        Transaction tx = root->new_transaction();
        tx.add(patch);
        if (not tx.commit()) {
            std::cout << "fail @ " << i << std::endl;
        }
    }

    auto value = *root->get_object("test.First").get<Int>("member");

    std::cout << "after change: First.member == "
              << value.str()
              << std::endl;

    if (value != 24) {
        std::cout << "patch result is wrong" << std::endl;
        return 1;
    }


    order_t trans_callback_time = -1;
    bool got_child_callback = false;
    auto cb_func = [&](order_t t, const fqon_t &fqon, const ObjectState &) {
        trans_callback_time = t;
        if (fqon == "test.TestChild") {
            got_child_callback = true;
        }
        std::cout << "got transaction callback for object " << fqon << std::endl;
    };

    auto callback_hdl_test = root->get_object("test.Test").subscribe(cb_func);
    auto callback_hdl_testchild = root->get_object("test.TestChild").subscribe(cb_func);

    bool success;
    order_t trans_time = 1;
    Transaction tx = root->new_transaction(trans_time);
    tx.add(patch);
    tx.add(root->get_object("test.Patch"));
    tx.add(root->get_object("test.SetPatch"));
    success = tx.commit();

    if (trans_callback_time != trans_time) {
        std::cout << "Transaction callback failed" << std::endl;
    }

    if (not got_child_callback) {
        std::cout << "got no callback for TestChild" << std::endl;
        ret = 1;
    }

    if (success) {
        std::cout << "Transaction OK" << std::endl;
    }
    else {
        std::cout << "Transaction FAILED" << std::endl;
        ret = 1;
    }

    std::cout << "after change: Second.member == "
              << *second.get<Int>("member", 1)
              << std::endl;

    std::cout << "SetTest.member = "
              << root->get_object("test.SetTest").get_value("member")->str()
              << std::endl
              << "SetTest.orderedmember = "
              << root->get_object("test.SetTest").get_value("orderedmember")->str()
              << std::endl
              << "Fifth.truth = "
              << root->get_object("test.Fifth").get_value("truth")->str()
              << std::endl
              << "PATCH"
              << std::endl
              << "SetTest.member = "
              << root->get_object("test.SetTest").get_value("member", 1)->str()
              << std::endl
              << "SetTest.orderedmember = "
              << root->get_object("test.SetTest").get_value("orderedmember", 1)->str()
              << std::endl << std::endl;

    std::cout << "test.gschicht parents = " << util::strjoin(", ", root->get_object("test.Test").get_parents())
              << std::endl << "PATCH" << std::endl
              << "test.gschicht parents = " << util::strjoin(", ", root->get_object("test.Test").get_parents(1))
              << std::endl << "newvalue = " << root->get_object("test.Test").get_value("new_value", 1)->str()
              << std::endl;

    return ret;
}


int run(flags_t flags, params_t params) {
    try {
        if (flags[option_flag::TEST_PARSER]) {
            const std::string &filename = params[option_param::FILE];

            if (filename.size() == 0) {
                throw Error{"empty filename given"};
            }

            std::vector<std::string> parts = util::split(filename, '/');

            // first file is assumed to be in the root.
            std::string first_file = parts[parts.size() - 1];

            // everything else is the base path
            parts.pop_back();
            std::string base_path = util::strjoin("/", parts);

            try {
                return nyan::test_parser(base_path, first_file);
            }
            catch (LangError &err) {
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

    for (int option_index = 1; option_index < argc; ++option_index) {
        std::string arg = argv[option_index];
        if (arg == "-h" or arg == "--help") {
            help();
            exit(0);
        }
        else if (arg == "-f" or arg == "--file") {
            ++option_index;
            if (option_index == argc) {
                std::cerr << "Filename not specified" << std::endl;
                help();
                exit(-1);
            }
            params[option_param::FILE] = argv[option_index];
        }
        else if (arg == "-b" or arg == "--break") {
            Error::enable_break(true);
        }
        else if (arg == "--echo") {
            flags[option_flag::ECHO] = true;
        }
        else if (arg == "--test-parser") {
            flags[option_flag::TEST_PARSER] = true;
        }
        else {
            std::cerr << "Unused argument: " << arg << std::endl;
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
