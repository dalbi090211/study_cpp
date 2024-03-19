#define BLOCK_COUNT 32
#define SECTOR_BYTE 512
#define TEXT_FILE "test.txt"

#include <string>
#include <iostream>
#include <algorithm>	//remove�Լ� ���
#include <vector>		//erase���

#include"FlashMemory.h"

#pragma warning(disable:4996)

using namespace std;


//�Ҹ���
flashMemory::~flashMemory() {

}

void flashMemory::init(int blockMax) {	

	//���Ͻ�Ʈ���� ������������ ���� �κ�. ���Ŀ� ������ �����ϴ����� ���� ����ó���� �տ� �߰��� �����
	writeBuffer = fopen(TEXT_FILE, "w+");

	//��ϴ���
	for (int i = 0; i < blockMax; i++) {
		//�� ��Ͽ� ���ʹ� 32��
		for (int k = 0; k < BLOCK_COUNT; k++) {
			//���ʹ���
			for (int j = 0; j < SECTOR_BYTE - 1; j++) {
				fputs(" ", writeBuffer);
			}
			fputs("\n", writeBuffer);
		}
	}
	fclose(writeBuffer);
}

/*
	sector�� �о���� string���� ��ȯ
*/
string flashMemory::read(int sectorNum) {

	readBuffer = fopen(TEXT_FILE, "r+");
	char result[SECTOR_BYTE] = "";
	fseek(readBuffer, sectorNum * SECTOR_BYTE, SEEK_SET);		//������ ���� ó����ġ���� sectorNum * ������ ����Ʈ��ŭ ���� �������� ��ġ�� �̵�
	fgets(result, SECTOR_BYTE, readBuffer);
	fclose(readBuffer);
	string strResult(result);	//fgets�� ���� char�� �迭 string���� ��ȯ(string���̺귯�� �Լ� ����ϱ� ���ؼ�)
	

	strResult.erase(remove(strResult.begin(), strResult.end(), string::npos), strResult.end());	//remove�� ���� ������, erase�� ���� ����ִ� ���ڿ��� ����
	if (strResult.compare("") == 0 || strResult.compare("\n") == 0) {
		cout << "�޸𸮰� �����ϴ�" << endl;
		return "";
	}
	else {
		return strResult;
	}
}

/*	
	1. �Է¹��� ���Ϳ� �����Ͱ� �����ϴ��� Ȯ��(read����) 
	2. �����Ͱ� ���������ʴ� ��� �����͸� �Է�(write����)	
*/	
void flashMemory::write(int sectorNum, string inputData) {

	const char* temp = inputData.c_str();	//string -> char* ����ȯ(fputs����ϱ����ؼ� ��ȯ)
	string str;
	char result[SECTOR_BYTE] = "";

	/*	�Է¹��� ���� ��ȣ�� �´� �����͸� �о���� �κ�	*/
	readBuffer = fopen(TEXT_FILE, "r+");
	fseek(readBuffer, sectorNum * SECTOR_BYTE, SEEK_SET);		
	fgets(result, SECTOR_BYTE, readBuffer);
	fclose(readBuffer);
	string strResult(result);	//fgets�� ���� char�� �迭 string���� ��ȯ(string���̺귯�� �Լ� ����ϱ� ���ؼ�)

	/*	�о�� �����Ͱ� �ִٸ� ����, ���ٸ� �Է¹��� �����͸� ����	*/	
	
	strResult.erase(remove(strResult.begin(), strResult.end(), string::npos), strResult.end());	//remove�� ���� ������, erase�� ���� ����ִ� ���ڿ��� ����
	if(strResult.compare("") == 0 || strResult.compare("\n")==0) {
		writeBuffer = fopen(TEXT_FILE, "w+");
		cout << "PSN : " << sectorNum << ", data : " << inputData << endl;
		fseek(writeBuffer, sectorNum * SECTOR_BYTE, SEEK_SET);
		fputs(temp, writeBuffer);
		fclose(writeBuffer);
	}
	else {
		cout << "error : �̹� �޸𸮰� �����մϴ�." << endl;
	}

}

void flashMemory::erase(int blockNum) {

	writeBuffer = fopen(TEXT_FILE, "w+");

	fseek(writeBuffer, blockNum * SECTOR_BYTE * BLOCK_COUNT, SEEK_SET);
	fwrite(" ", SECTOR_BYTE * BLOCK_COUNT, 0, writeBuffer);
	//�� ��Ͽ� ���ʹ� 32��
	for (int k = 0; k < BLOCK_COUNT; k++) {
		//���ʹ���
		for (int j = 0; j < SECTOR_BYTE; j++) {
			fputs(" ", writeBuffer);
		}
	}
	fclose(writeBuffer);
}