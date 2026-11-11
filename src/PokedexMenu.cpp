#include "../include/PokedexMenu.h"
#include "../include/ConsoleUtils.h"
#include "../include/GameSettings.h"
#include "../include/MenuUtils.h"
#include "../include/PokemonSummary.h"
#include "../include/TypeTags.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace
{
constexpr int MAX_POKEDEX_NUMBER = 1025;

int lastSelectedDexNumber()
{
    const int selected = getGameSettings().lastPokedexEntry;
    return selected >= 1 && selected <= MAX_POKEDEX_NUMBER ? selected : 1;
}

void rememberPokedexEntry(int dexNumber)
{
    if (dexNumber >= 1 && dexNumber <= MAX_POKEDEX_NUMBER)
        getGameSettings().lastPokedexEntry = dexNumber;
}

void displayPokedexMenu()
{
    clearScreen();
    printMenuHeader("POKÉDEX");
    cout << "1. Completion" << endl;
    cout << "2. Browse entries" << endl;
    cout << "3. Search entries" << endl;
    cout << "4. Go to Dex number" << endl;
    cout << "5. Filter by Caught / Seen / Unseen" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
}

int promptPokedexNumber()
{
    return readMenuChoice("\nEnter Dex number (1-1025, 0 to cancel): ",
                          MAX_POKEDEX_NUMBER, true, displayPokedexMenu);
}

} // namespace

vector<const SpeciesData *> getSortedPokedexEntries(
    const vector<SpeciesData *> &pokedex);

bool hasRegisteredForm(int dexNumber,
                       const vector<SpeciesData *> &pokedex,
                       const set<int> &registeredSpecies)
{
    for (const SpeciesData *species : pokedex)
    {
        if (species != nullptr && species->pokedexID == dexNumber &&
            registeredSpecies.count(species->internalID) != 0)
            return true;
    }
    return false;
}

bool hasEncounteredForm(int dexNumber,
                        const vector<SpeciesData *> &pokedex,
                        const set<int> &encounteredSpecies)
{
    for (const SpeciesData *species : pokedex)
    {
        if (species != nullptr && species->pokedexID == dexNumber &&
            encounteredSpecies.count(species->internalID) != 0)
            return true;
    }
    return false;
}

const SpeciesData *selectPokedexForm(
    int dexNumber, const vector<SpeciesData *> &pokedex,
    const set<int> &registeredSpecies, const set<int> &encounteredSpecies,
    const vector<const SpeciesData *> &matchingForms)
{
    const SpeciesData *baseForm = nullptr;
    const SpeciesData *caughtRegionalForm = nullptr;
    const SpeciesData *encounteredRegionalForm = nullptr;
    for (const SpeciesData *species : pokedex)
    {
        if (species == nullptr || species->pokedexID != dexNumber)
            continue;

        if (species->internalID == dexNumber)
        {
            baseForm = species;
            continue;
        }

        if (caughtRegionalForm == nullptr &&
            registeredSpecies.count(species->internalID) != 0)
            caughtRegionalForm = species;
        if (encounteredRegionalForm == nullptr &&
            encounteredSpecies.count(species->internalID) != 0)
            encounteredRegionalForm = species;
    }

    if (baseForm != nullptr &&
        (registeredSpecies.count(baseForm->internalID) != 0 ||
         encounteredSpecies.count(baseForm->internalID) != 0))
        return baseForm;

    if (caughtRegionalForm != nullptr)
        return caughtRegionalForm;
    if (encounteredRegionalForm != nullptr)
        return encounteredRegionalForm;

    for (const SpeciesData *species : matchingForms)
    {
        if (species != nullptr && species->pokedexID == dexNumber)
            return species;
    }

    return baseForm;
}

