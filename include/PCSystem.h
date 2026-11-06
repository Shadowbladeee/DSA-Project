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

class PCSystem
{
private:
    std::vector<Box> boxes;
    int currentBoxIndex;

public:
    PCSystem();

    int getCurrentBoxIndex() const { return currentBoxIndex; }
    std::string getCurrentBoxName() const { return boxes[currentBoxIndex].name; }
    int getCurrentBoxCount() const { return boxes[currentBoxIndex].list.getCount(); }

    bool switchBox(int boxNumber);
    bool renameCurrentBox(const std::string &newName);
    void displayCurrentBox() const;
    void listAllBoxes() const;

    DoublyLinkedList &getCurrentList() { return boxes[currentBoxIndex].list; }

    bool moveBetweenBoxes(int sourceBoxIndex, int targetBoxIndex, int instanceID);
};

#endif