#include "LinkedList.h"

void deleteTableAdd(deleteNode_h L, int sectorNum) {
	deleteNode* addNode = new deleteNode;
	deleteNode* temp = nullptr;

	temp = L.head;
	while (temp != nullptr) {	//head�� null�� ��춧���� �ʿ�
		if (temp->block == sectorNum / 32) {
			temp->block = temp->block++;
			addNode = nullptr;
			break;
		}
		else {
			if (temp->link == nullptr) {		//null�̸� addNode�� link
				break;
			}
			else {
				temp = temp->link;
			}
		}
	}
	if (temp == nullptr) {	//head�� null�� ���
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