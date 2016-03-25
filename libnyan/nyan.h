#ifndef NYAN_H_
#define NYAN_H_

/*
TODO: static access
 class LolObject : NyanObject {int health;};
 NyanObject u = LolObject{1337};
 int health = dynamic_cast<LolObject &>(u).health;

dynamic access:
 LolObject().get_attribute<int>("health");
*/

/**
 * The nyan engine and interpreter is defined in this namespace.
 */
namespace nyan {


/**
 * Data definition with members and inheritance.
 */
class NyanObject {
public:
	NyanObject() = default;
	virtual ~NyanObject() = default;

	virtual void patch(const NyanObject *top);

	std::string name;
	std::vector<const NyanObject *> parents;
	std::unordered_map<std::string, std::unique_ptr<NyanObject>> members;
};

class NyanPatch : public NyanObject {
	NyanPatch(NyanObject *target);
	virtual ~NyanPatch() = default;
};

// used for unassigned members that only have a type
class NyanNone : public NyanObject {};

class NyanInt : public NyanObject {
	int value;

	void patch(const NyanObject *top) override {
		// type check:
		ENSURE(typeid(this) == typeid(top), "can only patch same-type")
		this->value
	}
};

class NyanString : public NyanObject {};

class NyanFloat : public NyanObject {};

/*

nyanc:

villager.nyan:
Villager(Unit):
    vomit_velocity : int = 9001

-> produces
namespace villager {
class Villager : public NyanObject {
    Villager() : vomit_velocity{9001} {}
    int vomit_velocity;
    
    // TODO: patching code
}
}

*/

} // namespace nyan

#endif
