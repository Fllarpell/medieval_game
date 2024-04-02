#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>

class Character;
class Weapon;
class Fighter;
class WeaponUser;
class PotionUser;
class SpellUser;
class Arsenal;

template <typename T>
class Container;


template <typename T>
class PhysicalItem {
protected:
    Character& getOwner() { return *owner; }

public:
    PhysicalItem(std::string name, std::shared_ptr<Character> owner) : name(std::move(name)), owner(std::move(owner)) {}

    virtual ~PhysicalItem() = default;

    virtual void use(std::shared_ptr<Character> target, std::ofstream &outputFile) {};

    std::string getName() { return this->name; }

    void setContainer (std::shared_ptr<Container<T>> box) {
        this->container = box;
    }

private:
    std::shared_ptr<Character> owner;
    std::string name;

    std::shared_ptr<Container<T>> container;
};


class Character {

public:
    explicit Character(std::string name, int initHP, std::string class_hero) : name(std::move(name)), healthPoints(initHP), class_type(std::move(class_hero)) {
        characters.insert({name, std::make_shared<Character>(*this)});
    }
    virtual ~Character() = default;

    [[nodiscard]] std::string getName() const { return this->name; }

    [[nodiscard]] int getHP() const { return this->healthPoints; }

    [[nodiscard]] std::string getClassType() const { return this->class_type; }

    void setHP(int HP) { this->healthPoints = HP; }

    static std::map<std::string, std::shared_ptr<Character>> getMapCharacters() {
        return characters;
    }

    static std::map<std::string, std::shared_ptr<Character>> characters;
private:
    int healthPoints;
    std::string name;
    std::string class_type;
};


class Weapon : public PhysicalItem<Weapon> {
public:
    Weapon(std::string weaponName, int damage, std::shared_ptr<Character> owner) : damage(damage), PhysicalItem(std::move(weaponName), std::move(owner)) {}

    [[nodiscard]] int getDamage() const { return this->damage; }

    void use(std::shared_ptr<Character> target, std::ofstream &outputFile) override {
        target->setHP(target->getHP()-getDamage());

        if (target->getHP() <= 0) {
            outputFile << target->getName() << " has died..." << std::endl;
            target->~Character();
        }
    }

private:
    int damage;

};


class Potion : public PhysicalItem<Potion> {

public:
    Potion(std::string potionName, int healValue, std::shared_ptr<Character> owner) : healValue(healValue), PhysicalItem(std::move(potionName), std::move(owner)) {}

    [[nodiscard]] int getHealValue() const { return this->healValue; }

    void use(std::shared_ptr<Character> target, std::ofstream &outputFile) override {
        target->setHP(target->getHP()+getHealValue());
    }

private:
    int healValue;
};


class Spell : public PhysicalItem<Spell> {

public:

    Spell(std::string spellName, std::vector<std::shared_ptr<Character>> allowedTargets, std::shared_ptr<Character> owner) : allowedTargets(std::move(allowedTargets)), PhysicalItem(std::move(spellName), std::move(owner)) {}

    int getNumAllowedTargets() { return this->allowedTargets.size(); }

    void use(std::shared_ptr<Character> target, std::ofstream &outputFile) override {
        bool _allowed_to_cast = false;
        for (int i = 0; i < allowedTargets.size(); ++i) {
            if (allowedTargets[i] == target) {
                _allowed_to_cast = true;
                break;
            }
        }
        if (_allowed_to_cast) {
            outputFile << target->getName() << " has died..." << std::endl;
            target->~Character();
        } else {
            outputFile << "Error caught" << std::endl;
        }
    }

private:
    std::vector<std::shared_ptr<Character>> allowedTargets;

};


template <typename T>
class Container : public PhysicalItem<T> {
public:
    explicit Container(std::string name, int max_capacity, const std::shared_ptr<Character>& owner) : capacity(max_capacity), PhysicalItem<T>(name, owner) {}
    ~Container() override = default;

    void addItem(std::shared_ptr<T> newItem) {
        elements.insert({newItem->getName(), newItem});
    }

    void removeItem(std::shared_ptr<T> newItem) {
        while (elements.begin != elements.end && elements.begin->second != newItem) { ++elements.begin; }

        if (elements.begin->second == newItem)
            elements.erase(elements.begin->first);
    }

    bool find(std::shared_ptr<T> item) {
        while (elements.begin != elements.end && elements.begin->second != item) { ++elements.begin; }

        if (elements.begin->second == item)
            return true;
        return false;
    }

