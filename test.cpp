#include "stdafx.h"
#include "c++test.h"
#include <iostream>
using namespace std;


class ID list[20];

int main( void)
{
	int num = 0;
	int n1, n2;
	int cnt=0;
	char str[10];
	while (num != 5)
	{
		MenuPrint();
		cout << "¼±ÅÃ :";
		cin >> num;
		switch (num)
		{ 
			case 1:
				Create(list,cnt);
				break;
			case 2:
				deposit(list);
				break;
			case 3:
				break;
			case 4:
				Printing(list);
				break;
		}
		
	}
	return 0;
}