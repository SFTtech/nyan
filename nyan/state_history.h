// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "config.h"
#include "object_history.h"


namespace nyan {

class Database;
class MetaInfo;
class ObjectState;
class State;



/**
 * Object state history tracking.
 */
class StateHistory {
public:
	StateHistory(const std::shared_ptr<Database> &base);

	/** return the state for t or if t doesn't exist, the latest state before that */
	const std::shared_ptr<State> &get_state(order_t t) const;

	/** return the latest state before t, that is if t exists, the previous one. */
	const std::shared_ptr<State> &get_state_before(order_t t) const;

	/** return the state at exactly t, which is nullptr if there's no state at t */
	const std::shared_ptr<State> *get_state_exact(order_t t) const;

	/**
	 * find the latest object state for a given object at t.
	 * if there's no object state at t, take the latest state before t.
	 */
	const std::shared_ptr<ObjectState> *get_obj_state(const fqon_t &fqon, order_t t) const;

	void insert(std::shared_ptr<State> &&new_state, order_t t);

	void insert_linearization(std::vector<fqon_t> &&ins, order_t t);
	const std::vector<fqon_t> &get_linearization(const fqon_t &obj, order_t t,
	                                             const MetaInfo &meta_info) const;

	void insert_children(const fqon_t &obj, std::unordered_set<fqon_t> &&ins, order_t t);
	const std::unordered_set<fqon_t> &get_children(const fqon_t &obj, order_t t,
	                                               const MetaInfo &meta_info) const;

protected:
	ObjectHistory *get_obj_history(const fqon_t &obj);
	const ObjectHistory *get_obj_history(const fqon_t &obj) const;
	ObjectHistory &get_create_obj_history(const fqon_t &obj);

	/**
	 * Storage of states over time.
	 */
	Curve<std::shared_ptr<State>> history;

	/**
	 * Information history for each object.
	 * Optimizes searches in the history.
	 */
	std::unordered_map<fqon_t, ObjectHistory> object_obj_hists;
};


} // namespace nyan
