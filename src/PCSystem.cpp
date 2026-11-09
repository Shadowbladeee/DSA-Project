#include "../include/PCSystem.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <functional>

using namespace std;

PCSystem::PCSystem() : boxes(MAX_BOXES), currentBoxIndex(0)
{
    for (int i = 0; i < MAX_BOXES; i++)
        boxes[i].name = "Box " + to_string(i + 1);
}

bool PCSystem::switchBox(int boxNumber)
{
    if (boxNumber < 1 || boxNumber > MAX_BOXES)
    {
        cout << "[ERROR] Invalid box number! Choose between 1 and " << MAX_BOXES << "." << endl;
        return false;
    }
    currentBoxIndex = boxNumber - 1;
    cout << "Switched to [" << boxes[currentBoxIndex].name << "]" << endl;
    return true;
}

bool PCSystem::renameCurrentBox(const string &newName)
{
    if (newName.empty())
    {
        cout << "[ERROR] Box name cannot be empty." << endl;
        return false;
    }
    cout << "Renamed [" << boxes[currentBoxIndex].name << "] to [" << newName << "]" << endl;
    boxes[currentBoxIndex].name = newName;
    return true;
}

void PCSystem::displayCurrentBox() const
{
    cout << "\n=== " << boxes[currentBoxIndex].name << " (" << boxes[currentBoxIndex].list.getCount() << "/" << BOX_CAPACITY << ") ===" << endl;
    boxes[currentBoxIndex].list.displayBox();
}

void PCSystem::listAllBoxes() const
{
    cout << "\n================ ALL PC BOXES ================" << endl;
    for (int i = 0; i < MAX_BOXES; i++)
    {
        cout << (i + 1) << ". " << boxes[i].name << " [" << boxes[i].list.getCount() << "/" << BOX_CAPACITY << "]";
        if (i == currentBoxIndex)
        {
            cout << "  <-- (Active)";
        }
        cout << endl;
    }
    cout << "==============================================" << endl;
}

bool PCSystem::moveBetweenBoxes(int sourceBox, int targetBox, int instanceID)
{
    int srcIdx = sourceBox - 1;
    int tgtIdx = targetBox - 1;

    if (srcIdx < 0 || srcIdx >= MAX_BOXES || tgtIdx < 0 || tgtIdx >= MAX_BOXES)
        return false;
    if (boxes[tgtIdx].list.getCount() >= BOX_CAPACITY)
        return false;

    PokemonNode *moved = boxes[srcIdx].list.removeByID(instanceID);
    if (moved == nullptr)
        return false;

    boxes[tgtIdx].list.insertNode(moved);
    return true;
}

bool PCSystem::moveBetweenBoxesByPosition(int sourceBox, int targetBox, int slotPosition)
{
    int srcIdx = sourceBox - 1;
    int tgtIdx = targetBox - 1;

    if (srcIdx < 0 || srcIdx >= MAX_BOXES || tgtIdx < 0 || tgtIdx >= MAX_BOXES)
        return false;
    if (boxes[tgtIdx].list.getCount() >= BOX_CAPACITY)
        return false;

    PokemonNode *moved = boxes[srcIdx].list.removeByPosition(slotPosition);
    if (moved == nullptr)
        return false;

    boxes[tgtIdx].list.insertNode(moved);
    return true;
}

static string toLower(string s)
{
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

vector<SearchResult> PCSystem::searchIf(const function<bool(const PokemonNode &)> &pred)
{
    vector<SearchResult> results;
    for (int i = 0; i < MAX_BOXES; i++)
    {
        int slot = 1;
        for (PokemonNode *cur = boxes[i].list.getHead(); cur != nullptr; cur = cur->next, slot++)
        {
            if (!pred(*cur))
                continue;

            SearchResult r;
            r.boxNumber = i + 1;
            r.boxName = boxes[i].name;
            r.slot = slot;
            r.nickname = cur->nickname;
            if (cur->species)
            {
                r.speciesName = cur->species->name;
                r.type1 = cur->species->type1;
                r.type2 = cur->species->type2;
                r.generation = cur->species->generation;
                r.dexNumber = cur->species->pokedexNumber;
            }
            results.push_back(r);
        }
    }
    return results;
}

vector<SearchResult> PCSystem::search(const string &query)
{
    const string q = toLower(query);
    if (q.empty())
        return {};

    return searchIf([&](const PokemonNode &p) {
        string species = p.species ? p.species->name : "";
        return toLower(p.nickname).find(q) != string::npos ||
               toLower(species).find(q) != string::npos;
    });
}

vector<SearchResult> PCSystem::searchByType(const string &type)
{
    const string t = toLower(type);
    if (t.empty())
        return {};

    return searchIf([&](const PokemonNode &p) {
        return p.species &&
               (toLower(p.species->type1) == t || toLower(p.species->type2) == t);
    });
}

vector<SearchResult> PCSystem::searchByGeneration(int generation)
{
    return searchIf([generation](const PokemonNode &p) {
        return p.species && p.species->generation == generation;
    });
}

vector<SearchResult> PCSystem::searchByDexNumber(int dexNumber)
{
    return searchIf([dexNumber](const PokemonNode &p) {
        return p.species && p.species->pokedexNumber == dexNumber;
    });
}

vector<SearchResult> PCSystem::searchByTypePair(const string &type1, const string &type2)
{
    const string a = toLower(type1);
    const string b = toLower(type2);
    if (a.empty() || b.empty())
        return {};

    return searchIf([a, b](const PokemonNode &p) {
        if (!p.species)
            return false;
        string x = toLower(p.species->type1);
        string y = toLower(p.species->type2);
        return (x == a && y == b) || (x == b && y == a);
    });
}

vector<SearchResult> PCSystem::searchByMonoType(const string &type)
{
    const string t = toLower(type);
    if (t.empty())
        return {};

    return searchIf([t](const PokemonNode &p) {
        return p.species && toLower(p.species->type1) == t && p.species->type2.empty();
    });
}