    std::shared_ptr<T> find(std::string itemName) {
        return elements.find(itemName)->second;
    }

    void removeItem(std::string itemName) {
        elements.erase(itemName);
    }

    std::map<std::string, std::shared_ptr<T>> getElements() {
        return this->elements;
    }

private:
    int capacity;
    static std::map<std::string, std::shared_ptr<T>> elements;

};


class Arsenal : public Container<Weapon> {
public:
    explicit Arsenal(int max_capacity, const std::shared_ptr<Character>& owner, std::string name) : capacity(max_capacity), Container<Weapon>("container weapon" + name, max_capacity, owner) {}
    ~Arsenal() override = default;

private:
    int capacity;
};


class MedicalBag : public Container<Potion> {
public:
    explicit MedicalBag(int max_capacity, const std::shared_ptr<Character>& owner, std::string name) : capacity(max_capacity), Container<Potion>("container potion" + name, max_capacity, owner) {}
    ~MedicalBag() override = default;

private:
    int capacity;
};


class SpellBook : public Container<Spell> {
public:
    explicit SpellBook(int max_capacity, const std::shared_ptr<Character>& owner, std::string name) : capacity(max_capacity), Container<Spell>("container spell" + name, max_capacity, owner) {}
    ~SpellBook() override = default;

private:
    int capacity;
};


class WeaponUser : public Character, public Arsenal {
protected:
    std::shared_ptr<Arsenal> arsenal;
public:
    WeaponUser(std::string name, int healthPoints, int max_capacity, std::string class_hero) : Character(std::move(name), healthPoints, class_hero), Arsenal(max_capacity, Character::getMapCharacters().find(name)->second, name) {
    }
    ~WeaponUser() override = default;

    virtual void attack(std::shared_ptr<Character> target, std::string weaponName, std::ofstream &outputFile) {
        arsenal->find(std::move(weaponName))->use(std::move(target), outputFile);
    }

    std::shared_ptr<Arsenal> getArsenal() {return this->arsenal;}

};


class PotionUser : public Character, public MedicalBag {
protected:
    std::shared_ptr<MedicalBag> medicalBag;
public:
    PotionUser(std::string name, int healthPoints, int max_capacity, std::string class_hero) : Character(std::move(name), healthPoints, class_hero), MedicalBag(max_capacity, Character::getMapCharacters().find(name)->second, name) {
    }
    ~PotionUser() override = default;

    void drink(std::shared_ptr<Character> target, const std::string& potionName, std::ofstream &outputFile) {
        medicalBag->find(potionName)->use(std::move(target), outputFile);
        medicalBag->removeItem(potionName);
    }

    std::shared_ptr<MedicalBag> getMedicalBag() {return this->medicalBag;}

};


class SpellUser : public Character, public SpellBook {
protected:
    std::shared_ptr<SpellBook> spellBook;
public:
    SpellUser(std::string name, int healthPoints, int max_capacity, std::string class_hero) : Character(std::move(name), healthPoints, class_hero), SpellBook(max_capacity, Character::getMapCharacters().find(name)->second, name) {
    }
    ~SpellUser() override = default;

    void cast(std::shared_ptr<Character> target, const std::string& spellName, std::ofstream &outputFile) {
        spellBook->find(spellName)->use(std::move(target), outputFile);
        spellBook->removeItem(spellName);
    }

    std::shared_ptr<SpellBook> getSpellBook() {return this->spellBook;}
};


class Fighter : public WeaponUser, public PotionUser {
public:
    Fighter(const std::string& name, int healthPoints) : WeaponUser(name, healthPoints, MAX_ALLOWED_WEAPONS, "fighter"), PotionUser(name, healthPoints, MAX_ALLOWED_POTIONS, "fighter") {}
    ~Fighter() override = default;

    static const int MAX_ALLOWED_WEAPONS = 3;
    static const int MAX_ALLOWED_POTIONS = 5;
};


class Archer : public WeaponUser, public PotionUser, public SpellUser {
public:
    Archer(const std::string& name, int healthPoints) : WeaponUser(name, healthPoints, MAX_ALLOWED_WEAPONS, "archer"), PotionUser(name, healthPoints, MAX_ALLOWED_POTIONS, "archer"), SpellUser(name, healthPoints, MAX_ALLOWED_SPELLS, "archer") {}
    ~Archer() override = default;

