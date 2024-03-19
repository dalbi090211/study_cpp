#define BLOCK_COUNT 32
#define SECTOR_BYTE 512
#define TEXT_FILE "test.txt"

#include <string>
#include <iostream>
#include <algorithm>	//remove함수 사용
#include <vector>		//erase사용

#include"FlashMemory.h"

#pragma warning(disable:4996)

using namespace std;


//소멸자
flashMemory::~flashMemory() {

}

void flashMemory::init(int blockMax) {	

	//파일스트림을 쓰기형식으로 여는 부분. 추후에 파일이 존재하는지에 대한 예외처리를 앞에 추가해 줘야함
	writeBuffer = fopen(TEXT_FILE, "w+");

	//블록단위
	for (int i = 0; i < blockMax; i++) {
		//한 블록에 섹터는 32개
		for (int k = 0; k < BLOCK_COUNT; k++) {
			//섹터단위
			for (int j = 0; j < SECTOR_BYTE - 1; j++) {
				fputs(" ", writeBuffer);
			}
			fputs("\n", writeBuffer);
		}
	}
	fclose(writeBuffer);
}

/*
	sector을 읽어오고 string으로 반환
*/
string flashMemory::read(int sectorNum) {

	readBuffer = fopen(TEXT_FILE, "r+");
	char result[SECTOR_BYTE] = "";
	fseek(readBuffer, sectorNum * SECTOR_BYTE, SEEK_SET);		//파일의 가장 처음위치부터 sectorNum * 섹터의 바이트만큼 파일 포인터의 위치를 이동
	fgets(result, SECTOR_BYTE, readBuffer);
	fclose(readBuffer);
	string strResult(result);	//fgets로 읽은 char형 배열 string으로 변환(string라이브러리 함수 사용하기 위해서)
	

	strResult.erase(remove(strResult.begin(), strResult.end(), string::npos), strResult.end());	//remove를 통해 공백을, erase를 통해 비어있는 문자열을 제거
	if (strResult.compare("") == 0 || strResult.compare("\n") == 0) {
		cout << "메모리가 없습니다" << endl;
		return "";
	}
	else {
		return strResult;
	}
}

/*	
	1. 입력받은 섹터에 데이터가 존재하는지 확인(read버퍼) 
	2. 데이터가 존재하지않는 경우 데이터를 입력(write버퍼)	
*/	
void flashMemory::write(int sectorNum, string inputData) {

	const char* temp = inputData.c_str();	//string -> char* 형변환(fputs사용하기위해서 변환)
	string str;
	char result[SECTOR_BYTE] = "";

	/*	입력받은 섹터 번호에 맞는 데이터를 읽어오는 부분	*/
	readBuffer = fopen(TEXT_FILE, "r+");
	fseek(readBuffer, sectorNum * SECTOR_BYTE, SEEK_SET);		
	fgets(result, SECTOR_BYTE, readBuffer);
	fclose(readBuffer);
	string strResult(result);	//fgets로 읽은 char형 배열 string으로 변환(string라이브러리 함수 사용하기 위해서)

	/*	읽어온 데이터가 있다면 종료, 없다면 입력받은 데이터를 저장	*/	
	
	strResult.erase(remove(strResult.begin(), strResult.end(), string::npos), strResult.end());	//remove를 통해 공백을, erase를 통해 비어있는 문자열을 제거
	if(strResult.compare("") == 0 || strResult.compare("\n")==0) {
		writeBuffer = fopen(TEXT_FILE, "w+");
		cout << "PSN : " << sectorNum << ", data : " << inputData << endl;
		fseek(writeBuffer, sectorNum * SECTOR_BYTE, SEEK_SET);
		fputs(temp, writeBuffer);
		fclose(writeBuffer);
	}
	else {
		cout << "error : 이미 메모리가 존재합니다." << endl;
	}

}

void flashMemory::erase(int blockNum) {

	writeBuffer = fopen(TEXT_FILE, "w+");

	fseek(writeBuffer, blockNum * SECTOR_BYTE * BLOCK_COUNT, SEEK_SET);
	fwrite(" ", SECTOR_BYTE * BLOCK_COUNT, 0, writeBuffer);
	//한 블록에 섹터는 32개
	for (int k = 0; k < BLOCK_COUNT; k++) {
		//섹터단위
		for (int j = 0; j < SECTOR_BYTE; j++) {
			fputs(" ", writeBuffer);
		}
	}
	fclose(writeBuffer);
}