void showPokedexCompletion(const vector<SpeciesData *> &pokedex,
                           const set<int> &registeredSpecies)
{
    const vector<const SpeciesData *> entries = getSortedPokedexEntries(pokedex);
    int totalByGeneration[10] = {};
    int registeredByGeneration[10] = {};
    int totalRegistered = 0;

    for (const SpeciesData *species : entries)
    {
        ++totalByGeneration[species->generation >= 1 && species->generation <= 9
                                ? species->generation
                                : 0];
        if (hasRegisteredForm(species->pokedexID, pokedex, registeredSpecies))
        {
            ++totalRegistered;
            if (species->generation >= 1 && species->generation <= 9)
                ++registeredByGeneration[species->generation];
        }
    }

    const double overallPercent = entries.empty()
                                      ? 0.0
                                      : 100.0 * totalRegistered / entries.size();

    clearScreen();
    printMenuHeader("POKÉDEX COMPLETION");
    cout << "Overall: " << totalRegistered << "/" << entries.size()
         << " (" << fixed << setprecision(1) << overallPercent << "%)" << endl;
    cout << fillBar(totalRegistered, static_cast<int>(entries.size()), 30) << endl;

    for (int generation = 1; generation <= 9; ++generation)
    {
        if (totalByGeneration[generation] == 0)
            continue;

        const double percent = 100.0 * registeredByGeneration[generation] /
                               totalByGeneration[generation];
        cout << GENERATION_LABELS[generation - 1] << ": "
             << registeredByGeneration[generation] << "/" << totalByGeneration[generation]
             << " (" << fixed << setprecision(1) << percent << "%)" << endl;
    }
    waitForContinue();
}

vector<const SpeciesData *> getSortedPokedexEntries(const vector<SpeciesData *> &pokedex)
{
    vector<const SpeciesData *> entries;
    entries.reserve(pokedex.size());
    for (const SpeciesData *species : pokedex)
    {
        if (species != nullptr)
            entries.push_back(species);
    }

    sort(entries.begin(), entries.end(), [](const SpeciesData *a, const SpeciesData *b) {
        if (a->pokedexID != b->pokedexID)
            return a->pokedexID < b->pokedexID;
        const bool aIsBaseEntry = a->internalID == a->pokedexID;
        const bool bIsBaseEntry = b->internalID == b->pokedexID;
        if (aIsBaseEntry != bIsBaseEntry)
            return aIsBaseEntry;
        return a->internalID < b->internalID;
    });

    entries.erase(unique(entries.begin(), entries.end(),
                         [](const SpeciesData *a, const SpeciesData *b) {
                             return a->pokedexID == b->pokedexID;
                         }),
                  entries.end());
    return entries;
}

vector<const SpeciesData *> getSortedPokedexSpecies(const vector<SpeciesData *> &pokedex)
{
    vector<const SpeciesData *> entries;
    entries.reserve(pokedex.size());
    for (const SpeciesData *species : pokedex)
    {
        if (species != nullptr)
            entries.push_back(species);
    }

    sort(entries.begin(), entries.end(), [](const SpeciesData *a, const SpeciesData *b) {
        if (a->pokedexID != b->pokedexID)
            return a->pokedexID < b->pokedexID;
        const bool aIsBaseEntry = a->internalID == a->pokedexID;
        const bool bIsBaseEntry = b->internalID == b->pokedexID;
        if (aIsBaseEntry != bIsBaseEntry)
            return aIsBaseEntry;
        return a->internalID < b->internalID;
    });

    return entries;
}

vector<const SpeciesData *> getCanonicalEntriesForMatches(
    const vector<const SpeciesData *> &matches,
    const vector<SpeciesData *> &pokedex)
{
    set<int> matchingDexNumbers;
    for (const SpeciesData *species : matches)
        matchingDexNumbers.insert(species->pokedexID);

    vector<const SpeciesData *> entries;
    for (const SpeciesData *species : getSortedPokedexEntries(pokedex))
    {
        if (matchingDexNumbers.count(species->pokedexID) != 0)
            entries.push_back(species);
    }
    return entries;
}

