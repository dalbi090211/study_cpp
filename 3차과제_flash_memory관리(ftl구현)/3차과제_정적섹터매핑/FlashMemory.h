#include<string>
#include<iostream>
#include <algorithm>	//remove�Լ� ���
#include<vector>		//erase���

using namespace std;

class flashMemory {

public : 
	~flashMemory();

private:
	FILE* writeBuffer;
	FILE* readBuffer;

public:
	void init(int blockMax);
	string read(int sectorNum);
	void write(int sectorNum,string inputData);
	void erase(int blcokNum);

};