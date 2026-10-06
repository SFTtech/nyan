// Copyright 2026 the nyan authors, LGPLv3+. See copying.md for legal info.

/**
 * @file
 * C++ module interface for `import nyan`.
 *
 * The headers stay the single source of truth: this unit only re-exports
 * their declarations, so imported and included consumers share one set
 * of linker-visible symbols.
 */

module;

#include <nyan/nyan.h>

export module nyan;

export namespace nyan {

using ::nyan::APIError;
using ::nyan::AST;
using ::nyan::ASTArgument;
using ::nyan::ASTBase;
using ::nyan::ASTError;
using ::nyan::ASTImport;
using ::nyan::ASTInheritanceChange;
using ::nyan::ASTMember;
using ::nyan::ASTMemberType;
using ::nyan::ASTMemberTypeArgument;
using ::nyan::ASTMemberValue;
using ::nyan::ASTObject;
using ::nyan::Backtrace;
using ::nyan::BasicType;
using ::nyan::ChangeTracker;
using ::nyan::Container;
using ::nyan::ContainerIterBase;
using ::nyan::ContainerIterator;
using ::nyan::Curve;
using ::nyan::DEFAULT_T;
using ::nyan::Database;
using ::nyan::DefaultIterator;
using ::nyan::Dict;
using ::nyan::Error;
using ::nyan::File;
using ::nyan::FileReadError;
using ::nyan::Filename;
using ::nyan::Float;
using ::nyan::IDToken;
using ::nyan::InheritanceChange;
using ::nyan::Int;
using ::nyan::InternalError;
using ::nyan::InvalidObjectError;
using ::nyan::LATEST_T;
using ::nyan::LangError;
using ::nyan::Lexer;
using ::nyan::LexerError;
using ::nyan::Location;
using ::nyan::Member;
using ::nyan::MemberInfo;
using ::nyan::MemberNotFoundError;
using ::nyan::MemberTypeError;
using ::nyan::MetaInfo;
using ::nyan::NameError;
using ::nyan::Namespace;
using ::nyan::NamespaceFinder;
using ::nyan::None;
using ::nyan::Number;
using ::nyan::NumberBase;
using ::nyan::Object;
using ::nyan::ObjectChanges;
using ::nyan::ObjectHistory;
using ::nyan::ObjectInfo;
using ::nyan::ObjectNotFoundError;
using ::nyan::ObjectNotifier;
using ::nyan::ObjectNotifierHandle;
using ::nyan::ObjectState;
using ::nyan::ObjectValue;
using ::nyan::Operator;
using ::nyan::OrderedSet;
using ::nyan::Parser;
using ::nyan::PatchInfo;
using ::nyan::SPACES_PER_INDENT;
using ::nyan::Set;
using ::nyan::SetBase;
using ::nyan::SetIterator;
using ::nyan::State;
using ::nyan::StateHistory;
using ::nyan::Text;
using ::nyan::Token;
using ::nyan::TokenStream;
using ::nyan::TokenizeError;
using ::nyan::Transaction;
using ::nyan::Type;
using ::nyan::TypeError;
using ::nyan::Value;
using ::nyan::ValueHolder;
using ::nyan::ValueLike;
using ::nyan::ValueOrObjectLike;
using ::nyan::ValueToken;
using ::nyan::View;
using ::nyan::backtrace_symbol;
using ::nyan::bracket_type;
using ::nyan::comma_list;
using ::nyan::composite_t;
using ::nyan::composite_type_to_string;
using ::nyan::dict_t;
using ::nyan::fqnn_t;
using ::nyan::fqon_t;
using ::nyan::inher_change_t;
using ::nyan::memberid_t;
using ::nyan::modifier_t;
using ::nyan::namespace_lookup_t;
using ::nyan::no_nyan_ops;
using ::nyan::nyan_op;
using ::nyan::op_from_string;
using ::nyan::op_from_token;
using ::nyan::op_to_string;
using ::nyan::operator<<;
using ::nyan::order_t;
using ::nyan::ordered_set_t;
using ::nyan::override_depth_t;
using ::nyan::primitive_t;
using ::nyan::set_t;
using ::nyan::token_needs_payload;
using ::nyan::token_type;
using ::nyan::token_type_str;
using ::nyan::type_to_string;
using ::nyan::update_cb_t;
using ::nyan::value_float_t;
using ::nyan::value_int_t;
using ::nyan::view_state;
using ::nyan::view_update;

namespace datastructure {

using ::nyan::datastructure::OrderedSet;

} // namespace datastructure

namespace lexer {

using ::nyan::lexer::Impl;

} // namespace lexer

namespace util {

using ::nyan::util::Container;
using ::nyan::util::Flags;
using ::nyan::util::Streamable;
using ::nyan::util::contains;
using ::nyan::util::convert_str;
using ::nyan::util::demangle;
using ::nyan::util::ends_with;
using ::nyan::util::hash_combine;
using ::nyan::util::isinstance;
using ::nyan::util::match_failure;
using ::nyan::util::read_file;
using ::nyan::util::split;
using ::nyan::util::stream_container_elem;
using ::nyan::util::strjoin;
using ::nyan::util::symbol_name;
using ::nyan::util::typestring;
using ::nyan::util::vector_extend;

} // namespace util

} // namespace nyan