void browsePokedexForms(const SpeciesData *selected,
                        const vector<SpeciesData *> &pokedex,
                        const set<int> &registeredSpecies,
                        const set<int> &encounteredSpecies)
{
    rememberPokedexEntry(selected->pokedexID);
    vector<const SpeciesData *> forms;
    for (const SpeciesData *species : pokedex)
    {
        if (species != nullptr && species->pokedexID == selected->pokedexID)
            forms.push_back(species);
    }
    sort(forms.begin(), forms.end(), [](const SpeciesData *a, const SpeciesData *b) {
        return a->internalID < b->internalID;
    });

    size_t formIndex = 0;
    for (size_t i = 0; i < forms.size(); ++i)
    {
        if (forms[i]->internalID == selected->internalID)
        {
            formIndex = i;
            break;
        }
    }

    while (true)
    {
        const SpeciesData *form = forms[formIndex];
        const bool encountered = encounteredSpecies.count(form->internalID) != 0;
        const bool caught = registeredSpecies.count(form->internalID) != 0;
        printSpeciesSummary(form, encountered, caught, true, false);

        if (forms.size() > 1)
            cout << "\nForm " << (formIndex + 1) << "/" << forms.size()
                 << " | N = Next form | P = Previous form" << endl;
        const int navigation = readSlotOrNav(
            forms.size() > 1 ? "N = Next form | P = Previous form | 0 = Back: "
                             : "0 = Back: ",
            0);
        if (navigation == NAV_INVALID)
            continue;
        if (navigation == 0)
            return;
        if (navigation == NAV_NEXT)
            formIndex = (formIndex + 1) % forms.size();
        else if (navigation == NAV_PREV)
            formIndex = (formIndex + forms.size() - 1) % forms.size();
        else
        {
            reportInvalidInput("Invalid choice.");
        }
    }
}

void showPokedexEntryByNumber(int dexNumber,
                              const vector<SpeciesData *> &pokedex,
                              const set<int> &registeredSpecies,
                              const set<int> &encounteredSpecies)
{
    const vector<const SpeciesData *> noMatchingForms;
    const SpeciesData *selected = selectPokedexForm(
        dexNumber, pokedex, registeredSpecies, encounteredSpecies, noMatchingForms);

    if (selected != nullptr)
    {
        rememberPokedexEntry(selected->pokedexID);
        browsePokedexForms(selected, pokedex, registeredSpecies, encounteredSpecies);
        return;
    }

    clearScreen();
    printMenuHeader("POKÉDEX ENTRY #" + to_string(dexNumber));
    cout << "[EMPTY ENTRY] No species data is available for this Dex number.\n";
    waitForContinue();
}

