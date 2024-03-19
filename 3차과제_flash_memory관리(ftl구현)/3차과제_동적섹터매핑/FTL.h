#include<string>
#include<iostream>

#include"LinkedList.h"
#include"FlashMemory.h"

using namespace std;


class FTL {
private:
	flashMemory memory;
	mapNode_h mappingTable;
	spareNode_h spareTable;
	deleteNode_h deleteTable;
	int blockMax = 0;
	int height = 0;

public:
	void init(int megabytes);
	void read(int sectorNum);
	void write(int sectorNum, string data);
	void Print_table();
};
