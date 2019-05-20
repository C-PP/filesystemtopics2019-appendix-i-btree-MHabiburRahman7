#pragma once
// btree.h

#ifndef BTREE_H
#define BTREE_H

#include "btnode.h"
#include "recfile.h"
#include "fixfld.h"
#include "indbuff.h"

// btree needs to be able to pack, unpack, read and
// 	write index records
// 	also, handle the insert, delete, split, merge,
//	growth of the tree
//	a pool of nodes will be managed
//	keep at least a branch of the tree in memory
//	
template <class keyType> class BTreeNode;
template <class keyType> 
class BTree
	// this is the full version of the BTree
{
public:
	BTree(int order, int keySize = sizeof(keyType), int unique = 1);
	~BTree();
	int Open(char * name, int mode);
	int Create(char * name, int mode);
	int Close();

	int Insert(const keyType key, const int recAddr);
	int Remove(const keyType key, const int recAddr = -1);
	int Search(const keyType key, const int recAddr = -1);
	void Print(ostream &);
	void Print(ostream &, int nodeAddr, int level);

	void InOrderTraversal(ostream &);
	void InOrderTraversal(ostream &, int nodeAddr, int level);

	int Height; // height of tree
protected:
	typedef BTreeNode<keyType> BTNode;// useful shorthand
	BTNode * FindLeaf(const keyType key); // load a branch into memory down to the leaf with key
	//BTNode * FindLeaf_ins(const keyType key); // load a branch into memory down to the leaf with key
	BTNode * NewNode();
	BTNode * Fetch(const int recaddr);
	//BTNode * Fetch_ins(const int recaddr);
	int Store(BTNode *thisNode);
	BTNode Root;
	
	int Order; // order of tree
	int PoolSize;
	BTNode ** Nodes; // pool of available nodes
					 // Nodes[1] is level 1, etc. (see FindLeaf)
					 // Nodes[Height-1] is leaf
	FixedFieldBuffer Buffer;
	RecordFile<BTNode> BTreeFile;
};

//btree.tc

const int MaxHeight = 5;
template <class keyType>
BTree<keyType>::BTree(int order, int keySize, int unique)
	: Buffer(1 + 2 * order, sizeof(int) + order * keySize + order * sizeof(int)),
	BTreeFile(Buffer), Root(order) {
	Height = 1;
	Order = order;
	PoolSize = MaxHeight * 2;
	Nodes = new BTNode *[PoolSize];
	BTNode::InitBuffer(Buffer, order);
	Nodes[0] = &Root;
}

template <class keyType>
BTree<keyType>::~BTree()
{
	Close();
	delete Nodes;
}

template <class keyType>
int BTree<keyType>::Open(char * name, int mode)
{
	int result;
	result = BTreeFile.Open(name, mode);
	if (!result) return result;
	// load root
	BTreeFile.Read(Root);
	Height = 1; // find height from BTreeFile!
	return 1;
}

template <class keyType>
int BTree<keyType>::Create(char * name, int mode) {
	int result;
	result = BTreeFile.Create(name, mode);
	if (!result) return result;
	// append root node
	result = BTreeFile.Write(Root);
	Root.RecAddr = result;
	return result != -1;
}

template <class keyType>
int BTree<keyType>::Close()
{
	int result;
	result = BTreeFile.Rewind();
	if (!result) return result;
	result = BTreeFile.Write(Root);
	if (result == -1) return 0;
	return BTreeFile.Close();
}