void browsePokedexEntries(vector<const SpeciesData *> entries,
                          const vector<SpeciesData *> &pokedex,
                          const set<int> &registeredSpecies,
                          const set<int> &encounteredSpecies, const string &title)
{
    const vector<const SpeciesData *> matchingForms = entries;
    entries = getCanonicalEntriesForMatches(entries, pokedex);
    const size_t pageSize = 20;
    size_t page = 0;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (entries[i]->pokedexID == lastSelectedDexNumber())
        {
            page = i / pageSize;
            break;
        }
    }

    while (true)
    {
        clearScreen();
        const size_t pageStart = page * pageSize;
        const size_t pageEnd = min(entries.size(), pageStart + pageSize);
        printMenuHeader(title + " (" + to_string(entries.size()) + " entries)");
        if (entries.empty())
        {
            cout << "(No matches)" << endl;
            waitForContinue();
            return;
        }
        cout << "Page " << (page + 1) << "/" << ((entries.size() + pageSize - 1) / pageSize)
             << " | C = Caught, S = Seen, - = Unseen" << endl;

        for (size_t i = pageStart; i < pageEnd; ++i)
        {
            const SpeciesData *entry = entries[i];
            const SpeciesData *species = selectPokedexForm(
                entry->pokedexID, pokedex, registeredSpecies, encounteredSpecies,
                matchingForms);
            const size_t slot = i - pageStart + 1;
            const bool isRegistered = hasRegisteredForm(
                entry->pokedexID, pokedex, registeredSpecies);
            const bool isEncountered = hasEncounteredForm(
                entry->pokedexID, pokedex, encounteredSpecies);
            cout << (entry->pokedexID == lastSelectedDexNumber() ? "> " : "  ")
                 << setw(2) << slot << ". #"
                 << right << setfill('0') << setw(3) << entry->pokedexID << setfill(' ')
                 << " " << (isEncountered ? species->name : "???");
            if (isRegistered)
                cout << " " << colorTypeTag(species->type1)
                     << (species->type2.empty() ? "" : " " + colorTypeTag(species->type2))
                     << " \033[92m[C]\033[0m";
            else if (isEncountered)
                cout << " \033[93m[S]\033[0m";
            else
                cout << " \033[90m[-]\033[0m";
            cout
                 << endl;
        }

        const int selection = readSlotOrNav(
            "Entry # to view | N = Next | P = Previous | 0 = Back: ",
            static_cast<int>(pageEnd - pageStart));
        if (selection == NAV_INVALID)
            continue;
        if (selection == 0)
            return;
        if (selection == NAV_NEXT)
        {
            if (pageEnd < entries.size())
                ++page;
            else
            {
                reportError("Already on the last page.");
            }
            continue;
        }
        if (selection == NAV_PREV)
        {
            if (page > 0)
                --page;
            else
            {
                reportError("Already on the first page.");
            }
            continue;
        }
        if (selection < 1 ||
            selection > static_cast<int>(pageEnd - pageStart))
        {
            reportInvalidInput("Invalid choice.");
            continue;
        }

        const SpeciesData *selected = entries[pageStart + static_cast<size_t>(selection - 1)];
        rememberPokedexEntry(selected->pokedexID);
        browsePokedexForms(selected, pokedex, registeredSpecies, encounteredSpecies);
    }
}

void filterPokedexByStatus(const vector<SpeciesData *> &pokedex,
                           const set<int> &registeredSpecies,
                           const set<int> &encounteredSpecies)
{
    clearScreen();
    printMenuHeader("FILTER POKÉDEX BY STATUS");
    cout << "1. Caught" << endl;
    cout << "2. Seen (not caught)" << endl;
    cout << "3. Unseen" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
    const int choice = readMenuChoice("\nChoice: ", 3, true, []() {
        clearScreen();
        printMenuHeader("FILTER POKÉDEX BY STATUS");
        cout << "1. Caught" << endl;
        cout << "2. Seen (not caught)" << endl;
        cout << "3. Unseen" << endl;
        printMenuSeparator();
        cout << "0. Back" << endl;
    });
    if (choice == 0)
        return;

    vector<const SpeciesData *> matches;
    for (const SpeciesData *species : getSortedPokedexEntries(pokedex))
    {
        const bool caught = hasRegisteredForm(
            species->pokedexID, pokedex, registeredSpecies);
        const bool seen = hasEncounteredForm(
            species->pokedexID, pokedex, encounteredSpecies);
        if ((choice == 1 && caught) ||
            (choice == 2 && seen && !caught) ||
            (choice == 3 && !seen))
            matches.push_back(species);
    }

    const string title = choice == 1 ? "POKÉDEX: CAUGHT"
                         : choice == 2 ? "POKÉDEX: SEEN"
                                       : "POKÉDEX: UNSEEN";
    browsePokedexEntries(std::move(matches), pokedex, registeredSpecies,
                         encounteredSpecies, title);
}

