#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>

// Forward declarations of classes to resolve circular dependencies
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

// Global map to store pointers to the characters
static std::map<std::string, std::shared_ptr<Character>> characters;

// Template class for physical items
template <typename T>
class PhysicalItem {
public:
    // Constructor and Destructor
    explicit PhysicalItem(std::string name) : name(std::move(name)) {}
    virtual ~PhysicalItem() = default;

    // Getter of the name of the item
    std::string getName() { return this->name; }

private:
    std::string name;
};

// Template base class for containers
template <typename T>
class Container {
public:
    // map to store items of different categories of the items
    std::map<std::string, std::shared_ptr<T>> elements;

    // Constructor and Destructor
    Container() = default;
    ~Container() = default;

    // Add the item into the map elements
    void addItem(std::shared_ptr<T> newItem) {
        elements.insert({newItem->getName(), newItem});
    }

    // Find the item in the map elements
    std::shared_ptr<T> find(std::string itemName) {
        return elements.find(itemName)->second;
    }

    // Remove the item from the map elements
    void removeItem(std::string itemName) {
        elements.erase(itemName);
    }

};

// Class for weapons, inheriting from PhysicalItem
class Weapon : public PhysicalItem<Weapon> {
private:
    int damage;

public:
    // Constructor
    Weapon(std::string weaponName, int damage) : damage(damage), PhysicalItem(std::move(weaponName)) {}

    // Getter of the damage of the weapon
    [[nodiscard]] int getDamage() const { return this->damage; }

};

// Class for potions, inheriting from PhysicalItem
class Potion : public PhysicalItem<Potion> {
private:
    int healValue;

public:
    // Constructor
    Potion(std::string potionName, int healValue) : healValue(healValue), PhysicalItem(std::move(potionName)) {}

    // Getter of the heal value of the potion
    [[nodiscard]] int getHealValue() const { return this->healValue; }

};

// Class for spells, inheriting from PhysicalItem
class Spell : public PhysicalItem<Spell> {
private:
    // list that keep allowed targets
    std::vector<std::shared_ptr<Character>> allowedTargets;

public:
    // Constructor
    Spell(std::string spellName, std::vector<std::shared_ptr<Character>> allowedTargets) : allowedTargets(std::move(allowedTargets)), PhysicalItem(std::move(spellName)) {}

    // Getter of available targets of the spell
    int getNumAllowedTargets() { return this->allowedTargets.size(); }

    // Getter of the list of all targets of the spell
    std::vector<std::shared_ptr<Character>> getAllowedTargets() { return this->allowedTargets; }

};

// Class for keeping weapons, inheriting from Container
class Arsenal : public Container<Weapon> {
public:
    // Constructor and Destructor
    Arsenal() = default;
    ~Arsenal() = default;
};

// Class for keeping potions, inheriting from Container
class MedicalBag : public Container<Potion> {
public:
    // Constructor and Destructor
    MedicalBag() = default;
    ~MedicalBag() = default;
};

// Class for keeping spells, inheriting from Container
class SpellBook : public Container<Spell> {
public:
    // Constructor and Destructor
    SpellBook() = default;
    ~SpellBook() = default;
};

// Base class of characters
class Character {
private:
    int healthPoints;
    std::string name;
    std::string class_type;

    // Pointers to arsenal, medical bag, and book of the spells
    std::shared_ptr<Arsenal> arsenal;
    std::shared_ptr<MedicalBag> medicalBag;
    std::shared_ptr<SpellBook> spellBook;

public:
    // Constructors and Destructor
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

    // Getter of the name of the character
    [[nodiscard]] std::string getName() const { return this->name; }

    // Getter of the health points of the character
    [[nodiscard]] int getHP() const { return this->healthPoints; }

    // Getter of the class type of the character
    [[nodiscard]] std::string getClassType() const { return this->class_type; }

    // Setter of the health points to the character
    void setHP(int HP) { this->healthPoints = HP; }

