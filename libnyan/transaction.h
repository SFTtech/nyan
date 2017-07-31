// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "config.h"


namespace nyan {

class Object;
class View;


/**
 * Patch transaction
 */
class Transaction {
public:
	struct patch_group {
		fqon_t target;
		std::vector<fqon_t> patches;
	};

	Transaction(order_t at, std::shared_ptr<View> &&origin);

	/**
	 * Add a patch to the transaction.
	 * Apply the patch to the target stored in the patch.
	 */
	bool add(const Object &obj);

	/**
	 * Add a patch to the transaction.
	 * Apply the patch to a custom target, which must be a
	 * child of the target stored in the patch.
	 */
	bool add(const Object &obj, const Object &target);

	bool commit();

private:
	bool commit_view(std::shared_ptr<View> &target_view);

	order_t at;
	std::shared_ptr<View> view;
	std::vector<patch_group> patches;
};

} // nyan
