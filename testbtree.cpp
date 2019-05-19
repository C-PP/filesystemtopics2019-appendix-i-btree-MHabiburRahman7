//#pragma warning(disable:4996)
//tstbtree.cc
//#include "btnode.h"

#include <iostream>
#include <string>
#include "btree.h"
using namespace std;

const char * keys = "CSDTAMPIBWNGURKEHOLJYQZFXV";
//const char * keys = "CSDTA";

const int BTreeSize = 4;
int main(int argc, char * argv) {
	int result, i;
	BTree <char> bt(BTreeSize);

	int sel;

	while (1) {
		cout << "\nSelect command 1: record input, 2. display, 3. write to file, 4. read from file, 5. direct read from file, 6. Quit => ";
		cin >> sel;

		switch (sel)
		{
		case 1: {
			cout << "input data = " << keys << endl;
			result = bt.Create("btree.dat", ios::out);
			if (!result) {
				cout << "Please delete btree.dat" << endl;
				system("pause");
				//return 0;
			}

			//for (i = 0; i<26; i++)
			for (i = 0; i < 20; i++)
			{
				cout << "Inserting " << keys[i] << endl;
				result = bt.Insert(keys[i], i);
				//result = bt.Insert(keys[i], i);
				/*bt.Print(cout);*/
			}

			bt.Close();

			break;
		}
		case 2: {
			result = bt.Open("btree.dat", ios::in);
			if (!result) {
				cout << "cannot found btree.dat" << endl;
				system("pause");
				//return 0;
			}
			break;
		}
		case 3: {
			//bt.Height = 2;
			bt.Print(cout);
			break;
		}
		case 4: {
			bt.Search('A');
			break;
		}

		default: {
			exit(0);
			break;
		}
		}
	}
		
	//bt.Print(cout);

	//bt.Search(1, 1);

	//system("pause");
	return 1;
}