#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "BPlusNode.h"
#include <string>
using namespace std;

class BPlusTree {
    private:
        BPlusNode* root;
    
        void insertInternal(int key, string value, BPlusNode* node, BPlusNode*& newChild, int& newKey);
        void splitLeaf(BPlusNode* node, BPlusNode*& newChild, int& newKey);
        void splitInternal(BPlusNode* node, BPlusNode*& newChild, int& newKey);
        bool deleteEntry(BPlusNode* node, int key);
        void mergeLeaf(BPlusNode* node);
        void borrowLeaf(BPlusNode* node);
        void printTree(BPlusNode* node, int level);
    
    public:
        BPlusTree() { root = new BPlusNode(true); }
    
        void insert(int key, string value);
        void remove(int key);
        void print();
        void printLeaves();
        string search(int key);
    };
    

#endif // BPLUSTREE_H