    // Attack character if it possible and write in the file
    void attack(std::shared_ptr<Character> target, std::string weaponName, std::ofstream &outputFile) {
        if (this->arsenal->elements.count(weaponName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        outputFile << this->getName() << " attacks " << target->getName() << " with their " << weaponName << "!" << std::endl;
        target->setHP(target->getHP()-this->arsenal->find(std::move(weaponName))->getDamage());
    }

    // Attack the potion if it possible and write in the file
    void drink(std::shared_ptr<Character> target, const std::string& potionName, std::ofstream &outputFile) {
        if (this->medicalBag->elements.count(potionName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        target->setHP(target->getHP()+this->medicalBag->find(potionName)->getHealValue());
        outputFile << target->getName() << " drinks " << potionName << " from " << this->getName() << "." << std::endl;
        this->medicalBag->removeItem(potionName);
    }

    // Cast spell to the character if it possible and write in the file
    void cast(std::shared_ptr<Character> target, const std::string& spellName, std::ofstream &outputFile) {
        if (this->spellBook->elements.count(spellName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        // Check allowed targets
        bool _allowed_to_cast = false;
        for (const auto & allowedTarget : this->spellBook->find(spellName)->getAllowedTargets()) {
            if (allowedTarget == target) {
                _allowed_to_cast = true;
                break;
            }
        }
        // Kill the target if it exists in the list of allowed targets
        if (_allowed_to_cast) {
            target->setHP(0);
            outputFile << this->getName() << " casts " << spellName << " on " << target->getName() << "!" << std::endl;

        } else {
            outputFile << "Error caught" << std::endl;
            return;
        }
        this->spellBook->removeItem(spellName);
    }

    // Display sorted weapons of characters if it possible
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

    // Display sorted potions of characters if it possible
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

    // Display sorted spells of characters if it possible
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

    // Function check capacity of container and add weapon into the arsenal
    bool is_full_container_weapons(std::ofstream &outputFile, std::shared_ptr<Weapon> weapon, int max_weapons) {
        if (this->arsenal->elements.size() >= max_weapons)  {
            outputFile << "Error caught" << std::endl;
            return false;
        }
        this->arsenal->addItem(std::move(weapon));
        return true;
    }

    // Function check capacity of container and add potion into the medicalBag
    bool is_full_container_potions(std::ofstream &outputFile, std::shared_ptr<Potion> potion, int max_potions) {
        if (this->medicalBag->elements.size() >= max_potions)  {
            outputFile << "Error caught" << std::endl;
            return false;
        }
        this->medicalBag->addItem(std::move(potion));
        return true;
    }

    // Function check capacity of container and add spell into the spellBook
    bool is_full_container_spells(std::ofstream &outputFile, std::shared_ptr<Spell> spell, int max_spells) {
        if (this->spellBook->elements.size() >= max_spells)  {
            outputFile << "Error caught" << std::endl;
            return false;
        }
        this->spellBook->addItem(std::move(spell));
        return true;
    }
};

// Subclass for fighters, inheriting from Character
class Fighter : public Character {
public:
    // Constructor and Destructor
    Fighter(const std::string& name, int healthPoints) : Character(name, healthPoints, "fighter", std::make_shared<Arsenal>(), std::make_shared<MedicalBag>(), nullptr) {

    }
    ~Fighter() override = default;

    // Constants for maximum allowed weapons and potions
    static const int MAX_ALLOWED_WEAPONS = 3;
    static const int MAX_ALLOWED_POTIONS = 5;

};

// Subclass for archers, inheriting from Character
class Archer : public Character {
public:
    // Constructor and Destructor
    Archer(const std::string& name,
           int healthPoints) : Character(name, healthPoints, "archer",
                                         std::make_shared<Arsenal>(), std::make_shared<MedicalBag>(), std::make_shared<SpellBook>())
    {}
    ~Archer() override = default;

    // Constants for maximum allowed weapons, potions, and spells
    static const int MAX_ALLOWED_WEAPONS = 2;
    static const int MAX_ALLOWED_POTIONS = 3;
    static const int MAX_ALLOWED_SPELLS  = 2;

};

// Subclass for wizards, inheriting from Character
class Wizard : public Character {
public:
    // Constructor and Destructor
    Wizard(const std::string& name,
           int healthPoints) : Character(name, healthPoints, "wizard", nullptr, std::make_shared<MedicalBag>(), std::make_shared<SpellBook>()) {}
    ~Wizard() override = default;

    // Constants for maximum allowed potions and spells
    static const int MAX_ALLOWED_POTIONS = 10;
    static const int MAX_ALLOWED_SPELLS  = 10;

};

// Class for the Narrator, responsible for displaying dialogue
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

// Class for analyzing text events
class TextAnalyzer{
public:
    // Processing event in the line from the file
    static void eventProcessing(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string _event_type = textLine[0];

        // Check present event
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
    // Display phrase of the character
    static void dialogue(std::vector<std::string> textLine, std::ofstream &outputFile) {
        // Check existing of the character
        if (textLine[1] != "Narrator") {
            if (characters.count(textLine[1]) == 0) {
                outputFile << "Error caught" << std::endl;
                return;
            }
        }
        // Check length of the phrase
        if (!(4 <= textLine.size() && textLine.size() <= 13)) {
            outputFile << "Error caught" << std::endl;
            return;
        }

        std::string speaker = textLine[1];
        std::vector<std::string> speech(textLine.begin() + 3, textLine.end());

        Narrator::dialogue(speaker, speech, outputFile);
    }

    // Creating character or item
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
        // Check class type of the character and write him in the map characters
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

                // Check existing of the character
                if (characters.count(item_owner) == 0) {
                    outputFile << "Error caught" << std::endl;
                    return;
                }

                std::shared_ptr<Character> owner = characters.find(item_owner)->second;

                // Check class type of the character
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
                // Create weapon, potion or spell if it possible
                if (item[item_type] == 'w') {
                    if (owner->getClassType() == "wizard") {
                        outputFile << "Error caught" << std::endl;
                        return;
                    }
                    if (!(1 <= item_unique_action_value && item_unique_action_value <= 50)) {
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

    // Attack the character
    static void attack(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string attackerName    = textLine[1];
        std::string targetName      = textLine[2];
        std::string weaponName      = textLine[3];
        // Check existing of attacker or target
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
            // Calling function attack and check dead the character or not
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

    // Cast the spell to the character
    static void cast(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string casterName  = textLine[1];
        std::string targetName  = textLine[2];
        std::string castName    = textLine[3];
        // Check existing of attacker or target
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
            // Calling function cast and kill the target
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

    // Drink the potion
    static void drink(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string supplierName    = textLine[1];
        std::string drinkerName     = textLine[2];
        std::string potionName      = textLine[3];
        // Check existing of attacker or target
        if (characters.count(supplierName) == 0 || characters.count(drinkerName) == 0) {
            outputFile << "Error caught" << std::endl;
            return;
        }
        auto supplierID             = characters.find(supplierName);
        auto drinkerID              = characters.find(drinkerName);
        // Calling drink function for healing
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

    // Display characters, weapons, potions, or spells
    static void show(std::vector<std::string> textLine, std::ofstream &outputFile) {
        std::string type_show = textLine[1];
        // Check which type of showing was called
        std::map<std::string, char> types;
        types["characters"]  = 'c';
        types["weapons"]     = 'w';
        types["potions"]     = 'p';
        types["spells"]      = 's';

        // Check existing of the character
        if (types[type_show] != 'c') {
            if (characters.count(textLine[2]) == 0) {
                outputFile << "Error caught" << std::endl;
                return;
            }
        }
        // Calling necessary function for display characters or items, sorting it and then display it in the file
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

// Function to split a string into words
std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Main function
int main() {
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    std::string line;

    int numEvents;
    inputFile >> numEvents;
    inputFile.ignore();

    // Process each event in the input file
    while (std::getline(inputFile, line)) {
        std::vector<std::string> tokens = split(line);
        TextAnalyzer::eventProcessing(tokens, outputFile);
    }

    inputFile.close();
    outputFile.close();

    return 0;
}
