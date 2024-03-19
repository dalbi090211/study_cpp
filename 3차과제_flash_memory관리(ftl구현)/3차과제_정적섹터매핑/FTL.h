#include<string>
#include<iostream>

#include"FlashMemory.h"

using namespace std;



class FTL {
private:
	flashMemory memory;
	int* mappingTable;
	int blockMax = 0;
	int* spareTable;
	int spareMax;

public:
	void init(int megabytes);
	void read(int sectorNum);
	void write(int sectorNum, string data);
	void Print_table();
};