template <class keyType>
int BTree<keyType>::Insert(const keyType key, const int recAddr)
{
	
	int result; int level = Height - 1;
	int newLargest = 0; 

	int thisNodeAddr, newNodeAddr;

	keyType prevKey, largestKey;
	BTNode * thisNode, *newNode, *parentNode;

	thisNode = FindLeaf(key);
	//cout << "this is thisNode at beginning :?????????" << endl;
	//thisNode->Print(cout);

	// test for special case of new largest key in tree
	if (key > thisNode->LargestKey())
	{
		newLargest = 1; 
		prevKey = thisNode->LargestKey();
	}

	result = thisNode->Insert(key, recAddr);
	//thisNode->Print(cout);

	// handle special case of new largest key in tree
	if (newLargest)
		for (int i = 0; i < Height - 1; i++)
		{
			Nodes[i]->UpdateKey(prevKey, key);
			if (i > 0) Store(Nodes[i]);
		}

	while (result == -1) // if overflow and not root
	{
		//remember the largest key
		largestKey = thisNode->LargestKey();
		// split the node
		newNode = NewNode();
		//thisNode->Split(newNode);
		thisNode->Split(newNode, thisNode);
		//Nodes[level] = newNode;
		//thisNodeAddr = Store(thisNode);
		Store(thisNode);
		newNodeAddr = Store(newNode);
		//cout << "address of thisNode: " << thisNodeAddr << endl;
		//cout << "address of newNode: " << newNodeAddr << endl;

		newNode->RecAddr = newNodeAddr;
		//thisNode->UpdateKey(newNode->largestKey(), thisNode->LargestKey());

		//Store(newNode);
		level--; // go up to parent level
		if (level < 0) break;
		// insert newNode into parent of thisNode
		parentNode = Nodes[level];
		result = parentNode->UpdateKey(largestKey, thisNode->LargestKey());
		result = parentNode->Insert(newNode->LargestKey(), newNode->RecAddr);
		thisNode = parentNode;
	}
	Store(thisNode);
	if (level >= 0) return 1;// insert complete
	// else we just split the root
	
	//cout << "this is root ------" << endl;
	//Root.Print(cout);

	int newAddr = BTreeFile.Append(Root); // put previous root into file
	// insert 2 keys in new root node
	Root.Keys[0] = thisNode->LargestKey();
	Root.RecAddrs[0] = newAddr;
	Root.Keys[1] = newNode->LargestKey();
	Root.RecAddrs[1] = newNode->RecAddr;

	//Store(&Root);
	//int newAddr = BTreeFile.Append(Root); // put previous root into file
	//Root.Keys[0] = thisNode->LargestKey();
	//Root.RecAddrs[0] = newAddr;
	//Root.Keys[1] = newNode->LargestKey();
	//Root.RecAddrs[1] = newNode->RecAddr;
	Root.NumKeys = 2;
	Height++;

	//cout << "this is root ------" << endl;
	//Root.Print(cout);
	//BTreeFile.Close();

	return 1;
}


template <class keyType>
int BTree<keyType>::Remove(const keyType key, const int recAddr)
{
	// left for exercise
	int level = Height - 1;
	int result, newAddr, levelItt;
	BTNode *leafNode, *parentNode;
	leafNode = FindLeaf(key);

	if (leafNode->Search(key) == -1)
		return -1;

	//Delete at lowest level
	result = leafNode->Remove(key, recAddr);
	//cout << "this is leaf after remove   ------" << endl;
	//leafNode->Print(cout);

	//leafNode->Print(cout);
	//Nodes[level-1]->Print(cout);

	level--;

	//cout << "this is level: " << leafNode->LargestKey() << endl;
	//Update Address and clean Root
	if (leafNode->numKeys() <= 0) {
		parentNode = Nodes[level];
		result = parentNode->Remove(key, parentNode->Search(key));
		//cout << "after removal:" << endl;
		//leafNode->Print(cout);
		//parentNode->Print(cout);

		level--;

		newAddr = Store(parentNode);
		leafNode = parentNode;
		parentNode = Nodes[level];
		result = parentNode->UpdateKey(key, leafNode->LargestKey());
		result = parentNode->Insert(leafNode->LargestKey(), leafNode->RecAddr);
		//cout << "after after removal:" << endl;
		//leafNode->Print(cout);
		//parentNode->Print(cout);
	}
	else {
		//cout << "this is te largest key " << endl;
		newAddr = Store(leafNode);
		parentNode = Nodes[level];
		leafNode->RecAddr = newAddr;
		//cout << "this is Nodes - level that found  ------" << endl;
		result = parentNode->UpdateKey(parentNode->LargestKey(), leafNode->LargestKey());
		result = parentNode->Insert(leafNode->LargestKey(), leafNode->RecAddr);

		//leafNode->Print(cout);
		//parentNode->Print(cout);
		level--;
		while (level > -1) {
			leafNode = parentNode;
			parentNode = Nodes[level];

			//cout << "this is level: " << level << "-- "<< parentNode->Search(key) << endl;
			//leafNode->Print(cout);
			//parentNode->Print(cout);
			if (parentNode->Search(key) >= 1) {
				//result = parentNode->Remove(key, leafNode->Search(key));
				newAddr = Store(leafNode);
				parentNode = Nodes[level];
				leafNode->RecAddr = newAddr;

				//result = parentNode->Remove(key, parentNode->Search(key));
				//cout << "this is Nodes - level that found  ------" << endl;
				result = parentNode->UpdateKey(key, leafNode->LargestKey());
				result = parentNode->Insert(leafNode->LargestKey(), leafNode->RecAddr);

				cout << "this is after update: " << endl;
				//parentNode->Print(cout);
				level--;
			}
			else
				break;
		}
	}
	//cout << "this is nodes 0:" << endl;
	//Nodes[0]->Print(cout);
	
	//newAddr = Store(leafNode);
	//parentNode = Nodes[level--];
	//leafNode->RecAddr = newAddr;
	////cout << "this is Nodes - level that found  ------" << endl;
	//result = parentNode->UpdateKey(parentNode->LargestKey(), leafNode->LargestKey());
	//result = parentNode->Insert(leafNode->LargestKey(), leafNode->RecAddr);
	////parentNode->Print(cout);

	////level--;
	//
	//while (level > -1) {
	//	if (Nodes[level]->Search(key) >= 1) {
	//		result = Nodes[level]->UpdateKey(Nodes[level]->LargestKey(), leafNode->LargestKey());
	//		result = Nodes[level]->Insert(leafNode->LargestKey(), leafNode->RecAddr);

	//		parentNode->Print(cout);
	//		level--;
	//	}
	//}

	//Clean the root
	/*while (1) {
		if (Nodes[level]->Search(key) != -1) {
			result = Nodes[level]->Remove(key);
			level--;
		}
		else
			break;
	}*/
	
	return 1;
}

