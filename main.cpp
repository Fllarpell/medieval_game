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
class Potion;
class Spell;
class Fighter;
class Archer;
class Wizard;
class Arsenal;
class MedicalBag;
class SpellBook;

static std::map<std::string, std::shared_ptr<Character>> characters;


template <typename T>
class PhysicalItem {
public:
    explicit PhysicalItem(std::string name) : name(std::move(name)) {}

    virtual ~PhysicalItem() = default;

    std::string getName() { return this->name; }

private:
    std::string name;
};

template <typename T>
class Container {
public:
    std::map<std::string, std::shared_ptr<T>> elements;

    Container() = default;
    ~Container() = default;

    void addItem(std::shared_ptr<T> newItem) {
        elements.insert({newItem->getName(), newItem});
    }

    std::shared_ptr<T> find(std::string itemName) {
        return elements.find(itemName)->second;
    }

    void removeItem(std::string itemName) {
        elements.erase(itemName);
    }

};

class Weapon : public PhysicalItem<Weapon> {
private:
    int damage;

public:
    Weapon(std::string weaponName, int damage) : damage(damage), PhysicalItem(std::move(weaponName)) {}

    [[nodiscard]] int getDamage() const { return this->damage; }

};


class Potion : public PhysicalItem<Potion> {
private:
    int healValue;

public:
    Potion(std::string potionName, int healValue) : healValue(healValue), PhysicalItem(std::move(potionName)) {}

    [[nodiscard]] int getHealValue() const { return this->healValue; }

};


class Spell : public PhysicalItem<Spell> {
private:
    std::vector<std::shared_ptr<Character>> allowedTargets;

public:

    Spell(std::string spellName, std::vector<std::shared_ptr<Character>> allowedTargets) : allowedTargets(std::move(allowedTargets)), PhysicalItem(std::move(spellName)) {}

    int getNumAllowedTargets() { return this->allowedTargets.size(); }

    std::vector<std::shared_ptr<Character>> getAllowedTargets() { return this->allowedTargets; }

};


class Arsenal : public Container<Weapon> {
public:
    Arsenal() = default;
    ~Arsenal() = default;
};


class MedicalBag : public Container<Potion> {
public:
    MedicalBag() = default;
    ~MedicalBag() = default;
};


class SpellBook : public Container<Spell> {
public:
    SpellBook() = default;
    ~SpellBook() = default;
};

class Character {
private:
    int healthPoints;
    std::string name;
    std::string class_type;

    std::shared_ptr<Arsenal> arsenal;
    std::shared_ptr<MedicalBag> medicalBag;
    std::shared_ptr<SpellBook> spellBook;

public:
    Character(const std::string& name, int initHP, const std::string& class_hero) : name(name), healthPoints(initHP), class_type(class_hero) {}
    Character(const std::string& name, int initHP, const std::string& class_hero,
              std::shared_ptr<Arsenal> arsenal, std::shared_ptr<MedicalBag> medicalBag,
              std::shared_ptr<SpellBook> spellBook) : name(name), healthPoints(initHP),
              class_type(class_hero), arsenal(arsenal), medicalBag(medicalBag), spellBook(spellBook) {}
    virtual ~Character() {
        arsenal.reset();
        medicalBag.reset();
        spellBook.reset();
    };

    [[nodiscard]] std::string getName() const { return this->name; }

    [[nodiscard]] int getHP() const { return this->healthPoints; }

    [[nodiscard]] std::string getClassType() const { return this->class_type; }

    void setHP(int HP) { this->healthPoints = HP; }

