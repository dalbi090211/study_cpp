#define MEGABYTE 1048576
#define BLOCK_COUNT 32
#define SECTOR_BYTE 512
#define SPARE_LENGTH 8

#include<string>
#include<iostream>

#include"FTL.h"

using namespace std;

/*	����-���� ����
	1. 1��Ͽ��� 32���� �� 24���͸� ����ϰ� 8���͸� �������ͷ� ����
	2. ������ �Ͼ�� �� �����͸� �ű� ������ ���ʹ� �ش� ��ϳ��� �����س��⿡ dram���� ���м����� �ּҸ� ������� �ʿ䰡 ����
	3. �ٸ� �������Ͱ� �������� �������÷����� ������ �����͸� ��� dram�� �÷����ߵǱ⿡ 24���Ͱ� �ʿ���
	4. ����Ʈ����� �޸���, �׸��ǰ� ���� ���� ������Ʈ�� �̷������ �ʴ� ����Ʈ��� �ְ� ������Ʈ�� �Ǵ��� �Ϻ��� ���ϸ� ������ �̷������.
	5. ���� ���� �÷��� �޸𸮿����� �����Ͱ� ���� Ƚ��, ������ Ƚ��, ��ġ�� ��¥���� �̿��� ���������� �����͸� �����ϰ� ���м����� ������ ���������� ��������� ����������
	�� �ڵ忡���� �ܼ��� �������ο� ���� ���������� ���� �����ʹ� 32������ ��Ͽ� �����ϰ� ������ �����ʹ� 24���� + 8���м����� ��Ͽ� ����
*/

/* �޸� �Ҵ�
* 
*	1. �Է¹��� �ް�����Ʈ�� �´� ������ ��ϸ�ŭ ����
*	2. ������ ��ϸ�ŭ �������̺��� ���� �� �������̺��� �ʱ�ȭ(flashmemory�� �������̺�� ��ġ��Ŵ, �����̹Ƿ� ���� ������ ������ü�� �� �Ǿ��־����)
*	3. �������̺�� �������̺� ����(�������̺��� ������ ���� ��ǥ�� �˾Ƴ� ����� read�ۿ� ���⿡ �ʱ⿡�� spare_block�� ����־����)
*/
void FTL::init(int megabytes) {

	//megabytes/3*4��ŭ �÷����޸𸮸� ����
	if (blockMax == 0) {
		blockMax = (megabytes * MEGABYTE) / (SECTOR_BYTE * BLOCK_COUNT);
		spareMax = blockMax / 4;
		mappingTable = new int[blockMax * (BLOCK_COUNT - SPARE_LENGTH)];	//�������̺� ����(���� �߰��������� �Է¹��� ũ�⸸ŭ)(sector����)
		for (int i = 0; i < blockMax * (BLOCK_COUNT - SPARE_LENGTH); i++) {
			mappingTable[i] = i / (BLOCK_COUNT - SPARE_LENGTH) * BLOCK_COUNT + i % (BLOCK_COUNT - SPARE_LENGTH);
		}
		memory.init(blockMax + spareMax);
		spareTable = new int[spareMax];
		for (int i = 0; i < spareMax; i++) {
			spareTable[i] = blockMax + i;
		}
		cout << megabytes << " megabytes flash memory ;" << endl;


	}
	else {
		cout << "�̹� �Ҵ�� �޸𸮰� �ֽ��ϴ�." << endl;
	}
}

/*	�б�
* 
*	1. �Է¹��� sectorNum�� ������ �÷��� �޸𸮺��� ū �� Ȯ��(����ó��)
*	2. ���� �ּҴ� �����Ǿ� �����Ƿ� mappingTable�� ���� �ּҸ� �ε����� ������ ����. ��ȯ ���� flash.read()�� �־� �б�
*	3. �о�� ���� �����Ѵٸ� ����, ������ �����޼��� ���.
*
*/
void FTL::read(int sectorNum) {
	int read = 0;
	string readData = "";

	if (blockMax == 0) {
		cout << "�Ҵ�� �÷��� �޸𸮰� �����ϴ�." << endl;
	}
	else {
		if (sectorNum > blockMax*(BLOCK_COUNT - SPARE_LENGTH)) {
			cout << "�Ҵ�� �޸� �ּҺ��� Ů�ϴ�." << endl;
		}
		else {
			readData = memory.read(mappingTable[sectorNum]);
			read++;
			if (readData.compare("") == 0) {
				cout << "�ش� ���Ϳ� �����Ͱ� �����ϴ�." << endl;
			}
			else {
				cout << sectorNum/(BLOCK_COUNT - SPARE_LENGTH) << "-" << sectorNum % (BLOCK_COUNT - SPARE_LENGTH) << ", data : " << readData << endl;
			}
		}
	}
	cout << "read : " << read << endl;
}