template <class keyType>
int BTree<keyType>::Search(const keyType key, const int recAddr)
{
	BTNode * leafNode;
	leafNode = FindLeaf(key);
	return leafNode->Search(key, recAddr);
}

template <class keyType>
void BTree<keyType>::Print(ostream & stream)
{
	stream << "BTree of height " << Height << " is " << endl;
	Root.Print(stream);
	if (Height > 1)
		for (int i = 0; i < Root.numKeys(); i++)
		{
			Print(stream, Root.RecAddrs[i], 2);
		}
	stream << "end of BTree" << endl;
}

template <class keyType>
void BTree<keyType>::Print
(ostream & stream, int nodeAddr, int level)
{
	//BTNode * thisNode = Fetch(62);
	BTNode * thisNode = Fetch(nodeAddr);
	stream << "BTree::Print() ->Node at level " << level << " address " << nodeAddr << ' ' << endl;

	//cout << "thisNode numKey: " << thisNode->numKeys() << endl;
	//for (int i = 0; i < thisNode->numKeys(); i++) {
	//	cout << "thisnode is real: " << thisNode->RecAddrs[i] << endl;
	//}

	thisNode->Print(stream);
	

	if (Height > level)
	{
		level++;
		for (int i = 0; i < thisNode->numKeys(); i++)
		{
			Print(stream, thisNode->RecAddrs[i], level);
		}
		stream << "end of level " << level << endl;
	}
}

template<class keyType>
inline void BTree<keyType>::InOrderTraversal(ostream & stream)
{
	stream << "BTree of height " << Height << " is " << endl;
	//Root.Print(stream);
	if (Height > 1)
		for (int i = 0; i < Root.numKeys(); i++)
		{
			InOrderTraversal(stream, Root.RecAddrs[i], 2);
		}
	stream << "end of BTree" << endl;
}

template<class keyType>
inline void BTree<keyType>::InOrderTraversal(ostream & stream, int nodeAddr, int level)
{
	BTNode * thisNode = Fetch(nodeAddr);
	//stream << "BTree::Print() ->Node at level " << level << " address " << nodeAddr << ' ' << endl;
	if(level > 2) //skip the root
		thisNode->Print(stream);

	if (Height > level)
	{
		level++;
		for (int i = 0; i < thisNode->numKeys(); i++)
		{
			InOrderTraversal(stream, thisNode->RecAddrs[i], level);
		}
		stream << "end of level " << level << endl;
	}
}

template <class keyType>
BTreeNode<keyType> * BTree<keyType>::FindLeaf(const keyType key)
// load a branch into memory down to the leaf with key
{
	int recAddr, level;
	for (level = 1; level < Height; level++)
	{
		recAddr = Nodes[level - 1]->Search(key, -1, 0);//inexact search

		Nodes[level] = Fetch(recAddr);
		//Nodes[level - 1]->Merge(Nodes[level]);
	}
	return Nodes[level - 1];
}

template <class keyType>
BTreeNode<keyType> * BTree<keyType>::NewNode()
{// create a fresh node, insert into tree and set RecAddr member
	BTNode * newNode = new BTNode(Order);
	int recAddr = BTreeFile.Append(*newNode);
	newNode->RecAddr = recAddr;
	return newNode;
}

template <class keyType>
BTreeNode<keyType> * BTree<keyType>::Fetch(const int recaddr)
{// load this node from File into a new BTreeNode
	int result;
	BTNode * newNode = new BTNode(Order);
	result = BTreeFile.Read(*newNode, recaddr);
	if (result == -1) return NULL;
	newNode->RecAddr = result;
	return newNode;
}

template<class keyType>
int BTree<keyType>::Store(BTNode * thisNode)
{
	//return BTreeFile.Write(*thisNode);
	return BTreeFile.Write(*thisNode, thisNode->RecAddr);
}

#endif
