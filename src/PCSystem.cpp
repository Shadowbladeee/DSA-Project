#include "../include/PCSystem.h"
#include <iostream>

using namespace std;

PCSystem::PCSystem()
{
    currentBoxIndex = 0;
    for (int i = 1; i <= MAX_BOXES; i++)
    {
        boxes.push_back(Box("Box " + to_string(i)));
    }
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