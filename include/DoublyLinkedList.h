#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include "PokemonNode.h"
#include "Species.h"
#include <string>

class DoublyLinkedList
{
private:
    PokemonNode *head;
    PokemonNode *tail;
    int count;
    int nextID;

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    void append(const SpeciesData *species, std::string nick, std::string gndr,
                int lvl, std::string nat, std::string abil,
                bool shiny, std::string date);
    void displayBox() const;

    int getCount() const { return count; }
    bool isEmpty() const { return head == nullptr; }
    PokemonNode *getHead() const { return head; }

    PokemonNode *removeByID(int instanceID);
    void insertNode(PokemonNode *node);

    bool swapNodes(int index1, int index2);
};

#endif