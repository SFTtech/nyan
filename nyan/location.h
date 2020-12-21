// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <sstream>
#include <string>

namespace nyan {

class File;
class Token;
class IDToken;


/**
 * Location of some data in nyan.
 * Used to display error messages for positions in the file.
 */
class Location {
public:
    Location() = default;
    Location(const Token &token);
    Location(const IDToken &token);
    Location(const std::shared_ptr<File> &file, int line,
             int line_offset, int length=0);
    explicit Location(const std::string &custom);

    ~Location() = default;

    /**
     * Checks if the location is a built-in nyan location.
     *
     * @return true if the location is built-in, else false.
     */
    bool is_builtin() const;

    /**
     * Get the message for the location.
     *
     * @return String containing the message.
     */
    const std::string &get_msg() const;

    /**
     * Get the line index of the location in the file.
     *
     * @return Line index in the file (starting from 1).
     */
    int get_line() const;

    /**
     * Get the line offset of the location in its line.
     *
     * @return Line offset in the line.
     */
    int get_line_offset() const;

    /**
     * Get the content length of the location in the file.
     *
     * @return Number of characters in the location content.
     */
    int get_length() const;

    /**
     * Get the line of the location in the file.
     *
     * @return String containing the contents of the line.
     */
    std::string get_line_content() const;

    /**
     * Append the string representation of the location to a given output stream.
     *
     * @param builder Output stream the string representation is appended to.
     */
    void str(std::ostringstream &builder) const;

protected:
    /**
     * if true, this location does not point to a file,
     * instead it describes some built-in location of nyan itself.
     */
    bool _is_builtin = false;

    /**
     * Shared pointer to the file of the location.
     */
    std::shared_ptr<File> file;

    /**
     * Line index in the file (starting from 1).
     */
    int line;

    /**
     * Line offset in its line.
     */
    int line_offset;

    /**
     * Length of the content the location associates with.
     */
    int length;

    /**
     * Message for built-in locations.
     */
    std::string msg;
};

} // namespace nyan