    static const int MAX_ALLOWED_WEAPONS = 2;
    static const int MAX_ALLOWED_POTIONS = 3;
    static const int MAX_ALLOWED_SPELLS  = 2;
};


class Wizard : public PotionUser, public SpellUser {
public:
    Wizard(const std::string& name, int healthPoints) : PotionUser(name, healthPoints, MAX_ALLOWED_POTIONS, "wizard"), SpellUser(name, healthPoints, MAX_ALLOWED_SPELLS, "wizard") {}
    ~Wizard() override = default;

    static const int MAX_ALLOWED_POTIONS = 10;
    static const int MAX_ALLOWED_SPELLS  = 10;
};


class Narrator {
public:
    static void dialogue(const std::string& speaker, const std::vector<std::string>& speech, std::ofstream &outputFile) {
        outputFile << speaker << ": ";
        for (const auto& s : speech) {
            outputFile << s << " ";
        }
        outputFile << std::endl;
    }
};


class TextAnalyzer{
public:
    static void eventProcessing(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string _event_type = textLine[0];

        std::map <std::string, int> event;
        event["Dialogue"] = 0;
        event["Create"]   = 1;
        event["Attack"]   = 2;
        event["Cast"]     = 3;
        event["Drink"]    = 4;
        event["Show"]     = 5;

        switch (event[_event_type]) {
            case 0: dialogue(textLine, outputFile); break;
            case 1: create(textLine, outputFile);   break;
            case 2: attack(textLine, outputFile);   break;
            case 3: cast(textLine, outputFile);     break;
            case 4: drink(textLine, outputFile);    break;
            case 5: show(textLine, outputFile);     break;
        }
    }

private:
    static void dialogue(std::vector<std::string> textLine, std::ofstream &outputFile) {
        if (!(4 <= textLine.size() && textLine.size() <= 13)) {
            outputFile << "Error caught" << std::endl;
            return;
        }

        std::string speaker = textLine[1];
        std::vector<std::string> speech(textLine.begin() + 3, textLine.end());

        Narrator::dialogue(speaker, speech, outputFile);
    }

