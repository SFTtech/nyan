// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "transaction.h"

#include "c3.h"
#include "object_state.h"
#include "state.h"
#include "view.h"


namespace nyan {

Transaction::Transaction(order_t at, std::shared_ptr<View> &&origin):
	at{at},
	view{origin} {}


// TODO: inheritance changes need to be simulated within this transactions
//       so later-added patches can depend on that.

bool Transaction::add(const Object &obj) {
	if (not obj.is_patch()) {
		return false;
	}

	// TODO: more sanity checks for the transaction here

	patch_group patch;
	patch.target = obj.get_target();

	for (auto &patch_name : obj.get_linearized(this->at)) {
		patch.patches.push_back(patch_name);
	}

	this->patches.push_back(std::move(patch));
	return true;
}


bool Transaction::add(const Object &/*obj*/, const Object &/*target*/) {
	throw InternalError{"TODO custom patch target"};
}


bool Transaction::commit() {
	// TODO: check if no other transaction happened

	return (this->commit_view(this->view) == false);
}


bool Transaction::commit_view(std::shared_ptr<View> &target_view) {
	bool failed = false;

	// if not existing, create a new state
	// this drops all states that were later than the new one
	State &state = target_view->new_state(this->at);

	// apply all patches in the transaction
	for (auto &patch : this->patches) {
		// copy the object to be patched so the old one keeps its state
		ObjectState &target_obj = state.copy_object(patch.target, this->at, target_view);

		bool parents_changed = false;

		// apply all patch parents in order (last the patch itself)
		for (auto &patch_component : patch.patches) {
			parents_changed |= target_obj.apply(
				target_view->get_raw(patch_component, this->at),
				target_view->get_info(patch_component)
			);
		}

		// the parents of the target object changed
		// -> we need to recalculate the inheritance hierarchy.
		if (parents_changed) {
			linearize(
				patch.target,
				[this] (const fqon_t &name) -> ObjectState& {
					// TODO: make sure no other state is modified somehow.
					ObjectState *state = this->view->get_raw(name, this->at).get();
					if (unlikely(state == nullptr)) {
						throw InternalError{"object state not found for parent"};
					}
					return *state;
				}
			);
		}
	}

	// apply the patch in all child views
	for (auto &target_child_view_weakptr : target_view->get_children()) {
		auto target_child_view = target_child_view_weakptr.lock();
		if (not target_child_view) {
			throw InternalError{"child view vanished"};
		}

		failed |= this->commit_view(target_child_view);
	}

	// TODO: update the object-changed pointer in the target view

	return failed;
}


} // namespace nyan
