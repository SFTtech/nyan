// Copyright 2019-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_notifier.h"

#include "view.h"


namespace nyan {

ObjectNotifierHandle::ObjectNotifierHandle(const update_cb_t &func)
	:
	func{func} {}


void ObjectNotifierHandle::fire(order_t t, const fqon_t &fqon, const ObjectState &state) const {
	this->func(t, fqon, state);
}


ObjectNotifier::ObjectNotifier(const fqon_t &fqon,
                               const update_cb_t &func,
                               const std::shared_ptr<View> &view)
	:
	fqon{fqon},
	view{view},
	handle{std::make_shared<ObjectNotifierHandle>(func)} {}


ObjectNotifier::~ObjectNotifier() {
	this->view->deregister_notifier(this->fqon, this->handle);
}


const std::shared_ptr<ObjectNotifierHandle> &ObjectNotifier::get_handle() const {
	return this->handle;
}


} // namespace nyan
