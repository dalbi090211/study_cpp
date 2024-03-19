#define MEGABYTE 1048576
#define BLOCK_COUNT 32
#define SECTOR_BYTE 512
#define SPARE_COUNT 20

#include<string>
#include<iostream>

#include"FTL.h"
#include"LinkedList.h"

using namespace std;

/*	동적-섹터 매핑
*	1. 연결리스트를 활용해 mappingTable과 spareTable, deleteTable을 동적할당
*	2. height를 통해 현재 할당된 높이를 저장
*  3. height를 넘지 않는 선에서 spare영역을 사용가능.
*	4. height를 넘으면 가비지 컬렉션을 실행해 남는 영역을 만듬.
*/

/* 메모리 할당
* 
*	1. 입력받은 메가바이트에 맞는 개수의 블록만큼 생성
*	2. 생성한 블록만큼 매핑테이블을 생성 및 매핑테이블을 초기화
*/
void FTL::init(int megabytes) {

	//megabytes/3*4만큼 플래쉬메모리를 생성
	if (blockMax == 0) {
		blockMax = (megabytes * MEGABYTE) / (SECTOR_BYTE * BLOCK_COUNT);
		mappingTable.head = nullptr;
		spareTable.head = nullptr;
		deleteTable.head = nullptr;
		memory.init(blockMax+SPARE_COUNT);
		cout << megabytes << " megabytes flash memory ;" << endl;
	}
	else {
		cout << "이미 할당된 메모리가 있습니다." << endl;
	}
}

/*	읽기
* 
*	1. 플래시 메모리가 할당되었는지, 입력받은 값이 할당된 메모리 주소보다 큰지에 대한 예외처리
*	2. mappingTable 전체 순회해서 일치하는 값 찾기
*
*/
void FTL::read(int sectorNum) {
	int read = 0;
	string readData = "";
	mapNode* map;

	if (blockMax == 0) {
		cout << "할당된 플래시 메모리가 없습니다." << endl;
	}
	else {
		if (sectorNum > blockMax*BLOCK_COUNT) {
			cout << "할당된 메모리 주소보다 큽니다." << endl;
		}
		else {
			map = mappingTable.head;
			while (map != nullptr) {
				if (map->logicalAddress == sectorNum) {
					readData = memory.read(map->realAddress);
					read++;
					cout << sectorNum / BLOCK_COUNT << "-" << sectorNum % BLOCK_COUNT << ", data : " << readData << endl;
					break;
				}
				else {
					map = map->link;
				}
			}
			if (map == nullptr) {
				cout << "해당 섹터에 데이터가 없습니다." << endl;
			}
		}
	}
	read++;
}

