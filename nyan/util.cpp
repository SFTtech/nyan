// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "util.h"

#include <cstring>
#ifndef _MSC_VER
#include <cxxabi.h>
#include <dlfcn.h>
#endif
#include <cerrno>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

#if defined(_WIN32) || defined(__CYGWIN__)
#include <array>
#include <mutex>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>


namespace {


std::string symbol_name_win(const void *addr, bool require_exact_addr) {
    // Handle to the current process
    static HANDLE process_handle = INVALID_HANDLE_VALUE;
    static bool initialized_symbol_handler = false;
    static bool initialized_symbol_handler_successfully = false;

    // SymInitialize & SymFromAddr are, according to MSDN, not thread-safe.
    static std::mutex sym_mutex;
    std::lock_guard<std::mutex> sym_lock_guard{sym_mutex};

    // Initialize symbol handler for process, if it has not yet been initialized
    // If we are not succesful on the first try, leave it, since MSDN says that searching for symbol files is very time consuming
    if (!initialized_symbol_handler) {
        initialized_symbol_handler = true;

        process_handle = GetCurrentProcess();
        initialized_symbol_handler_successfully = SymInitialize(process_handle, nullptr, TRUE);
    }

    if (!initialized_symbol_handler_successfully) {
        return {};
    }

    // The magic of winapi
    constexpr int name_buffer_size = 1024;
    constexpr int buffer_size = sizeof(SYMBOL_INFO) + name_buffer_size * sizeof(char);
    std::array<char, buffer_size> buffer;

    SYMBOL_INFO *symbol_info = reinterpret_cast<SYMBOL_INFO*>(buffer.data());

    symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol_info->MaxNameLen = name_buffer_size;

    DWORD64 symbol_offset = 0;
    if (not SymFromAddr(process_handle, reinterpret_cast<DWORD64>(addr),
                        &symbol_offset, symbol_info)
        or (require_exact_addr and symbol_offset != 0)) {
        return {};
    }

    return std::string(symbol_info->Name);
}


} // <anonymous> namespace
#endif


namespace nyan::util {


std::string read_file(const std::string &filename, bool binary) {
    std::ifstream::openmode mode = std::ifstream::in | std::ifstream::ate;
    if (binary) {
        mode |= std::ifstream::binary;
    }

    std::ifstream input{filename, mode};

    if (input) {
        std::string ret;

        // we're at the file end already because of ifstream::ate
        ret.resize(input.tellg());
        input.seekg(0, std::ios::beg);
        input.read(&ret[0], ret.size());
        input.close();

        return ret;
    }
    else {
        std::ostringstream builder;
        builder << "failed reading file '"
                << filename << "': "
                << strerror(errno);
        throw FileReadError{builder.str()};
    }
}


std::string demangle(const char *symbol) {
#ifdef _MSC_VER
    // TODO: demangle names for MSVC; Possibly using UnDecorateSymbolName
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms681400(v=vs.85).aspx
    return symbol;
#else
    int status;
    char *buf = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);

    if (status != 0) {
        return symbol;
    } else {
        std::string result{buf};
        free(buf);
        return result;
    }
#endif
}


std::string addr_to_string(const void *addr) {
    std::ostringstream out;
    out << "[" << addr << "]";
    return out.str();
}


std::string symbol_name(const void *addr,
                        bool require_exact_addr, bool no_pure_addrs) {
#if defined(_WIN32) || defined(__CYGWIN__)
    auto symbol_name_result = symbol_name_win(addr, require_exact_addr);

    if (!symbol_name_result.empty()) {
        return symbol_name_result;
    }

    return no_pure_addrs ? "" : addr_to_string(addr);
#else
    Dl_info addr_info;

    if (dladdr(addr, &addr_info) == 0) {
        // dladdr has... failed.
        return no_pure_addrs ? "" : addr_to_string(addr);
    } else {
        size_t symbol_offset = reinterpret_cast<size_t>(addr) -
                               reinterpret_cast<size_t>(addr_info.dli_saddr);

        if (addr_info.dli_sname == nullptr or
            (symbol_offset != 0 and require_exact_addr)) {

            return no_pure_addrs ? "" : addr_to_string(addr);
        }

        if (symbol_offset == 0) {
            // this is our symbol name.
            return demangle(addr_info.dli_sname);
        } else {
            std::ostringstream out;
            out << demangle(addr_info.dli_sname)
                << "+0x" << std::hex
                << symbol_offset << std::dec;
            return out.str();
        }
    }
#endif
}


std::vector<std::string> split(const std::string &txt, char delimiter) {
    std::vector<std::string> items;
    // use the back inserter iterator and the templated split function.
    split(txt, delimiter, std::back_inserter(items));
    return items;
}


bool ends_with(const std::string &txt, const std::string &end) {
    if (end.size() > txt.size()) {
        return false;
    }
    return std::equal(end.rbegin(), end.rend(), txt.rbegin());
}


size_t hash_combine(size_t hash1, size_t hash2) {
    // Taken from http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
    return hash1 ^ (hash2 + 0x9e3779b9 + ((hash1 << 6) + (hash1 >> 2)));
}

} // namespace nyan::util
