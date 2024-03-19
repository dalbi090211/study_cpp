#include "LinkedList.h"

void deleteTableAdd(deleteNode_h L, int sectorNum) {
	deleteNode* addNode = new deleteNode;
	deleteNode* temp = nullptr;

	temp = L.head;
	while (temp != nullptr) {	//head가 null인 경우때문에 필요
		if (temp->block == sectorNum / 32) {
			temp->block = temp->block++;
			addNode = nullptr;
			break;
		}
		else {
			if (temp->link == nullptr) {		//null이면 addNode를 link
				break;
			}
			else {
				temp = temp->link;
			}
		}
	}
	if (temp == nullptr) {	//head가 null인 경우
		addNode->block = sectorNum / 32;
		addNode->count = 1;
		addNode->link = nullptr;
		L.head = addNode;
	}
	else if (addNode == nullptr) {
	
	}
	else if (temp->link == nullptr) {
		addNode->block = sectorNum / 32;
		addNode->count = 1;
		temp->link = addNode;
	}
}

void spareTablePush(spareNode_h L, int sectorNum) {
	spareNode* addNode = new spareNode;
	spareNode* temp = nullptr;
	
	addNode->link = nullptr;
	addNode->realAddress = sectorNum;

	temp = L.head;
	while (temp->link != nullptr) {
		temp = temp->link;
	}
	if (temp == nullptr) {
		L.head = addNode;
	}
	else {
		temp->link = addNode;
	}
}