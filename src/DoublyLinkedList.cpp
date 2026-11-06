#include "../include/DoublyLinkedList.h"
#include <iostream>

using namespace std;

DoublyLinkedList::DoublyLinkedList()
{
    head = nullptr;
    tail = nullptr;
    count = 0;
    nextID = 1;
}

DoublyLinkedList::~DoublyLinkedList()
{
    PokemonNode *current = head;
    while (current != nullptr)
    {
        PokemonNode *next = current->next;
        delete current;
        current = next;
    }
}

void DoublyLinkedList::append(const SpeciesData *species, string nick, string gndr,
                              int lvl, string nat, string abil,
                              bool shiny, string date)
{
    if (count >= 30)
    {
        cout << "[ERROR] Box is full! Cannot add more Pokémon." << endl;
        return;
    }

    PokemonNode *newNode = new PokemonNode(
        nextID++, species, nick, gndr, lvl,
        nat, abil, shiny, date);

    if (head == nullptr)
    {
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }

    count++;
}

void DoublyLinkedList::displayBox() const
{
    if (head == nullptr)
    {
        cout << "Box is empty." << endl;
        return;
    }

    PokemonNode *current = head;
    while (current != nullptr)
    {
        cout << current->pokemonID << ": "
             << current->nickname
             << " (" << current->species->name << ") - Lv. "
             << current->level << endl;

        current = current->next;
    }
}