#include "../include/DoublyLinkedList.h"
#include <iostream>
#include <iomanip>

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
        << " (#" << right << setfill('0') << setw(3) << current->species->pokedexID
        << setfill(' ') << " " << current->species->name
        << ") [" << current->species->type1
        << (current->species->type2.empty() ? "" : "/" + current->species->type2)
        << "] - Lv. " << current->level << endl;
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

    if (node->pokemonID == 0)
        node->pokemonID = globalNextID++;

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

PokemonNode *DoublyLinkedList::mergeLists(PokemonNode *a, PokemonNode *b, const NodeLess &less)
{
    PokemonNode dummy;
    dummy.next = nullptr;
    PokemonNode *last = &dummy;

    while (a != nullptr && b != nullptr)
    {
        if (less(*b, *a))
        {
            last->next = b;
            b = b->next;
        }
        else
        {
            last->next = a;
            a = a->next;
        }
        last = last->next;
    }
    last->next = (a != nullptr) ? a : b;
    return dummy.next;
}

PokemonNode *DoublyLinkedList::mergeSortRec(PokemonNode *h, const NodeLess &less)
{
    if (h == nullptr || h->next == nullptr)
        return h;

    PokemonNode *slow = h;
    PokemonNode *fast = h->next;
    while (fast != nullptr && fast->next != nullptr)
    {
        slow = slow->next;
        fast = fast->next->next;
    }
    PokemonNode *second = slow->next;
    slow->next = nullptr;

    return mergeLists(mergeSortRec(h, less), mergeSortRec(second, less), less);
}

void DoublyLinkedList::sort(const NodeLess &less)
{
    if (count < 2)
        return;

    head = mergeSortRec(head, less);

    PokemonNode *prevNode = nullptr;
    for (PokemonNode *cur = head; cur != nullptr; cur = cur->next)
    {
        cur->prev = prevNode;
        prevNode = cur;
    }
    tail = prevNode;
}