/*	쓰기
* 
* a, 확인
*	1. 할당된 메모리가 있는지, 입력받은 sectorNum이 flash memory의 크기보다 큰 지 확인
*	2. mappingTable을 순회해 할당된 섹터인지 확인
*	3-1. 동적할당되지않은 섹터라면 logicalAddress만 넣은 상태로 mappingTable에 추가
*	3-2. 할당된 섹터라면 기존의 realAddress를 deleteTable로 전달
*	4. spareTable이 존재하면 spareTable을 없으면 메모리가 다 할당된건 아닌지 확인한 후 height의 주소를 사용. map의 realAddress를 사용하는 주소로 변경한 후 write
*	5. 만약 4번에서 메모리가 다 할당되었다면 가비지 컬렉션을 돌려서 여분 영역을 만들고 실행
* 
*/
void FTL::write(int sectorNum, string data) {

	int temp;
	int write = 0;
	int read = 0;
	int erase = 0;
	string* readData = new string[BLOCK_COUNT]{""};
	deleteNode* deleteNode;
	mapNode* map;
	mapNode* lastNode = nullptr;

	if(blockMax == 0) {	//1
		cout << "할당된 플래시 메모리가 없습니다." << endl;
	}
	else {
		if (sectorNum > blockMax * BLOCK_COUNT) {
			cout << "할당된 메모리 주소보다 큽니다." << endl;
		}
		else {	//2
			map = mappingTable.head;
			while (map != nullptr) {
				if (map->logicalAddress == sectorNum) {
					lastNode = map;
					break;
				}
				else {
					lastNode = map;
					map = map->link;
				}
			}
			if (lastNode == nullptr) {	//mappingTable이 없는 경우
				map = new mapNode;	
				map->logicalAddress = sectorNum;
				map->link = nullptr;
				mappingTable.head = map;
			}
			else if (map == nullptr) {	//3-1. 동적할당되지않은 섹터인 경우
				map = new mapNode;
				map->link = nullptr;
				map->logicalAddress = sectorNum;
				lastNode->link = map;
			}
			else {	//3-2. 이미 할당된 섹터인 경우
				deleteTableAdd(deleteTable, map->realAddress);
			}
			
			if (spareTable.head != nullptr) {	//4. spareTable검사
				map->realAddress = spareTable.head->realAddress;
				memory.write(spareTable.head->realAddress, data);
				write++;
				spareTable.head = spareTable.head->link;
			}
			else {	//4. height검사 후 height에 따른 섹터 할당
				if (height == (blockMax + SPARE_COUNT)*BLOCK_COUNT) {		//5. 최대 크기에 도달하여 가비지 컬렉션 실행
					deleteNode = deleteTable.head;
					temp = 0;
					while (deleteNode != nullptr) {
						if (deleteNode->count > temp) {
							temp = deleteNode->count;
							height = deleteNode->block;	//height를 잠시 block번호를 저장하는 용도로 사용
						}
					}
					lastNode = mappingTable.head;

					//가비지컬렉션을 돌린 deleteTable의 노드를 해제해줘야함

					while (lastNode != nullptr) {	//mappingTable을 이용해 블록에 존재하는 데이터를 읽어옴
						if (lastNode->realAddress/BLOCK_COUNT == height) {
							readData[lastNode->realAddress % BLOCK_COUNT] = memory.read(lastNode->realAddress);
							read++;
						}
						lastNode = lastNode->link;
					}
					memory.erase(height);
					erase++;
					temp = 0;	//빈 데이터의 개수를 저장
					for (int i = 0; i < BLOCK_COUNT; i++) {	//데이터가 존재해 readData에 저장되어 있는 섹터라면 넘어가고, 
						if (readData[i].compare("") != 0) {
							memory.write(height * BLOCK_COUNT + i, readData[i]);
							write++;
						}
						else if (temp == 0) {
							map->realAddress = height * BLOCK_COUNT + i;
							memory.write(map->realAddress, data);
							write++;
							temp++;
						}
						else {
							spareTablePush(spareTable, height*BLOCK_COUNT + i);
							temp++;
						}
					}
					height = blockMax * BLOCK_COUNT - temp + 1;
				}
				else {	//height에 따른 섹터 할당
					map->realAddress = height;
					memory.write(height, data);
					write++;
					height++;
				}
			}
		}
	}

	cout << "write : " << write << endl;
	cout << "erase : " << erase << endl;
}


//매핑테이블 출력
void FTL::Print_table() {
	mapNode* mapNode;
	spareNode* spareNode;
	deleteNode* deleteNode;

	//mappingTable
	mapNode = mappingTable.head;
	cout << "mapping table : " << endl;
	while (mapNode != nullptr) {
		cout << mapNode->logicalAddress << " : " << mapNode->realAddress << endl;
		mapNode = mapNode->link;
	}

	//spareTable
	spareNode = spareTable.head;
	cout << "spare table : " << endl;
	while (spareNode != nullptr) {
		cout << "realAddress : " << spareNode->realAddress << endl;
		spareNode = spareNode->link;
	}

	//deleteTable
	deleteNode =	deleteTable.head;
	cout << "delete table : " << endl;
	while (deleteNode != nullptr) {
		cout << "block : " << deleteNode->block << ", count : " << deleteNode->count << endl;
		deleteNode = deleteNode->link;
	}
}