/*	����
* 
*	1. �Ҵ�� �޸𸮰� �ִ���, �Է¹��� sectorNum�� flash memory�� ũ�⺸�� ū �� Ȯ��
*	2. �����Ͱ� �����ϴ��� �Է¹��� sectorNum�� �̿��� flash memory�� read
*   3-1. �����Ͱ� �������� �ʴ´ٸ� �Է¹��� data�� flash memory�� write
*	3-2. �����Ͱ� �����Ѵٸ� �ش� ��Ͽ� ���м��͸� ã�������� read
*	3-2-1. ���м��͸� ã�� ��� mappingtable�� �ּҸ� ���м��ͷ� �ٲ��ְ� write
*	3-2-2. ���м��͸� ã�� ���� ��� ��Ͽ��� �ش� ���͸� ������ ������ ��ü�� �о���̰� erase�� �� ��� ��ü�� write(���� dram�� �ּ� ũ��� ������ ����ִ� �Ű� ���� + 1���)
*	3-2-2. �̶� write�ϴ� ����� spareBlock[0], ������ ����� spareBlock[spareMax-1]�� �����Ͽ� �ϳ��� ����� �ʹ� ���� ������ �ʰ� ó��
*/
void FTL::write(int sectorNum, string data) {

	int temp;
	int read = 0;
	int write = 0;
	int erase = 0;
	string* readData = new string[BLOCK_COUNT - SPARE_LENGTH];

	if(blockMax == 0) {	//1
		cout << "�Ҵ�� �÷��� �޸𸮰� �����ϴ�." << endl;
	}
	else {
		if (sectorNum > blockMax * (BLOCK_COUNT - SPARE_LENGTH)) {
			cout << "�Ҵ�� �޸� �ּҺ��� Ů�ϴ�." << endl;
		}
		else {	//2
			temp = mappingTable[sectorNum];
			readData[0] = memory.read(temp);
			read++;
			if (readData[0].compare("") == 0) {	//3-1
				memory.write(temp, data);
				write++;
			}
			else {		//3-2
				temp = temp / BLOCK_COUNT * BLOCK_COUNT;
				for (int i = temp + (BLOCK_COUNT - SPARE_LENGTH); i < temp + BLOCK_COUNT; i++) {	//���м��� ã��, temp�� �����ּ� sectorNum�� ���ּҷ� ���
					readData[0] = memory.read(i);
					read++;
					if (readData[0].compare("") == 0) {	//3-2-1
						memory.write(i, data);
						write++;
						mappingTable[sectorNum] = i;
						temp = -1;
						i = i + 8;
					}
				}
				if (temp != -1) {	//3-2-2, ������ �÷���
					for (int i = temp; i < temp + BLOCK_COUNT - SPARE_LENGTH; i++) {
						readData[i - temp] = memory.read(i);
						read++;
					}
					readData[sectorNum % (BLOCK_COUNT - SPARE_LENGTH)] = data;
					memory.erase(temp / BLOCK_COUNT);
					erase++;

					for (int i = spareTable[0]; i < spareTable[0] + BLOCK_COUNT - SPARE_LENGTH; i++) {
						memory.write(i, readData[i - spareTable[0]]);
						write++;
					}
					
					for (int i = 0; i < spareMax -1; i++){
						spareTable[i] = spareTable[i + 1];
					}
					spareTable[spareMax - 1] = temp;

				}
			}

		}
	}
	cout << "read : " << read << endl;
	cout << "write : " << write << endl;
	cout << "erase : " << erase << endl;
}


//�������̺� ���
void FTL::Print_table() {
	cout << "mapping table : " << endl;
	for (int i = 0; i < blockMax * (BLOCK_COUNT - SPARE_LENGTH); i++) {	//ĭ ���Ⲩ�� ���߿� printf�Ἥ %4d �̷� ��� Ȱ���ϴ°� ������
		cout << i << " : " << mappingTable[i] << endl;
	}
}