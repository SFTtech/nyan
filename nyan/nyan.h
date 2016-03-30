// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_H_
#define NYAN_NYAN_H_

/**
 * @file
 * Main header file for nyan.
 */

#include "nyan_error.h"
#include "nyan_namespace.h"
#include "nyan_object.h"
#include "nyan_parser.h"
#include "nyan_store.h"
#include "nyan_util.h"

/**
 * The nyan engine and interpreter is defined in this namespace.
 */
namespace nyan {

constexpr size_t VERSION = 1.0;

} // namespace nyan

#endif
