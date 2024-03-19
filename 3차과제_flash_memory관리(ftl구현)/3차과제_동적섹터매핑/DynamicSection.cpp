#define MEGABYTE 1048576
#define BLOCK_COUNT 32
#define SECTOR_BYTE 512
#define SPARE_COUNT 20

#include<string>
#include<iostream>

#include"FTL.h"
#include"LinkedList.h"

using namespace std;

/*	����-���� ����
*	1. ���Ḯ��Ʈ�� Ȱ���� mappingTable�� spareTable, deleteTable�� �����Ҵ�
*	2. height�� ���� ���� �Ҵ�� ���̸� ����
*  3. height�� ���� �ʴ� ������ spare������ ��밡��.
*	4. height�� ������ ������ �÷����� ������ ���� ������ ����.
*/

/* �޸� �Ҵ�
* 
*	1. �Է¹��� �ް�����Ʈ�� �´� ������ ��ϸ�ŭ ����
*	2. ������ ��ϸ�ŭ �������̺��� ���� �� �������̺��� �ʱ�ȭ
*/
void FTL::init(int megabytes) {

	//megabytes/3*4��ŭ �÷����޸𸮸� ����
	if (blockMax == 0) {
		blockMax = (megabytes * MEGABYTE) / (SECTOR_BYTE * BLOCK_COUNT);
		mappingTable.head = nullptr;
		spareTable.head = nullptr;
		deleteTable.head = nullptr;
		memory.init(blockMax+SPARE_COUNT);
		cout << megabytes << " megabytes flash memory ;" << endl;
	}
	else {
		cout << "�̹� �Ҵ�� �޸𸮰� �ֽ��ϴ�." << endl;
	}
}

/*	�б�
* 
*	1. �÷��� �޸𸮰� �Ҵ�Ǿ�����, �Է¹��� ���� �Ҵ�� �޸� �ּҺ��� ū���� ���� ����ó��
*	2. mappingTable ��ü ��ȸ�ؼ� ��ġ�ϴ� �� ã��
*
*/
void FTL::read(int sectorNum) {
	int read = 0;
	string readData = "";
	mapNode* map;

	if (blockMax == 0) {
		cout << "�Ҵ�� �÷��� �޸𸮰� �����ϴ�." << endl;
	}
	else {
		if (sectorNum > blockMax*BLOCK_COUNT) {
			cout << "�Ҵ�� �޸� �ּҺ��� Ů�ϴ�." << endl;
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
				cout << "�ش� ���Ϳ� �����Ͱ� �����ϴ�." << endl;
			}
		}
	}
	read++;
}

/*	����
* 
* a, Ȯ��
*	1. �Ҵ�� �޸𸮰� �ִ���, �Է¹��� sectorNum�� flash memory�� ũ�⺸�� ū �� Ȯ��
*	2. mappingTable�� ��ȸ�� �Ҵ�� �������� Ȯ��
*	3-1. �����Ҵ�������� ���Ͷ�� logicalAddress�� ���� ���·� mappingTable�� �߰�
*	3-2. �Ҵ�� ���Ͷ�� ������ realAddress�� deleteTable�� ����
*	4. spareTable�� �����ϸ� spareTable�� ������ �޸𸮰� �� �Ҵ�Ȱ� �ƴ��� Ȯ���� �� height�� �ּҸ� ���. map�� realAddress�� ����ϴ� �ּҷ� ������ �� write
*	5. ���� 4������ �޸𸮰� �� �Ҵ�Ǿ��ٸ� ������ �÷����� ������ ���� ������ ����� ����
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
		cout << "�Ҵ�� �÷��� �޸𸮰� �����ϴ�." << endl;
	}
	else {
		if (sectorNum > blockMax * BLOCK_COUNT) {
			cout << "�Ҵ�� �޸� �ּҺ��� Ů�ϴ�." << endl;
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
			if (lastNode == nullptr) {	//mappingTable�� ���� ���
				map = new mapNode;	
				map->logicalAddress = sectorNum;
				map->link = nullptr;
				mappingTable.head = map;
			}
			else if (map == nullptr) {	//3-1. �����Ҵ�������� ������ ���
				map = new mapNode;
				map->link = nullptr;
				map->logicalAddress = sectorNum;
				lastNode->link = map;
			}
			else {	//3-2. �̹� �Ҵ�� ������ ���
				deleteTableAdd(deleteTable, map->realAddress);
			}
			
			if (spareTable.head != nullptr) {	//4. spareTable�˻�
				map->realAddress = spareTable.head->realAddress;
				memory.write(spareTable.head->realAddress, data);
				write++;
				spareTable.head = spareTable.head->link;
			}
			else {	//4. height�˻� �� height�� ���� ���� �Ҵ�
				if (height == (blockMax + SPARE_COUNT)*BLOCK_COUNT) {		//5. �ִ� ũ�⿡ �����Ͽ� ������ �÷��� ����
					deleteNode = deleteTable.head;
					temp = 0;
					while (deleteNode != nullptr) {
						if (deleteNode->count > temp) {
							temp = deleteNode->count;
							height = deleteNode->block;	//height�� ��� block��ȣ�� �����ϴ� �뵵�� ���
						}
					}
					lastNode = mappingTable.head;

					//�������÷����� ���� deleteTable�� ��带 �����������

					while (lastNode != nullptr) {	//mappingTable�� �̿��� ��Ͽ� �����ϴ� �����͸� �о��
						if (lastNode->realAddress/BLOCK_COUNT == height) {
							readData[lastNode->realAddress % BLOCK_COUNT] = memory.read(lastNode->realAddress);
							read++;
						}
						lastNode = lastNode->link;
					}
					memory.erase(height);
					erase++;
					temp = 0;	//�� �������� ������ ����
					for (int i = 0; i < BLOCK_COUNT; i++) {	//�����Ͱ� ������ readData�� ����Ǿ� �ִ� ���Ͷ�� �Ѿ��, 
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
				else {	//height�� ���� ���� �Ҵ�
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


//�������̺� ���
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