    void attack(std::shared_ptr<Character> target, std::string weaponName, std::ofstream &outputFile) {
        if (this->arsenal->elements.count(weaponName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        outputFile << this->getName() << " attacks " << target->getName() << " with their " << weaponName << "!" << std::endl;
        target->setHP(target->getHP()-this->arsenal->find(std::move(weaponName))->getDamage());
    }

    void drink(std::shared_ptr<Character> target, const std::string& potionName, std::ofstream &outputFile) {
        if (this->medicalBag->elements.count(potionName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        target->setHP(target->getHP()+this->medicalBag->find(potionName)->getHealValue());
        outputFile << target->getName() << " drinks " << potionName << " from " << this->getName() << "." << std::endl;
        this->medicalBag->removeItem(potionName);
    }

    void cast(std::shared_ptr<Character> target, const std::string& spellName, std::ofstream &outputFile) {
        if (this->spellBook->elements.count(spellName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        bool _allowed_to_cast = false;
        for (const auto & allowedTarget : this->spellBook->find(spellName)->getAllowedTargets()) {
            if (allowedTarget == target) {
                _allowed_to_cast = true;
                break;
            }
        }
        if (_allowed_to_cast) {
            target->setHP(0);
            outputFile << this->getName() << " casts " << spellName << " on " << target->getName() << "!" << std::endl;

        } else {
            outputFile << "Error caught" << std::endl;
            return;
        }
        this->spellBook->removeItem(spellName);
    }

    void showArsenal(std::ofstream &outputFile) {

        std::string _tempOutput_w;
        std::vector<std::string> sorted_temp_output;

        for (const auto& [key, value] : this->arsenal->elements) {
            sorted_temp_output.push_back(key + ":" + std::to_string(value->getDamage()) + " ");
        }

        sort(sorted_temp_output.begin(),sorted_temp_output.end());
        for (const auto & i : sorted_temp_output) {
            _tempOutput_w += i;
        }

        outputFile << _tempOutput_w << std::endl;
    }

    void showMedicalBag(std::ofstream &outputFile) {
        std::string _tempOutput_p;
        std::vector<std::string> sorted_temp_output;

        for (const auto& [key, value] : this->medicalBag->elements) {
            sorted_temp_output.push_back(key + ":" + std::to_string(value->getHealValue()) + " ");
        }

        sort(sorted_temp_output.begin(),sorted_temp_output.end());
        for (const auto & i : sorted_temp_output) {
            _tempOutput_p += i;
        }

        outputFile << _tempOutput_p << std::endl;
    }

    void showSpellBook(std::ofstream &outputFile) {

        std::string _tempOutput_s;
        std::vector<std::string> sorted_temp_output;

        for (const auto& [key, value] : this->spellBook->elements) {
            sorted_temp_output.push_back(key + ":" + std::to_string(value->getNumAllowedTargets()/2) + " ");
        }

        sort(sorted_temp_output.begin(),sorted_temp_output.end());
        for (const auto & i : sorted_temp_output) {
            _tempOutput_s += i;
        }

        outputFile << _tempOutput_s << std::endl;
    }

    bool is_full_container_weapons(std::ofstream &outputFile, std::shared_ptr<Weapon> weapon, int max_weapons) {
        if (this->arsenal->elements.size() >= max_weapons)  {
            outputFile << "Error caught" << std::endl;
            return false;
        }
        this->arsenal->addItem(std::move(weapon));
        return true;
    }

    bool is_full_container_potions(std::ofstream &outputFile, std::shared_ptr<Potion> potion, int max_potions) {
        if (this->medicalBag->elements.size() >= max_potions)  {
            outputFile << "Error caught" << std::endl;
            return false;
        }
        this->medicalBag->addItem(std::move(potion));
        return true;
    }

    bool is_full_container_spells(std::ofstream &outputFile, std::shared_ptr<Spell> spell, int max_spells) {
        if (this->spellBook->elements.size() >= max_spells)  {
            outputFile << "Error caught" << std::endl;
            return false;
        }
        this->spellBook->addItem(std::move(spell));
        return true;
    }

};


class Fighter : public Character {
public:

    Fighter(const std::string& name, int healthPoints) : Character(name, healthPoints, "fighter", std::make_shared<Arsenal>(), std::make_shared<MedicalBag>(), nullptr) {

    }
    ~Fighter() override = default;

    static const int MAX_ALLOWED_WEAPONS = 3;
    static const int MAX_ALLOWED_POTIONS = 5;

};


class Archer : public Character {
public:
    Archer(const std::string& name,
           int healthPoints) : Character(name, healthPoints, "archer",
                                         std::make_shared<Arsenal>(), std::make_shared<MedicalBag>(), std::make_shared<SpellBook>())
    {}
    ~Archer() override = default;

    static const int MAX_ALLOWED_WEAPONS = 2;
    static const int MAX_ALLOWED_POTIONS = 3;
    static const int MAX_ALLOWED_SPELLS  = 2;

};


class Wizard : public Character {
public:
    Wizard(const std::string& name,
           int healthPoints) : Character(name, healthPoints, "wizard", nullptr, std::make_shared<MedicalBag>(), std::make_shared<SpellBook>()) {}
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

        std::map<std::string, int> event;
        event["Dialogue"]   = 0;
        event["Create"]     = 1;
        event["Attack"]     = 2;
        event["Cast"]       = 3;
        event["Drink"]      = 4;
        event["Show"]       = 5;


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
        if (textLine[1] != "Narrator") {
            if (characters.count(textLine[1]) == 0) {
                outputFile << "Error caught" << std::endl;
                return;
            }
        }

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

            if (character[character_class] == 'f') {
                std::shared_ptr<Fighter> fighter = std::make_shared<Fighter>(character_name, character_HP);
                characters.insert({character_name, fighter});
            } else if (character[character_class] == 'w') {
                std::shared_ptr<Wizard> wizard = std::make_shared<Wizard>(character_name, character_HP);
                characters.insert({character_name, wizard});
            } else if (character[character_class] == 'a') {
                std::shared_ptr<Archer> archer = std::make_shared<Archer>(character_name, character_HP);
                characters.insert({character_name, archer});
            }

            outputFile << "A new " << character_class << " came to town, " << character_name << "." << std::endl;

        } else if (_created == "item") {
            try {
                std::string item_type                   = textLine[2];
                std::string item_owner                  = textLine[3];
                std::string item_name                   = textLine[4];
                int         item_unique_action_value    = std::stoi(textLine[5]);

                int max_weapons;
                int max_potions;
                int max_spells;

                if (characters.count(item_owner) == 0) {
                    outputFile << "Error caught" << std::endl;
                    return;
                }

                std::shared_ptr<Character> owner = characters.find(item_owner)->second;

                if(item[item_type] != 'p' && item[item_type] != 's') {
                    if (!(1 <= item_unique_action_value && item_unique_action_value <= 50)) {
                        outputFile << "Error caught" << std::endl;
                        return;
                    }
                }

                if (owner->getClassType() == "fighter") {
                    max_weapons = Fighter::MAX_ALLOWED_WEAPONS;
                    max_potions = Fighter::MAX_ALLOWED_POTIONS;
                } else if (owner->getClassType() == "archer") {
                    max_weapons = Archer::MAX_ALLOWED_WEAPONS;
                    max_potions = Archer::MAX_ALLOWED_POTIONS;
                    max_spells = Archer::MAX_ALLOWED_SPELLS;
                } else if (owner->getClassType() == "wizard") {
                    max_potions = Wizard::MAX_ALLOWED_POTIONS;
                    max_spells = Wizard::MAX_ALLOWED_SPELLS;
                }

                if (item[item_type] == 'w') {
                    if (owner->getClassType() == "wizard") {
                        outputFile << "Error caught" << std::endl;
                        return;
                    }
                    std::shared_ptr<Weapon> weapon = std::make_shared<Weapon>(item_name, item_unique_action_value);
                    if (!owner->is_full_container_weapons(outputFile, weapon, max_weapons)) {
                        return;
                    }
                } else if (item[item_type] == 'p') {
                    if (item_unique_action_value < 1) {
                        outputFile << "Error caught" << std::endl;
                        return;
                    }
                    std::shared_ptr<Potion> potion = std::make_shared<Potion>(item_name, item_unique_action_value);
                    if (!owner->is_full_container_potions(outputFile, potion, max_potions)) {
                        return;
                    }
                } else if (item[item_type] == 's') {
                    if (owner->getClassType() == "fighter") {
                        outputFile << "Error caught" << std::endl;
                        return;
                    }
                    if (item_unique_action_value > 0) {
                        std::vector<std::shared_ptr<Character>> targets(item_unique_action_value);
                        for (int i = 1; i < item_unique_action_value+1; ++i) {
                            if (characters.count(textLine[5+i]) == 0) {
                                outputFile << "Error caught" << std::endl;
                                return;
                            }
                            targets.push_back(characters.find(textLine[5+i])->second);
                        }
                        std::shared_ptr<Spell> spell = std::make_shared<Spell>(item_name, targets);
                        if (!owner->is_full_container_spells(outputFile, spell, max_spells)) {
                            return;
                        }
                    } else if (item_unique_action_value == 0) {
                        std::vector<std::shared_ptr<Character>> targets;
                        std::shared_ptr<Spell> spell = std::make_shared<Spell>(item_name, targets);
                        if (!owner->is_full_container_spells(outputFile, spell, max_spells)) {
                            return;
                        }
                    }

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
        if (characters.count(attackerName) == 0 || characters.count(targetName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        auto attackerID             = characters.find(attackerName);
        auto targetID               = characters.find(targetName);

        if (attackerID != characters.end() && targetID != characters.end()) {
            auto attacker = attackerID->second;
            auto target = targetID->second;

            if (attacker->getClassType() == "wizard") {
                outputFile << "Error caught" << std::endl;
                return;
            }

            if (attacker && target) {
                attacker->attack(target, weaponName, outputFile);
                if (target->getHP() <= 0) {
                    outputFile << target->getName() << " has died..." << std::endl;
                    characters.erase(targetName);
                    target->~Character();
                    target.reset();
                }
            }
        } else {
            outputFile << "Error caught" << std::endl;
            return;
        }
    }

    static void cast(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string casterName  = textLine[1];
        std::string targetName  = textLine[2];
        std::string castName    = textLine[3];
        if (characters.count(casterName) == 0 || characters.count(targetName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        auto casterID           = characters.find(casterName);
        auto targetID           = characters.find(targetName);

        if (casterID != characters.end() && targetID != characters.end()) {
            auto caster = casterID->second;
            auto target = targetID->second;

            if (caster->getClassType() == "fighter") {
                outputFile << "Error caught" << std::endl;
                return;
            }

            if (caster && target) {
                caster->cast(target, castName, outputFile);
                if (target->getHP() <= 0) {
                    outputFile << target->getName() << " has died..." << std::endl;
                    characters.erase(targetName);
                    target->~Character();
                    target.reset();
                }
            }
        } else {
            outputFile << "Error caught" << std::endl;
            return;
        }
    }

    static void drink(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string supplierName    = textLine[1];
        std::string drinkerName     = textLine[2];
        std::string potionName      = textLine[3];
        if (characters.count(supplierName) == 0 || characters.count(drinkerName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        auto supplierID             = characters.find(supplierName);
        auto drinkerID              = characters.find(drinkerName);

        if (supplierID != characters.end() && drinkerID != characters.end()) {
            auto supplier = supplierID->second;
            auto drinker = drinkerID->second;

            if (supplier && drinker) {
                supplier->drink(drinker, potionName, outputFile);
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
        if (types[type_show] != 'c') {
            if (characters.count(textLine[2]) == 0) {
                outputFile << "Error caught" << std::endl;
                return;
            }
        }
        try {
            if (types[type_show] == 'c') {
                std::string _tempOutput_c;
                std::vector<std::string> sorted_temp_output;

                sorted_temp_output.reserve(characters.size());
                for (const auto& [key, value] : characters) {
                    sorted_temp_output.push_back(key + ":" + value->getClassType() + ":" + std::to_string(value->getHP()) + " ");
                }

                sort(sorted_temp_output.begin(),sorted_temp_output.end());
                for (const auto & i : sorted_temp_output) {
                    _tempOutput_c += i;
                }

                outputFile << _tempOutput_c << std::endl;

            } else if (types[type_show] == 'w') {
                std::string owner_weapon_name = textLine[2];
                auto owner_w = characters.find(owner_weapon_name)->second;
                if (owner_w->getClassType() == "wizard") {
                    outputFile << "Error caught" << std::endl;
                    return;
                }
                owner_w->showArsenal(outputFile);

            } else if (types[type_show] == 'p') {
                std::string owner_potion_name = textLine[2];
                auto owner_p = characters.find(owner_potion_name)->second;
                owner_p->showMedicalBag(outputFile);

            } else if (types[type_show] == 's') {
                std::string owner_spell_name = textLine[2];
                auto owner_s = characters.find(owner_spell_name)->second;
                if (owner_s->getClassType() == "fighter") {
                    outputFile << "Error caught" << std::endl;
                    return;
                }
                owner_s->showSpellBook(outputFile);
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
