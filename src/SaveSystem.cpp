#include "../include/SaveSystem.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace
{
const int SAVE_VERSION = 1;

struct SavedPokemon
{
    int location;
    const SpeciesData *species;
    string nickname;
    string gender;
    int level;
    string ability;
    string nature;
    bool isShiny;
    string dateCaught;
};

string quote(const string &s)
{
    string out = "\"";
    for (char c : s)
    {
        if (c == '"')
            out += "\"\"";
        else if (c == '\n' || c == '\r')
            out += ' ';
        else
            out += c;
    }
    out += '"';
    return out;
}

bool toInt(const string &s, int &out)
{
    try
    {
        size_t pos = 0;
        out = stoi(s, &pos);
        return pos == s.size();
    }
    catch (const exception &)
    {
        return false;
    }
}

void writeList(ofstream &out, int location, const DoublyLinkedList &list)
{
    for (const PokemonNode *n = list.getHead(); n != nullptr; n = n->next)
    {
        if (n->species == nullptr)
            continue;

        out << "POKEMON," << location << "," << n->species->pokedexID << ","
            << quote(n->nickname) << "," << quote(n->gender) << "," << n->level << ","
            << quote(n->ability) << "," << quote(n->nature) << ","
            << (n->isShiny ? 1 : 0) << "," << quote(n->dateCaught) << "\n";
    }
}

void clearList(DoublyLinkedList &list)
{
    while (!list.isEmpty())
        list.releaseByPosition(1);
}
} // namespace

bool saveFileExists(const string &filename)
{
    ifstream f(filename);
    return f.good();
}

bool saveGame(const string &filename, PCSystem &pc, DoublyLinkedList &party)
{
    const string tmpName = filename + ".tmp";

    {
        ofstream out(tmpName);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not write " << tmpName << endl;
            return false;
        }

        out << "VERSION," << SAVE_VERSION << "\n";
        out << "CURRENT_BOX," << (pc.getCurrentBoxIndex() + 1) << "\n";
        for (int b = 1; b <= MAX_BOXES; ++b)
            out << "BOXNAME," << b << "," << quote(pc.getBoxName(b)) << "\n";

        writeList(out, 0, party);
        for (int b = 1; b <= MAX_BOXES; ++b)
            writeList(out, b, pc.getList(b));

        out.flush();
        if (!out)
        {
            cout << "[ERROR] Failed while writing the save file." << endl;
            return false;
        }
    } // file is closed here

    std::remove(filename.c_str()); // rename() won't overwrite an existing file on Windows
    if (std::rename(tmpName.c_str(), filename.c_str()) != 0)
    {
        cout << "[ERROR] Could not finalize the save file." << endl;
        return false;
    }
    return true;
}

bool loadGame(const string &filename, PCSystem &pc, DoublyLinkedList &party,
              const vector<SpeciesData *> &pokedex, int maxPartySize)
{
    ifstream in(filename);
    if (!in.is_open())
    {
        cout << "[ERROR] No save file found (" << filename << ")." << endl;
        return false;
    }

    int currentBox = 1;
    bool sawVersion = false;
    vector<string> boxNames(MAX_BOXES + 1);
    for (int b = 1; b <= MAX_BOXES; ++b)
        boxNames[b] = "Box " + to_string(b);
    vector<SavedPokemon> saved;
    vector<int> counts(MAX_BOXES + 1, 0); // index 0 = party

    string line;
    int lineNo = 0;
    auto fail = [&](const string &why) {
        cout << "[ERROR] Save file is invalid (line " << lineNo << ": " << why
             << "). Nothing was changed." << endl;
        return false;
    };

    while (getline(in, line))
    {
        ++lineNo;
        if (line.empty() || line == "\r")
            continue;

        vector<string> t = parseCSVLine(line);
        const string &tag = t[0];

        if (tag == "VERSION")
        {
            int v = 0;
            if (t.size() != 2 || !toInt(t[1], v) || v != SAVE_VERSION)
                return fail("unsupported version");
            sawVersion = true;
        }
        else if (!sawVersion)
        {
            return fail("missing VERSION header");
        }
        else if (tag == "CURRENT_BOX")
        {
            if (t.size() != 2 || !toInt(t[1], currentBox) ||
                currentBox < 1 || currentBox > MAX_BOXES)
                return fail("bad CURRENT_BOX");
        }
        else if (tag == "BOXNAME")
        {
            int b = 0;
            if (t.size() != 3 || !toInt(t[1], b) || b < 1 || b > MAX_BOXES || t[2].empty())
                return fail("bad BOXNAME");
            boxNames[b] = t[2];
        }
        else if (tag == "POKEMON")
        {
            int loc = 0, dex = 0, level = 0, shiny = 0;
            if (t.size() != 10 || !toInt(t[1], loc) || !toInt(t[2], dex) ||
                !toInt(t[5], level) || !toInt(t[8], shiny))
                return fail("bad POKEMON record");
            if (loc < 0 || loc > MAX_BOXES)
                return fail("bad location");
            if (shiny != 0 && shiny != 1)
                return fail("bad shiny flag");

            const SpeciesData *species = getSpeciesByPokedexNumber(pokedex, dex);
            if (species == nullptr)
                return fail("unknown Dex number " + to_string(dex));

            if (++counts[loc] > (loc == 0 ? maxPartySize : BOX_CAPACITY))
                return fail(loc == 0 ? "too many Pokémon in the party" : "box over capacity");

            saved.push_back({loc, species, t[3], t[4], level, t[6], t[7], shiny == 1, t[9]});
        }
        else
        {
            return fail("unknown record \"" + tag + "\"");
        }
    }

    if (!sawVersion)
        return fail("empty file");

    clearList(party);
    for (int b = 1; b <= MAX_BOXES; ++b)
    {
        clearList(pc.getList(b));
        pc.setBoxName(b, boxNames[b]);
    }

    for (const SavedPokemon &p : saved)
    {
        DoublyLinkedList &target = (p.location == 0) ? party : pc.getList(p.location);
        target.append(p.species, p.nickname, p.gender, p.level, p.ability, p.nature,
                      p.isShiny, p.dateCaught);
    }

    pc.setCurrentBox(currentBox);
    return true;
}