void searchPokedex(const vector<SpeciesData *> &pokedex,
                   const set<int> &registeredSpecies,
                   const set<int> &encounteredSpecies)
{
    clearScreen();
    printMenuHeader("SEARCH POKÉDEX");
    cout << "1. By nickname / species" << endl;
    cout << "2. By type" << endl;
    cout << "3. By generation" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
    const int choice = readMenuChoice("\nChoice: ", 3, true, []() {
        clearScreen();
        printMenuHeader("SEARCH POKÉDEX");
        cout << "1. By nickname / species" << endl;
        cout << "2. By type" << endl;
        cout << "3. By generation" << endl;
        printMenuSeparator();
        cout << "0. Back" << endl;
    });
    if (choice == 0)
        return;

    vector<const SpeciesData *> matches;
    string title;
    const vector<const SpeciesData *> entries = getSortedPokedexSpecies(pokedex);

    if (choice == 1)
    {
        string query;
        cout << "Enter nickname or species: ";
        if (!getline(cin, query))
            return;
        const string normalizedQuery = toLowerStr(query);
        title = "POKÉDEX SEARCH: \"" + query + "\"";
        if (!normalizedQuery.empty())
        {
            for (const SpeciesData *species : entries)
            {
                if (toLowerStr(species->name).find(normalizedQuery) != string::npos)
                    matches.push_back(species);
            }
        }
    }
    else if (choice == 2)
    {
        const string type1 = chooseType();
        if (type1.empty())
            return;
        const int second = chooseSecondType(type1);
        if (second == 0)
            return;

        title = "POKÉDEX TYPE: " + colorTypeTag(type1);
        if (second >= 1 && second <= static_cast<int>(POKEMON_TYPES.size()))
        {
            const string type2 = POKEMON_TYPES[second - 1];
            title += " / " + colorTypeTag(type2) + " (either order)";
            for (const SpeciesData *species : entries)
            {
                const bool pairMatches =
                    (toLowerStr(species->type1) == toLowerStr(type1) &&
                     toLowerStr(species->type2) == toLowerStr(type2)) ||
                    (toLowerStr(species->type1) == toLowerStr(type2) &&
                     toLowerStr(species->type2) == toLowerStr(type1));
                if (pairMatches)
                    matches.push_back(species);
            }
        }
        else if (second == static_cast<int>(POKEMON_TYPES.size()) + 1)
        {
            title += " (any second type)";
            for (const SpeciesData *species : entries)
            {
                if (toLowerStr(species->type1) == toLowerStr(type1) ||
                    toLowerStr(species->type2) == toLowerStr(type1))
                    matches.push_back(species);
            }
        }
        else
        {
            title += " (single-type only)";
            for (const SpeciesData *species : entries)
            {
                if (toLowerStr(species->type1) == toLowerStr(type1) &&
                    species->type2.empty())
                    matches.push_back(species);
            }
        }
    }
    else if (choice == 3)
    {
        const int generation = chooseGeneration();
        if (generation == 0)
            return;
        title = "POKÉDEX GENERATION: " + GENERATION_LABELS[generation - 1];
        for (const SpeciesData *species : entries)
        {
            if (species->generation == generation)
                matches.push_back(species);
        }
    }
    browsePokedexEntries(std::move(matches), pokedex, registeredSpecies,
                         encounteredSpecies, title);
}

void browsePokedex(const vector<SpeciesData *> &pokedex,
                   const set<int> &registeredSpecies,
                   const set<int> &encounteredSpecies)
{
    browsePokedexEntries(getSortedPokedexSpecies(pokedex), pokedex,
                         registeredSpecies, encounteredSpecies, "POKÉDEX");
}

void handlePokedexMenu(const vector<SpeciesData *> &pokedex,
                       const set<int> &registeredSpecies,
                       const set<int> &encounteredSpecies)
{
    while (true)
    {
        displayPokedexMenu();
        const int choice = readMenuChoice("\nChoice: ", 5, true,
                                          displayPokedexMenu);
        if (choice == 0)
            return;
        if (choice == 1)
            showPokedexCompletion(pokedex, registeredSpecies);
        else if (choice == 2)
            browsePokedex(pokedex, registeredSpecies, encounteredSpecies);
        else if (choice == 3)
            searchPokedex(pokedex, registeredSpecies, encounteredSpecies);
        else if (choice == 4)
        {
            const int dexNumber = promptPokedexNumber();
            if (dexNumber != 0)
                showPokedexEntryByNumber(dexNumber, pokedex, registeredSpecies,
                                         encounteredSpecies);
        }
        else
            filterPokedexByStatus(pokedex, registeredSpecies, encounteredSpecies);
    }
}
