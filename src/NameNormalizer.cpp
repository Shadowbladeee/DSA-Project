#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "../include/NameNormalizer.h"

using namespace std;

namespace {

string trim(const string &value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos)
        return "";

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

string toLower(string value)
{
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                   { return static_cast<char>(tolower(ch)); });
    return value;
}

bool isDefaultFormSuffix(const string &suffix)
{
    static const unordered_set<string> strippedSuffixes = {
        "normal", "plant", "altered", "land", "red-striped", "standard",
        "male", "female", "incarnate", "ordinary", "aria", "average",
        "baile", "midday", "solo", "disguised", "amped", "ice",
        "full-belly", "single-strike", "zero", "curly", "two-segment",
        "green-plumage", "family-of-four", "shield", "blue-striped",
        "red-meteor", "50"
    };

    return strippedSuffixes.count(toLower(suffix)) > 0;
}

bool isPreservedHyphenatedName(const string &name)
{
    static const unordered_set<string> preserved = {
        "nidoran-f", "nidoran-m", "ho-oh", "porygon-z", "type-null",
        "jangmo-o", "hakamo-o", "kommo-o", "tapu-koko", "tapu-lele",
        "tapu-bulu", "tapu-fini", "wo-chien", "chien-pao", "ting-lu",
        "chi-yu"
    };

    return preserved.count(toLower(trim(name))) > 0;
}

string normalizeSpecialDisplayName(const string &name)
{
    const string lowered = toLower(trim(name));
    if (lowered == "nidoran-f")
        return "Nidoran♀";
    if (lowered == "nidoran-m")
        return "Nidoran♂";
    if (lowered == "type-null")
        return "Type: Null";
    if (lowered == "jangmo-o")
        return "Jangmo-o";
    if (lowered == "hakamo-o")
        return "Hakamo-o";
    if (lowered == "kommo-o")
        return "Kommo-o";
    return "";
}

string humanizeSpaces(string name)
{
    replace(name.begin(), name.end(), '-', ' ');

    const string lowered = toLower(name);
    if (lowered == "mr mime" || lowered == "mr. mime")
        return "Mr. Mime";
    if (lowered == "mr rime" || lowered == "mr. rime")
        return "Mr. Rime";
    if (lowered == "mime jr" || lowered == "mime jr.")
        return "Mime Jr.";

    return name;
}

} // namespace

string normalizePokemonName(const string &input)
{
    string name = trim(input);
    if (name.empty())
        return "";

    const string specialDisplay = normalizeSpecialDisplayName(name);
    if (!specialDisplay.empty())
        return specialDisplay;

    if (isPreservedHyphenatedName(name))
        return name;

    if (name.find('-') == string::npos)
        return name;

    vector<string> segments;
    stringstream stream(name);
    string segment;
    while (getline(stream, segment, '-'))
    {
        if (!segment.empty())
            segments.push_back(segment);
    }

    if (segments.size() > 1)
    {
        const string first = trim(segments.front());

        for (size_t i = 1; i < segments.size(); ++i)
        {
            string candidate;
            for (size_t j = i; j < segments.size(); ++j)
            {
                if (!candidate.empty())
                    candidate += '-';
                candidate += trim(segments[j]);

                if (isDefaultFormSuffix(candidate))
                    return first;
            }
        }
    }

    return humanizeSpaces(name);
}
