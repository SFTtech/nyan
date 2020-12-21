// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "transaction.h"

#include "c3.h"
#include "object_state.h"
#include "state.h"
#include "view.h"


namespace nyan {

// TODO think about parallel transactions:
// parralel ones in the future don't matter as we'll overwrite them
// ones that happen before must invalidate this one!


Transaction::Transaction(order_t at, std::shared_ptr<View> &&origin)
    :
    valid{true},
    at{at} {

    auto create_state_mod = [this] (std::shared_ptr<View> &&view) {
        StateHistory &view_history = view->get_state_history();

        // use this as parent state
        // might return the database initial state.
        const std::shared_ptr<State> &base_view_state = view_history.get_state_before(this->at);

        if (unlikely(base_view_state.get() == nullptr)) {
            throw InternalError{"transaction base state is nullptr"};
        }

        // create a state that follows the current view state.
        auto new_view_state = std::make_shared<State>(base_view_state);

        this->states.push_back({
            std::move(view),
            std::move(new_view_state),
            {}
        });
    };

    // first, perform transaction on the requested view
    create_state_mod(std::move(origin));

    // this is actually the `origin` view, but we've moved it there.
    auto &main_view = this->states.at(0).view;

    // recursively visit all of the view's children and their children
    // lol C++
    std::function<void(const std::shared_ptr<View>&)> recurse =
    [&create_state_mod, &recurse] (const std::shared_ptr<View> &view) {
        bool view_has_stale_children = false;

        // also apply the transaction in all childs of the view.
        for (auto &target_child_view_weakptr :
             view->get_children()) {

            auto target_child_view = target_child_view_weakptr.lock();
            if (not target_child_view) {
                // child view no longer there, so we skip it.
                view_has_stale_children = true;
                continue;
            }

            recurse(target_child_view);

            create_state_mod(std::move(target_child_view));
        }

        if (view_has_stale_children) {
            view->cleanup_stale_children();
        }
    };

    recurse(main_view);
}


bool Transaction::add(const Object &patch) {
    if (unlikely(not this->valid)) {
        // TODO: throw some error?
        return false;
    }

    if (unlikely(not patch.is_patch())) {
        return false;
    }

    const auto target_ptr = patch.get_target();
    // TODO: recheck if target exists?
    if (target_ptr == nullptr) {
        throw InternalError{"patch somehow has no target"};
    }
    const auto &target = *target_ptr;

    // apply the patch in each view's state
    for (auto &view_state : this->states) {

        auto &view = view_state.view;
        auto &new_state = view_state.state;
        auto &tracker = view_state.changes;

        // TODO: speed up the state backtracking for finding the object

        // This does not copy the object if the new state already has it.
        auto &target_obj = new_state->copy_object(target, this->at, view);

        // apply each patch component (i.e. all the parents of the patch)
        for (auto &patch_name : patch.get_linearized(this->at)) {

            auto &patch_tracker = tracker.track_patch(target);

            // apply all patch parents in order (last the patch itself)
            target_obj->apply(
                // TODO: use the same mechanism as above to get only parent
                //       obj states of base_state
                view->get_raw(patch_name, this->at),
                view->get_info(patch_name),
                patch_tracker
            );
        }

        // TODO: linearize here so other patches can depend on that?
    }

    return true;
}


bool Transaction::add(const Object &/*patch*/, const Object &/*target*/) {
    // TODO check if the target is a child of the original target
    throw InternalError{"TODO custom patch target"};
}


bool Transaction::commit() {
    if (unlikely(not this->valid)) {
        // TODO: throw some error?
        return false;
    }

    // TODO check if no other transaction was before this one.


    // merge a new state with an already existing base state
    // this must be done for a transaction at a time
    // where data is already stored.
    this->merge_changed_states();

    // for each view: those updates have to be performed
    std::vector<view_update> updates = this->generate_updates();

    // now, all sanity checks are done and we can update the view!
    this->update_views(std::move(updates));

    // TODO mark value caches dirty

    bool ret = this->valid;
    this->valid = false;
    return ret;
}


void Transaction::merge_changed_states() {
    for (auto &view_state : this->states) {
        auto &view = view_state.view;

        StateHistory &view_history = view->get_state_history();

        // new_state contains all modified objects for this view.
        // base_state probably unused

        // TODO: this is not robust against other parallel transactions
        const std::shared_ptr<State> *existing = view_history.get_state_exact(this->at);

        if (existing != nullptr) {
            // state did exist: we have to merge it with the new state:
            // the old state (which described the same time) must be updated
            // with changed objects from the new state.

            // create a copy of the base state which retains its pointers
            // to object states.
            auto merge_base = std::make_shared<State>(*existing->get());

            // replace all objects of the old state with objects from the new state
            merge_base->update(std::move(view_state.state));

            // so we now have a combined new state
            view_state.state = std::move(merge_base);
        }
    }
}



std::vector<view_update> Transaction::generate_updates() {

    std::vector<view_update> updates;

    // try linearizing objects which have changed parents
    // and their children
    for (auto &view_state : this->states) {
        auto &view = view_state.view;
        auto &new_state = view_state.state;
        auto &tracker = view_state.changes;

        // update to perform for this view.
        view_update update;

        // from the known parent changes, find all affected children
        // affected are: children of those objects which the child cache knows.

        // contains all objects whose parents changed.
        std::unordered_set<fqon_t> objs_to_linearize;

        // take a look at all the needed inheritance updates
        // and generate the child tracking update from it.
        update.children = this->inheritance_updates(
            tracker,
            view,
            objs_to_linearize
        );

        try {
            update.linearizations = this->relinearize_objects(
                objs_to_linearize,
                view,
                new_state
            );
        }
        catch (C3Error &) {
            // this error is non-fatal but aborts the transaction
            this->set_error(std::current_exception());
            break;
        }

        updates.push_back(std::move(update));
    }

    return updates;
}


view_update::child_map_t
Transaction::inheritance_updates(const ChangeTracker &tracker,
                                 const std::shared_ptr<View> &view,
                                 std::unordered_set<fqon_t> &objs_to_linearize) const {

    // maps fqon => set of children fqons
    view_update::child_map_t children;

    // those objects were changed and require handling.
    for (auto &it : tracker.get_object_changes()) {
        auto &obj = it.first;
        auto &obj_changes = it.second;

        // the object has new parents.
        if (obj_changes.parents_update_required()) {

            // so we register the object at each parent as new child.
            // the previous children are merged with that set later.
            for (auto &parent : obj_changes.get_new_parents()) {
                auto ins = children.emplace(
                    parent,
                    std::unordered_set<fqon_t>{}
                );
                ins.first->second.insert(obj);
            }

            // as this object's parents changed,
            // it requires relinearization.
            objs_to_linearize.insert(obj);

            // children of this object need to be relinearized as well.
            const auto &obj_children = view->get_obj_children_all(obj, this->at);

            objs_to_linearize.insert(std::begin(obj_children),
                                     std::end(obj_children));
        }
    }

    return children;
}


view_update::linearizations_t
Transaction::relinearize_objects(const std::unordered_set<fqon_t> &objs_to_linearize,
                                 const std::shared_ptr<View> &view,
                                 const std::shared_ptr<State> &new_state) {

    view_update::linearizations_t linearizations;

    for (auto &obj : objs_to_linearize) {
        auto lin = linearize(
            obj,
            [this, &view, &new_state]
            (const fqon_t &name) -> const ObjectState & {

                // try to use the object in the new state if it's in there
                const auto &new_obj_state = new_state->get(name);
                if (new_obj_state != nullptr) {
                    return *new_obj_state->get();
                }

                // else, get it from the already existing view.
                const ObjectState *view_obj_state = view->get_raw(name, this->at).get();
                if (unlikely(view_obj_state == nullptr)) {
                    throw InternalError{"could not find parent object"};
                }
                return *view_obj_state;
            }
        );

        linearizations.push_back(std::move(lin));
    }

    return linearizations;
}


void Transaction::update_views(std::vector<view_update> &&updates) {
    size_t idx = 0;
    for (auto &view_state : this->states) {
        auto &view = view_state.view;
        auto &new_state = view_state.state;

        // record the state updates in each view's history
        StateHistory &view_history = view->get_state_history();

        // insert the new state and drop later ones.
        view_history.insert(std::move(new_state), this->at);

        // insert all newly calculated linearizations.
        for (auto &lin : updates[idx].linearizations) {
            view_history.insert_linearization(std::move(lin), this->at);
        }

        // inheritance updates can generate new children for existing objects
        for (auto &it : updates[idx].children) {
            auto &obj = it.first;
            auto &new_children = it.second;

            // merge the previous children with tne new child set
            const auto &previous_children = view->get_obj_children(obj, this->at);
            new_children.insert(std::begin(previous_children),
                                std::end(previous_children));

            view_history.insert_children(obj, std::move(new_children), this->at);
        }

        idx += 1;
    }

    // now that the views were updated, we can fire the event notifications.
    for (auto &view_state : this->states) {
        auto &view = view_state.view;
        auto &tracker = view_state.changes;

        std::unordered_set<fqon_t> updated_objects = tracker.get_changed_objects();
        std::unordered_set<fqon_t> affected_children;
        // all children of the patched objects are also affected.
        for (auto &obj : updated_objects) {
            std::unordered_set<fqon_t> children = view->get_obj_children_all(obj, this->at);
            affected_children.merge(children);
        }

        updated_objects.merge(affected_children);

        // TODO: if we don't want to fire for every object, but only
        //       for those with some members changed, we have to
        //       extend the ChangeTracker and track individual member updates
        //       (maybe only if the object is of interest).
        //       then we pass only the relevant objects here.
        view->fire_notifications(updated_objects, this->at);
    }
}


void Transaction::set_error(std::exception_ptr &&exc) {
    this->valid = false;
    this->error = std::move(exc);
}

} // namespace nyan
