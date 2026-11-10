#include "../include/CSVParser.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

using namespace std;

bool checkCSVFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open file: " << filename << endl;
        return false;
    }
    file.close();
    return true;
}

vector<string> parseCSVLine(const string &line)
{
    vector<string> result;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++)
    {
        char ch = line[i];

        if (ch == '"')
        {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                current += '"';
                i++;
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (ch == ',' && !inQuotes)
        {
            result.push_back(current);
            current.clear();
        }
        else
        {
            current += ch;
        }
    }

    result.push_back(current);

    if (!result.empty() && !result.back().empty() && result.back().back() == '\r')
    {
        result.back().pop_back();
    }

    return result;
}

int safeStoi(const string &str, int column, int defaultValue)
{
    if (str.empty())
        return defaultValue;

    try
    {
        size_t pos;
        int value = stoi(str, &pos);
        if (pos != str.size())
        {
            throw invalid_argument("Invalid characters");
        }
        return value;
    }
    catch (const exception &)
    {
        cerr << "[WARNING] Invalid value at column " << column << ": [" << str << "]. Using default: " << defaultValue << endl;
        return defaultValue;
    }
}

int parseGeneration(const string &str)
{
    static const char *GENS[] = {"gen-i", "gen-ii", "gen-iii", "gen-iv", "gen-v",
                                 "gen-vi", "gen-vii", "gen-viii", "gen-ix"};

    for (int i = 0; i < 9; i++)
    {
        if (str == GENS[i])
            return i + 1;
    }

    return safeStoi(str, 16, 0);
}

bool getRegionalFormKey(const string &name, string &key)
{
    static const string regionalMarkers[] = {"-Alola", "-Galar", "-Hisui", "-Paldea"};

    for (const string &marker : regionalMarkers)
    {
        const size_t markerPosition = name.find(marker);
        if (markerPosition == string::npos)
            continue;

        key = name.substr(0, markerPosition + marker.size());
        return true;
    }

    return false;
}

bool isExcludedSpecialForm(const string &name)
{
    return name.find("-Totem") != string::npos ||
           (name.rfind("Pikachu-", 0) == 0 && name.find("-Cap") != string::npos);
}

vector<SpeciesData *> loadSpeciesDatabase(const string &filename)
{
    vector<SpeciesData *> database;
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open " << filename << endl;
        return database;
    }

    string line;
    getline(file, line);
    unordered_set<string> seenRegionalForms;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> tokens = parseCSVLine(line);

        if (tokens.size() < 30)
            continue;

        if (isExcludedSpecialForm(tokens[1]))
            continue;

        int dexNum = safeStoi(tokens[0], 0);

        string regionalFormKey;
        const bool isRegional = getRegionalFormKey(tokens[1], regionalFormKey);
        if (dexNum > 1025 && isRegional &&
            !seenRegionalForms.insert(regionalFormKey).second)
        {
            continue;
        }

        if (dexNum > 1025 && !isRegional)
        {
            continue;
        }

        SpeciesData *species = new SpeciesData;
        species->internalID = safeStoi(tokens[0], 0);
        species->name = tokens[1];

        species->pokedexID = species->internalID;

        if (species->internalID > 1025)
        {
            const string name = species->name;
            size_t cut = name.rfind('-');
            bool found = false;

            while (cut != string::npos && !found)
            {
                const string prefix = name.substr(0, cut);

                for (const auto *base : database)
                {
                    if (base->internalID > 1025)
                        continue;
                    if (base->name == prefix || base->name.rfind(prefix + "-", 0) == 0)
                    {
                        species->pokedexID = base->internalID;
                        found = true;
                        break;
                    }
                }
                cut = prefix.rfind('-');
            }
        }
        
        species->type1 = tokens[2];
        species->type2 = tokens[3];
        species->hp = safeStoi(tokens[4], 4);
        species->attack = safeStoi(tokens[5], 5);
        species->defense = safeStoi(tokens[6], 6);
        species->spAttack = safeStoi(tokens[7], 7);
        species->spDefense = safeStoi(tokens[8], 8);
        species->speed = safeStoi(tokens[9], 9);
        species->baseStatTotal = safeStoi(tokens[10], 10);
        species->heightM = tokens[11].empty() ? 0.0f : stof(tokens[11]);
        species->weightKg = tokens[12].empty() ? 0.0f : stof(tokens[12]);
        species->baseExperience = safeStoi(tokens[13], 13);

        species->abilities = tokens[14];
        species->hiddenAbility = tokens[15];
        species->generation = parseGeneration(tokens[16]);
        species->isLegendary = (tokens[17] == "1" || tokens[17] == "True" || tokens[17] == "true");
        species->isMythical = (tokens[18] == "1" || tokens[18] == "True" || tokens[18] == "true");

        species->color = tokens[20];
        species->shape = tokens[21];
        species->eggGroups = tokens[22];
        species->habitat = tokens[23];

        species->captureRate = tokens[25].empty() ? 45 : safeStoi(tokens[25], 25, 45);
        species->genus = tokens[27];
        species->flavorText = tokens[29];

        species->spritePath = "assets/sprites/" + to_string(species->internalID) + ".png";

        database.push_back(species);
    }

    file.close();
    return database;
}

const SpeciesData *getSpeciesByPokedexNumber(const vector<SpeciesData *> &database, int dexNum)
{
    for (const auto *species : database)
    {
        if (species->internalID == dexNum || species->pokedexID == dexNum)
        {
            return species;
        }
    }
    return nullptr;
}