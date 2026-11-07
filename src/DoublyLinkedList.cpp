#include "../include/DoublyLinkedList.h"
#include <iostream>

using namespace std;

static int globalNextID = 1;

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

void DoublyLinkedList::append(const SpeciesData *species, string nickname, string gender,
                              int level, string ability, string nature, bool isShiny, string dateCaught)
{
    PokemonNode *newNode = new PokemonNode;
    newNode->pokemonID = globalNextID++;
    newNode->species = species;
    newNode->nickname = nickname;
    newNode->gender = gender;
    newNode->level = level;
    newNode->ability = ability;
    newNode->nature = nature;
    newNode->isShiny = isShiny;
    newNode->dateCaught = dateCaught;
    newNode->next = nullptr;
    newNode->prev = nullptr;

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
    int slot = 1;
    while (current != nullptr)
    {
        cout << slot << ": " << current->nickname
             << " (" << current->species->name
             << ") - Lv. " << current->level << endl;
        current = current->next;
        slot++;
    }
}

PokemonNode *DoublyLinkedList::removeByID(int instanceID)
{
    PokemonNode *current = head;
    while (current != nullptr)
    {
        if (current->pokemonID == instanceID)
        {
            if (current->prev != nullptr)
                current->prev->next = current->next;
            else
                head = current->next;

            if (current->next != nullptr)
                current->next->prev = current->prev;
            else
                tail = current->prev;

            current->prev = nullptr;
            current->next = nullptr;
            count--;
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

PokemonNode *DoublyLinkedList::removeByPosition(int position)
{
    if (position < 1 || position > count || head == nullptr)
        return nullptr;

    PokemonNode *current = head;
    for (int i = 1; i < position; i++)
    {
        current = current->next;
    }

    if (current->prev != nullptr)
        current->prev->next = current->next;
    else
        head = current->next;

    if (current->next != nullptr)
        current->next->prev = current->prev;
    else
        tail = current->prev;

    current->prev = nullptr;
    current->next = nullptr;
    count--;

    return current;
}

void DoublyLinkedList::insertNode(PokemonNode *node)
{
    if (node == nullptr || count >= 30)
        return;

    node->prev = nullptr;
    node->next = nullptr;

    if (head == nullptr)
    {
        head = node;
        tail = node;
    }
    else
    {
        tail->next = node;
        node->prev = tail;
        tail = node;
    }
    count++;
}

bool DoublyLinkedList::swapNodes(int index1, int index2)
{
    if (index1 == index2 || index1 < 1 || index2 < 1 || index1 > count || index2 > count)
        return false;

    PokemonNode *node1 = head;
    for (int i = 1; i < index1; i++)
        node1 = node1->next;

    PokemonNode *node2 = head;
    for (int i = 1; i < index2; i++)
        node2 = node2->next;

    // Swap data pointers/values between nodes
    std::swap(node1->species, node2->species);
    std::swap(node1->nickname, node2->nickname);
    std::swap(node1->gender, node2->gender);
    std::swap(node1->level, node2->level);
    std::swap(node1->nature, node2->nature);
    std::swap(node1->ability, node2->ability);
    std::swap(node1->isShiny, node2->isShiny);
    std::swap(node1->dateCaught, node2->dateCaught);

    return true;
}

bool DoublyLinkedList::releaseByPosition(int position)
{
    PokemonNode *target = removeByPosition(position);
    if (target == nullptr)
        return false;

    delete target; // Free dynamically allocated memory
    return true;
}