    static void create(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string _created = textLine[1];

        std::map <std::string, char> character;
        character["fighter"] = 'f';
        character["wizard"]  = 'w';
        character["archer"]  = 'a';

        std::map <std::string, char> item;
        item["weapon"] = 'w';
        item["potion"] = 'p';
        item["spell"]  = 's';

        if (_created == "character") {
            std::string character_class = textLine[2];
            std::string character_name  = textLine[3];
            int         character_HP    = std::stoi(textLine[4]);

            switch (character[character_class]) {
                case 'f': std::make_shared<Fighter>(character_name, character_HP);  break;
                case 'w': std::make_shared<Wizard>(character_name, character_HP);   break;
                case 'a': std::make_shared<Archer>(character_name, character_HP);   break;
            }

            outputFile << "A new " << character_class << " came to town, " << character_name << "." << std::endl;

        } else if (_created == "item") {
            try {
                std::string item_type                   = textLine[2];
                std::string item_owner                  = textLine[3];
                std::string item_name                   = textLine[4];
                int         item_unique_action_value    = std::stoi(textLine[5]);
                std::shared_ptr<Character> owner        = Character::getMapCharacters().find(item_owner)->second;

                if (!(1 <= item_unique_action_value && item_unique_action_value <= 50)) {
                    outputFile << "Error caught" << std::endl;
                    return;
                }

                switch (character[item_type]) {
                    WeaponUser *weaponUser;
                    PotionUser *potionUser;
                    SpellUser *spellUser;
                    case 'w':
                        weaponUser = (WeaponUser *) &owner;
                        if (owner->getClassType() == "wizard") {
                            outputFile << "Error caught" << std::endl;
                            break;
                        }
                        if (owner->getClassType() == "fighter") {
                            if (weaponUser->getArsenal()->getElements().size() >= Fighter::MAX_ALLOWED_WEAPONS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        } else if (owner->getClassType() == "archer") {
                            if (weaponUser->getArsenal()->getElements().size() >= Archer::MAX_ALLOWED_WEAPONS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        }

                        weaponUser->getArsenal()->addItem(std::make_shared<Weapon>(item_name, item_unique_action_value, owner));
                        break;

                    case 'p':
                        potionUser = (PotionUser *) &owner;
                        if (owner->getClassType() == "fighter") {
                            if (potionUser->getMedicalBag()->getElements().size() >= Fighter::MAX_ALLOWED_POTIONS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        } else if (owner->getClassType() == "archer") {
                            if (potionUser->getMedicalBag()->getElements().size() >= Archer::MAX_ALLOWED_POTIONS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        } else if (owner->getClassType() == "wizard") {
                            if (potionUser->getMedicalBag()->getElements().size() >= Wizard::MAX_ALLOWED_POTIONS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        }

                        potionUser->getMedicalBag()->addItem(std::make_shared<Potion>(item_name, item_unique_action_value, owner));
                        break;

                    case 's':
                        spellUser = (SpellUser *) &owner;
                        if (owner->getClassType() == "fighter") {
                            outputFile << "Error caught" << std::endl;
                            break;
                        }
                        if (owner->getClassType() == "archer") {
                            if (spellUser->getSpellBook()->getElements().size() >= Archer::MAX_ALLOWED_SPELLS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        } else if (owner->getClassType() == "wizard") {
                            if (spellUser->getSpellBook()->getElements().size() >= Wizard::MAX_ALLOWED_SPELLS)  {
                                outputFile << "Error caught" << std::endl;
                                break;
                            }
                        }

                        std::vector<std::shared_ptr<Character>> targets(item_unique_action_value);
                        for (int i = 1; i < item_unique_action_value+1; ++i) {
                            targets.push_back(Character::getMapCharacters().find(textLine[5+i])->second);
                        }
                        spellUser->getSpellBook()->addItem(std::make_shared<Spell>(item_name, targets, owner));
                        break;
                }

                outputFile << item_owner << " just obtained a new " << item_type << " called " << item_name << "." << std::endl;

            } catch (...) {
                outputFile << "Error caught" << std::endl;
            }
        }
    }

    static void attack(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string attackerName    = textLine[1];
        std::string targetName      = textLine[2];
        std::string weaponName      = textLine[3];
        auto attackerID             = Character::characters.find(attackerName);
        auto targetID               = Character::characters.find(targetName);

        if (attackerID != Character::getMapCharacters().end() && targetID != Character::getMapCharacters().end()) {
            auto attacker = attackerID->second;
            auto target = targetID->second;
            auto attackFighter = (WeaponUser*)&attacker;

            if (attacker && target) {
                attackFighter->attack(target, weaponName, outputFile);
                outputFile << attacker->getName() << " attacks " << target->getName() << " with their " << weaponName << "!" << std::endl;
            }
        } else {
            outputFile << "Error caught" << std::endl;
        }


    }

    static void cast(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string casterName  = textLine[1];
        std::string targetName  = textLine[2];
        std::string castName    = textLine[3];
        auto casterID           = Character::getMapCharacters().find(casterName);
        auto targetID           = Character::getMapCharacters().find(targetName);

        if (casterID != Character::getMapCharacters().end() && targetID != Character::getMapCharacters().end()) {
            auto caster = casterID->second;
            auto target = targetID->second;
            auto casterWizard = (SpellUser*)&caster;

            if (caster && target) {
                casterWizard->cast(target, castName, outputFile);
                outputFile << caster->getName() << " casts " << castName << " on " << target->getName() << "!" << std::endl;
            }
        } else {
            outputFile << "Error caught" << std::endl;
        }
    }

    static void drink(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string supplierName    = textLine[1];
        std::string drinkerName     = textLine[2];
        std::string potionName      = textLine[3];
        auto supplierID             = Character::getMapCharacters().find(supplierName);
        auto drinkerID              = Character::getMapCharacters().find(drinkerName);

        if (supplierID != Character::getMapCharacters().end() && drinkerID != Character::getMapCharacters().end()) {
            auto supplier = supplierID->second;
            auto drinker = drinkerID->second;
            auto drinkerUser = (PotionUser*)&supplier;

            if (supplier && drinker) {
                drinkerUser->drink(drinker, potionName, outputFile);
                outputFile << supplier->getName() << " drinks " << potionName << " from " << drinker->getName() << "!" << std::endl;
            }
        } else {
            outputFile << "Error caught" << std::endl;
        }
    }

    static void show(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string type_show = textLine[1];

        std::map<std::string, char> types;
        types["characters"]  = 'c';
        types["weapons"]     = 'w';
        types["potions"]     = 'p';
        types["spells"]      = 's';
        try {
            if (types[type_show] == 'c') {
                std::_Rb_tree_iterator<std::pair<const std::basic_string<char>, std::shared_ptr<Character>>> iter_c = Character::getMapCharacters().begin();
                std::string _tempOutput_c;
                std::vector<std::string> sorted_temp_output;
                while (iter_c != Character::getMapCharacters().end())
                {
                    sorted_temp_output.push_back(iter_c->second->getName() + ":" + iter_c->second->getClassType() + ":" + std::to_string(iter_c->second->getHP()) + " ");
                }
                sort(sorted_temp_output.begin(),sorted_temp_output.end());
                for (int i = 0; i < sorted_temp_output.size(); ++i) {
                    _tempOutput_c += sorted_temp_output[i];
                }

                outputFile << _tempOutput_c << std::endl;

            } else if (types[type_show] == 'w') {
                std::string owner_weapon_name = textLine[2];
                auto owner_w = (WeaponUser*)&Character::getMapCharacters().find(owner_weapon_name)->second;
                std::_Rb_tree_iterator<std::pair<const std::basic_string<char>, std::shared_ptr<Weapon>>> iter_w = owner_w->getArsenal()->getElements().begin();
                std::string _tempOutput_w;
                std::vector<std::string> sorted_temp_output;
                while (iter_w != owner_w->getArsenal()->getElements().end())
                {
                    auto pWeapon = (Weapon*)&iter_w->second;
                    int damage = pWeapon->getDamage();
                    sorted_temp_output.push_back(iter_w->second->getName() + ":" + std::to_string(damage) + " ");
                }
                sort(sorted_temp_output.begin(),sorted_temp_output.end());
                for (int i = 0; i < sorted_temp_output.size(); ++i) {
                    _tempOutput_w += sorted_temp_output[i];
                }

                outputFile << _tempOutput_w << std::endl;

            } else if (types[type_show] == 'p') {
                std::string owner_potion_name = textLine[2];
                auto owner_p = (PotionUser*)&Character::getMapCharacters().find(owner_potion_name)->second;
                std::_Rb_tree_iterator<std::pair<const std::basic_string<char>, std::shared_ptr<Potion>>> iter_p = owner_p->getMedicalBag()->getElements().begin();
                std::string _tempOutput_p;
                std::vector<std::string> sorted_temp_output;
                while (iter_p != owner_p->getMedicalBag()->getElements().end())
                {
                    auto pPotion = (Potion*)&iter_p->second;
                    int healValue = pPotion->getHealValue();
                    sorted_temp_output.push_back(iter_p->second->getName() + ":" + std::to_string(healValue) + " ");
                }
                sort(sorted_temp_output.begin(),sorted_temp_output.end());
                for (int i = 0; i < sorted_temp_output.size(); ++i) {
                    _tempOutput_p += sorted_temp_output[i];
                }

                outputFile << _tempOutput_p << std::endl;

            } else if (types[type_show] == 's') {
                std::string owner_spell_name = textLine[2];
                auto owner_s = (SpellUser*)&Character::getMapCharacters().find(owner_spell_name)->second;
                std::_Rb_tree_iterator<std::pair<const std::basic_string<char>, std::shared_ptr<Spell>>> iter_s = owner_s->getSpellBook()->getElements().begin();
                std::string _tempOutput_s;
                std::vector<std::string> sorted_temp_output;
                while (iter_s != owner_s->getSpellBook()->getElements().end())
                {
                    auto pSpell = (Spell*)&iter_s->second;
                    int allowedTargets = pSpell->getNumAllowedTargets();
                    sorted_temp_output.push_back(iter_s->second->getName() + ":" + std::to_string(allowedTargets) + " ");
                }
                sort(sorted_temp_output.begin(),sorted_temp_output.end());
                for (int i = 0; i < sorted_temp_output.size(); ++i) {
                    _tempOutput_s += sorted_temp_output[i];
                }

                outputFile << _tempOutput_s << std::endl;
            }

        } catch (...) {
            outputFile << "Error caught" << std::endl;
        }
    }
};


std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}


int main() {
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    std::string line;

    int numEvents;
    inputFile >> numEvents;
    inputFile.ignore();

    while (std::getline(inputFile, line)) {
        std::vector<std::string> tokens = split(line);
        TextAnalyzer::eventProcessing(tokens, outputFile);
    }

    inputFile.close();
    outputFile.close();

    return 0;
}

template<typename T>
std::map<std::string, std::shared_ptr<T>> Container<T>::elements;

std::map<std::string, std::shared_ptr<Character>> Character::characters;
