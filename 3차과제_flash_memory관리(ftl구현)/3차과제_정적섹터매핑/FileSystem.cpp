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
		cout << "���ǵ��� ���� �����Դϴ�" << endl;
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


	//������� �Է¹��� temp�� end�� ������ �ݺ�
	while (!endTrigger) {
		cout << "�����Ͻ� ����� �Է����ּ���(������ ��� end�� �Է�) : ";
		getline(cin, curLine);

		if (curLine.find(" ") == string::npos) {
			/*		�Է¹��� ���忡 ������ ���� ���		*/
			if (curLine.compare("end") != string::npos) {
				endTrigger = true;
			}
			else if(curLine.compare("Print_table") != string::npos) {
				ftl.Print_table();
			}
		}
		else {
			/*		�Է¹��� ���忡 ������ �ִ� ���		*/

			/*		����(curLine)�� ���� ���� �ܾ�(word[])�� ����		*/
			lastCursor = 0;
			tempCursor = 0;
			word[2] = "";
			for (i = 0; i < 3; i++) {		//������ �ܾ� ����(����)�� ���� 4��° �ܾ���ʹ� ������������
				tempCursor = curLine.find(" ", lastCursor);
				word[i] = curLine.substr(lastCursor, tempCursor-lastCursor);
				lastCursor = tempCursor + 1;
				if (curLine.find(" ", lastCursor) == string::npos || i == 1) {
					word[i + 1] = curLine.substr(lastCursor, curLine.size()-tempCursor);
					i = 3;
				}
			}
			
			/*		����� ����(word[0])�� ���� �б�		*/	
			i = StringToInt(word[1]);
			if (word[0].compare("P_R") == 0) {
				if (i != -1) {
					ftl.read(i);
				}
			}
			else if (word[0].compare("P_W") == 0) {
				if (i != -1) {
					if (word[2].empty()) {
						cout << "data�� �Է����ּ���" << endl;
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
