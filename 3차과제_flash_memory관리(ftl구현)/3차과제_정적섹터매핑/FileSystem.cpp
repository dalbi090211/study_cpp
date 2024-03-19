#include<iostream>
#include<sstream>
#include<string>

#include"FTL.h"

#pragma warning(disable:4996)

using namespace std;

int StringToInt(string temp) {
	int	returnNum;
	try {
		returnNum = stoi(temp);
		return returnNum;
	}
	catch (const exception& expn) {	
		cout << expn.what() << endl;
		return -1;
	}
	catch (...) {	
		cout << "정의되지 않은 예외입니다" << endl;
		return -1;
	}
}


int main(void) {
	
	bool endTrigger = false;
	string curLine;
	string word[3];
	int i;
	int lastCursor;
	int tempCursor;
	FTL ftl;


	//명령으로 입력받은 temp가 end일 때까지 반복
	while (!endTrigger) {
		cout << "실행하실 명령을 입력해주세요(종료의 경우 end를 입력) : ";
		getline(cin, curLine);

		if (curLine.find(" ") == string::npos) {
			/*		입력받은 문장에 공백이 없는 경우		*/
			if (curLine.compare("end") != string::npos) {
				endTrigger = true;
			}
			else if(curLine.compare("Print_table") != string::npos) {
				ftl.Print_table();
			}
		}
		else {
			/*		입력받은 문장에 공백이 있는 경우		*/

			/*		문장(curLine)을 여러 개의 단어(word[])로 분해		*/
			lastCursor = 0;
			tempCursor = 0;
			word[2] = "";
			for (i = 0; i < 3; i++) {		//문장을 단어 단위(공백)로 끊고 4번째 단어부터는 저장하지않음
				tempCursor = curLine.find(" ", lastCursor);
				word[i] = curLine.substr(lastCursor, tempCursor-lastCursor);
				lastCursor = tempCursor + 1;
				if (curLine.find(" ", lastCursor) == string::npos || i == 1) {
					word[i + 1] = curLine.substr(lastCursor, curLine.size()-tempCursor);
					i = 3;
				}
			}
			
			/*		명령의 유형(word[0])에 따라 분기		*/	
			i = StringToInt(word[1]);
			if (word[0].compare("P_R") == 0) {
				if (i != -1) {
					ftl.read(i);
				}
			}
			else if (word[0].compare("P_W") == 0) {
				if (i != -1) {
					if (word[2].empty()) {
						cout << "data를 입력해주세요" << endl;
					}
					else {
						ftl.write(i, word[2]);
					}
				}
			}
			else if (word[0].compare("megabytes") == 0) {
				if (i != -1) {
					ftl.init(i);
				}
			}
		}
	}
}
