#define MEGABYTE 1048576
#define BLOCK_COUNT 32
#define SECTOR_BYTE 512
#define SPARE_LENGTH 8

#include<string>
#include<iostream>

#include"FTL.h"

using namespace std;

/*	정적-섹터 매핑
	1. 1블록에서 32섹터 중 24섹터를 사용하고 8섹터를 수정섹터로 잡음
	2. 수정이 일어났을 때 데이터를 옮길 여분의 섹터는 해당 블록내에 마련해놨기에 dram에서 여분섹터의 주소를 잡고있을 필요가 없음
	3. 다만 수정섹터가 가득차서 가비지컬렉션을 돌릴때 데이터를 잠시 dram에 올려놔야되기에 24섹터가 필요함
	4. 소프트웨어는 메모장, 그림판과 같이 거의 업데이트가 이루어지지 않는 소프트웨어도 있고 업데이트가 되더라도 일부의 파일만 변경이 이루어진다.
	5. 따라서 실제 플래쉬 메모리에서는 데이터가 사용된 횟수, 수정된 횟수, 설치된 날짜등을 이용해 종합적으로 데이터를 구분하고 여분섹터의 개수도 세부적으로 나뉠꺼라고 생각하지만
	이 코드에서는 단순히 수정여부에 따라 수정된적이 없는 데이터는 32섹터의 블록에 저장하고 수정된 데이터는 24섹터 + 8여분섹터의 블록에 저장
*/

/* 메모리 할당
* 
*	1. 입력받은 메가바이트에 맞는 개수의 블록만큼 생성
*	2. 생성한 블록만큼 매핑테이블을 생성 및 매핑테이블을 초기화(flashmemory와 매핑테이블과 일치시킴, 정적이므로 값이 없더라도 매핑자체는 다 되어있어야함)
*	3. 삭제테이블과 여분테이블 생성(여분테이블이 없으면 남는 좌표를 알아낼 방법이 read밖에 없기에 초기에는 spare_block을 잡고있어야함)
*/
void FTL::init(int megabytes) {

	//megabytes/3*4만큼 플래쉬메모리를 생성
	if (blockMax == 0) {
		blockMax = (megabytes * MEGABYTE) / (SECTOR_BYTE * BLOCK_COUNT);
		spareMax = blockMax / 4;
		mappingTable = new int[blockMax * (BLOCK_COUNT - SPARE_LENGTH)];	//매핑테이블 생성(스페어를 추가하지않은 입력받은 크기만큼)(sector단위)
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
		cout << "이미 할당된 메모리가 있습니다." << endl;
	}
}

/*	읽기
* 
*	1. 입력받은 sectorNum이 생성된 플래쉬 메모리보다 큰 지 확인(예외처리)
*	2. 논리적 주소는 고정되어 있으므로 mappingTable에 논리적 주소를 인덱스로 넣으면 매핑. 반환 값을 flash.read()에 넣어 읽기
*	3. 읽어온 값이 존재한다면 리턴, 없으면 오류메세지 출력.
*
*/
void FTL::read(int sectorNum) {
	int read = 0;
	string readData = "";

	if (blockMax == 0) {
		cout << "할당된 플래시 메모리가 없습니다." << endl;
	}
	else {
		if (sectorNum > blockMax*(BLOCK_COUNT - SPARE_LENGTH)) {
			cout << "할당된 메모리 주소보다 큽니다." << endl;
		}
		else {
			readData = memory.read(mappingTable[sectorNum]);
			read++;
			if (readData.compare("") == 0) {
				cout << "해당 섹터에 데이터가 없습니다." << endl;
			}
			else {
				cout << sectorNum/(BLOCK_COUNT - SPARE_LENGTH) << "-" << sectorNum % (BLOCK_COUNT - SPARE_LENGTH) << ", data : " << readData << endl;
			}
		}
	}
	cout << "read : " << read << endl;
}

/*	쓰기
* 
*	1. 할당된 메모리가 있는지, 입력받은 sectorNum이 flash memory의 크기보다 큰 지 확인
*	2. 데이터가 존재하는지 입력받은 sectorNum을 이용해 flash memory에 read
*   3-1. 데이터가 존재하지 않는다면 입력받은 data를 flash memory에 write
*	3-2. 데이터가 존재한다면 해당 블록에 여분섹터를 찾을때까지 read
*	3-2-1. 여분섹터를 찾은 경우 mappingtable의 주소를 여분섹터로 바꿔주고 write
*	3-2-2. 여분섹터를 찾지 못한 경우 블록에서 해당 섹터를 제외한 데이터 전체를 읽어들이고 erase한 후 블록 전체를 write(따라서 dram의 최소 크기는 변수가 들고있는 매개 변수 + 1블록)
*	3-2-2. 이때 write하는 블록은 spareBlock[0], 기존의 블록은 spareBlock[spareMax-1]에 저장하여 하나의 블록이 너무 자주 사용되지 않게 처리
*/
void FTL::write(int sectorNum, string data) {

	int temp;
	int read = 0;
	int write = 0;
	int erase = 0;
	string* readData = new string[BLOCK_COUNT - SPARE_LENGTH];

	if(blockMax == 0) {	//1
		cout << "할당된 플래시 메모리가 없습니다." << endl;
	}
	else {
		if (sectorNum > blockMax * (BLOCK_COUNT - SPARE_LENGTH)) {
			cout << "할당된 메모리 주소보다 큽니다." << endl;
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
				for (int i = temp + (BLOCK_COUNT - SPARE_LENGTH); i < temp + BLOCK_COUNT; i++) {	//여분섹터 찾기, temp는 물리주소 sectorNum은 논리주소로 사용
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
				if (temp != -1) {	//3-2-2, 가비지 컬렉션
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


//매핑테이블 출력
void FTL::Print_table() {
	cout << "mapping table : " << endl;
	for (int i = 0; i < blockMax * (BLOCK_COUNT - SPARE_LENGTH); i++) {	//칸 맞출꺼면 나중에 printf써서 %4d 이런 방식 활용하는게 좋을듯
		cout << i << " : " << mappingTable[i] << endl;
	}
}