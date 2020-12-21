// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>

#include "value.h"


namespace nyan {

class IDToken;

/**
 * Nyan value to store text.
 */
class Text : public Value {
public:
    Text(const std::string &value);
    Text(const IDToken &token);

    ValueHolder copy() const override;
    std::string str() const override;
    std::string repr() const override;
    size_t hash() const override;

    const std::string &get() const {
        return *this;
    }

    const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
    const BasicType &get_type() const override;

    operator const std::string&() const {
        return this->value;
    }

    operator const char *() const {
        return this->value.c_str();
    }

protected:
    void apply_value(const Value &value, nyan_op operation) override;
    bool equals(const Value &other) const override;

    std::string value;
};

} // namespace nyan
