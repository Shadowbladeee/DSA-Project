#ifndef PC_MENU_H
#define PC_MENU_H

#include "PCSystem.h"

void displayParty(const DoublyLinkedList &party);
void managePartyOutsideBox(DoublyLinkedList &party, PCSystem &pc);
void viewPCMenu(PCSystem &pc, DoublyLinkedList &party);

#endif
