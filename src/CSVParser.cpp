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
    catch (const exception &e)
    {
        cerr << "[WARNING] Invalid value at column " << column << ": [" << str << "]. Using default: " << defaultValue << endl;
        return defaultValue;
    }
}

int parseGeneration(const string &str)
{
    if (str.find("gen-i") != string::npos)
        return 1;
    if (str.find("gen-ii") != string::npos)
        return 2;
    if (str.find("gen-iii") != string::npos)
        return 3;
    if (str.find("gen-iv") != string::npos)
        return 4;
    if (str.find("gen-v") != string::npos)
        return 5;
    if (str.find("gen-vi") != string::npos)
        return 6;
    if (str.find("gen-vii") != string::npos)
        return 7;
    if (str.find("gen-viii") != string::npos)
        return 8;
    if (str.find("gen-ix") != string::npos)
        return 9;

    return safeStoi(str, 16, 1);
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

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> tokens = parseCSVLine(line);

        if (tokens.size() < 30)
            continue;

        int dexNum = safeStoi(tokens[0], 0);

        if (dexNum > 1025 && ALLOWED_REGIONAL_IDS.find(dexNum) == ALLOWED_REGIONAL_IDS.end())
        {
            continue;
        }

        SpeciesData *species = new SpeciesData;
        species->pokedexNumber = safeStoi(tokens[0], 0);
        species->name = tokens[1];
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

        species->spritePath = "sprites/" + to_string(species->pokedexNumber) + ".png";

        database.push_back(species);
    }

    file.close();
    return database;
}

const SpeciesData *getSpeciesByPokedexNumber(const vector<SpeciesData *> &database, int dexNum)
{
    for (const auto *species : database)
    {
        if (species->pokedexNumber == dexNum)
        {
            return species;
        }
    }
    return nullptr;
}

// Whitelisted regional variant forms
const std::unordered_set<int> ALLOWED_REGIONAL_IDS = {
    // Alolan
    10091, 10092, 10100, 10101, 10102, 10103, 10104, 10105, 10106,
    10107, 10108, 10109, 10110, 10111, 10112, 10113, 10114, 10115,

    // Galarian
    10161, 10162, 10163, 10164, 10165, 10166, 10167, 10168, 10169,
    10170, 10171, 10172, 10173, 10174, 10175, 10176, 10177, 10179, 10180,

    // Hisuian
    10229, 10230, 10231, 10232, 10233, 10234, 10235, 10236, 10237,
    10238, 10239, 10240, 10241, 10242, 10243, 10244,

    // Paldean
    10250, 10251, 10252, 10253};