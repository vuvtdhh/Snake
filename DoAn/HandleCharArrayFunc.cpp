#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
using namespace std;

int CompareString(char *s0, char *s1) {
	int n0 = strlen(s0);
	int n1 = strlen(s1);
	int i, n = (n0 < n1) ? n0 : n1;
	for (i = 0; i < n; i++) {
		if (s0[i] > s1[i])
			return 1;
		else if (s0[i] < s1[i])
			return -1;
	}
	if (n0 > n)
		return 1;
	if (n1 > n)
		return -1;
	return 0;
}

bool IsSubString(char *str, char *substr) {
	int length = strlen(str);
	int sublength = strlen(substr);
	for (int i = 0; i < length; i++) {
		char *temp = new char[sublength + 1];
		/*strncpy(temp, str + i, sublength);
		temp[sublength] = '\0';*/
		strncpy(temp, str + i, sublength);
		if (CompareString(temp, substr) == 0)
			return true;
	}
	return false;
}