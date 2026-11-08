#ifndef PC_SYSTEM_H
#define PC_SYSTEM_H

#include "DoublyLinkedList.h"
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
    int boxNumber; // 1-based
    std::string boxName;
    int slot; // 1-based
    std::string nickname;
    std::string speciesName;
};

class PCSystem
{
private:
    std::vector<Box> boxes;
    int currentBoxIndex;

public:
    PCSystem();

    int getCurrentBoxIndex() const { return currentBoxIndex; }
    std::string getCurrentBoxName() const { return boxes[currentBoxIndex].name; }
    std::vector<SearchResult> search(const std::string &query);
    int getCurrentBoxCount() const { return boxes[currentBoxIndex].list.getCount(); }

    bool switchBox(int boxNumber);
    bool renameCurrentBox(const std::string &newName);
    void displayCurrentBox() const;
    void listAllBoxes() const;

    DoublyLinkedList &getCurrentList() { return boxes[currentBoxIndex].list; }

    bool moveBetweenBoxes(int sourceBoxIndex, int targetBoxIndex, int instanceID);
    bool moveBetweenBoxesByPosition(int srcBoxNum, int tgtBoxNum, int slotPosition);
};

#endif