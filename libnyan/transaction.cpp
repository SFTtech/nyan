// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "transaction.h"

#include "object_state.h"
#include "state.h"
#include "view.h"


namespace nyan {

Transaction::Transaction(order_t at, std::shared_ptr<View> &&origin):
	at{at},
	view{origin} {}


// TODO: inheritance changes need to be simulated within this transactions
//       so later-added patches can depend on that.

bool Transaction::add(Object &obj) {
	if (not obj.is_patch()) {
		return false;
	}

	// TODO: more sanity checks for the transaction here

	patch_group patch;
	patch.target = obj.get_target();

	for (auto &patch_name : obj.linearize_parents()) {
		patch.patches.push_back(patch_name);
	}

	this->patches.push_back(std::move(patch));
	return true;
}


bool Transaction::add(Object &obj, Object &target) {
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
		nyan::ObjectState &target_obj = state.copy_object(patch.target, this->at, target_view);

		// apply all patch parents in order (last the patch itself)
		for (auto &patch_component : patch.patches) {
			target_obj.apply(target_view->get_raw(patch_component, this->at));
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
