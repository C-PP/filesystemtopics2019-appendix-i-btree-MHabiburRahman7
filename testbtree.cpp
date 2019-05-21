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

	int select;

	while (1) {
		cout << "\nSelect command 1: Insert all character, 2. Print Btree, 3. Print Btree inOrderTransversal, 4. Search Key, 5. Remove Key, 6. Quit => ";
		cin >> select;

		switch (select)
		{
		case 1: {
			cout << "input data = " << keys << endl;
			result = bt.Create("btree.dat", ios::out);
			if (!result) {
				cout << "Please delete btree.dat" << endl;
				system("pause");
				break;
			}

			for (i = 0; i < 26; i++)
			{
				cout << "Inserting " << keys[i] << endl;
				result = bt.Insert(keys[i], i);
				//bt.Print(cout);
			}

			break;
		}
		case 2: {
			cout << "Print Btree from the root :" << endl;
			bt.Print(cout);
			break;
		}
		case 3: {
			cout << "Print Btree inOrderTraversal mode :" << endl;
			bt.InOrderTraversal(cout);
			break;
		}
		case 4: {
			char ser;

			cout << "Key to Search :";
			cin >> ser;

			result = bt.Search(ser);
			if (result == -1) {
				cout << "key : " << ser << " --- does not found" << endl;
				break;
			}
			cout << "key : " <<ser << " --- with address :" << result << endl;
			break;
		}
		case 5: {
			char ser;

			cout << "Key to remove :";
			cin >> ser;

			result = bt.Remove(ser);
			if (result == -1) {
				cout << "key : " << ser << " --- does not found in this tree " << endl;
				break;
			}
			cout << "key : " << ser << " successfully removed!" << endl;
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