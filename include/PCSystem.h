#ifndef PC_SYSTEM_H
#define PC_SYSTEM_H

#include "DoublyLinkedList.h"
#include "Species.h"
#include <functional>
#include <string>
#include <vector>

const int MAX_BOXES = 30;
const int BOX_CAPACITY = 30;

struct Box
{
    std::string name;
    DoublyLinkedList list;

    Box(std::string boxName = "") : name(boxName) {}
};

struct SearchResult
{
    int boxNumber;
    std::string boxName;
    int slot;
    std::string nickname;
    std::string speciesName;
    std::string type1;
    std::string type2;
    int generation = 0;
    int dexNumber = 0;
};

class PCSystem
{
private:
    std::vector<Box> boxes;
    int currentBoxIndex;

public:
    PCSystem();

    DoublyLinkedList &getList(int boxNumber) { return boxes[boxNumber - 1].list; }
    const std::string &getBoxName(int boxNumber) const { return boxes[boxNumber - 1].name; }
    void setBoxName(int boxNumber, const std::string &name) { boxes[boxNumber - 1].name = name; }
    void setCurrentBox(int boxNumber) { currentBoxIndex = boxNumber - 1; }

    int getCurrentBoxIndex() const { return currentBoxIndex; }
    std::string getCurrentBoxName() const { return boxes[currentBoxIndex].name; }
    std::vector<SearchResult> search(const std::string &query);
    std::vector<SearchResult> searchIf(const std::function<bool(const PokemonNode &)> &pred);
    std::vector<SearchResult> searchByType(const std::string &type);
    std::vector<SearchResult> searchByTypePair(const std::string &type1, const std::string &type2);
    std::vector<SearchResult> searchByMonoType(const std::string &type);
    std::vector<SearchResult> searchByGeneration(int generation);
    std::vector<SearchResult> searchByDexNumber(int dexNumber);
    int getCurrentBoxCount() const { return boxes[currentBoxIndex].list.getCount(); }

    bool switchBox(int boxNumber);
    bool renameCurrentBox(const std::string &newName);
    void displayCurrentBox() const;
    void listAllBoxes() const;

    DoublyLinkedList &getCurrentList() { return boxes[currentBoxIndex].list; }

    bool moveBetweenBoxes(int sourceBoxIndex, int targetBoxIndex, int instanceID);
    bool moveBetweenBoxesByPosition(int srcBoxNum, int tgtBoxNum, int slotPosition);
    int storePokemonInAvailableBox(PokemonNode *pokemon);
};

#endif