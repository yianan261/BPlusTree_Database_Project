#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "BPlusNode.h"
#include <string>
using namespace std;

class BPlusTree {
private:
    BPlusNode* root;

    void insertInternal(int key, const vector<string>& attrs, BPlusNode* node, BPlusNode*& newChild, int& newKey);
    void splitLeaf(BPlusNode* node, BPlusNode*& newChild, int& newKey);
    void splitInternal(BPlusNode* node, BPlusNode*& newChild, int& newKey);
    bool deleteEntry(BPlusNode* node, int key);
    void borrowLeaf(BPlusNode* node);
    void mergeLeaf(BPlusNode* node);
    void printTree(BPlusNode* node, int level);

public:
    BPlusTree();

    void insert(int key, const vector<string>& attrs);
    void remove(int key);
    vector<string> search(int key);
    void print();
    void printLeaves();
};

#endif // BPLUSTREE_H