#pragma once

struct mapNode {
	int logicalAddress;
	int realAddress;
	mapNode* link;
};

struct mapNode_h{
	mapNode* head;
};

struct deleteNode {
	int block;
	int count;
	deleteNode* link;
};

struct deleteNode_h {
	deleteNode* head;
};

struct spareNode {
	int realAddress;
	spareNode* link;
};

struct spareNode_h {
	spareNode* head;
};

void deleteTableAdd(deleteNode_h L, int sectorNum);
void spareTablePush(spareNode_h L, int sectorNum);