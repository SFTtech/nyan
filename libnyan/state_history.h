// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "config.h"
#include "object_cache.h"


namespace nyan {

class Database;
class MetaInfo;
class State;



/**
 * Object state history tracking.
 * TODO: merge this class into View.
 */
class StateHistory {
public:
	StateHistory(const std::shared_ptr<Database> &base);

	const State &get_state(order_t t) const;
	const std::shared_ptr<State> &get_state_ptr(order_t t) const;
	const std::shared_ptr<State> &get_state_before(order_t t) const;
	const std::shared_ptr<State> *get_state_exact(order_t t) const;

	void insert(std::shared_ptr<State> &&new_state, order_t t);

	void insert_linearization(std::vector<fqon_t> &&ins, order_t t);
	const std::vector<fqon_t> &get_linearization(const fqon_t &obj, order_t t,
	                                             const MetaInfo &meta_info) const;

	void insert_children(const fqon_t &obj, std::unordered_set<fqon_t> &&ins, order_t t);
	const std::unordered_set<fqon_t> &get_children(const fqon_t &obj, order_t t,
	                                               const MetaInfo &meta_info) const;

protected:
	ObjectCache *get_cache(const fqon_t &obj);
	const ObjectCache *get_cache(const fqon_t &obj) const;
	ObjectCache &get_create_cache(const fqon_t &obj);

	std::shared_ptr<Database> database;

	/**
	 * Storage of states over time.
	 */
	Curve<std::shared_ptr<State>> history;

	/**
	 * Information cache for each object.
	 * Optimizes searches in the history.
	 */
	std::unordered_map<fqon_t, ObjectCache> object_caches;
};


} // namespace nyan
