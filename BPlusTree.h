#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "BPlusNode.h"
#include <set>
#include <vector>
#include <string>
using namespace std;

template<typename K, typename P>
class BPlusTree {
private:
    BPlusNode<K,P>* root;
    set<K> keySet;
    
    void splitLeaf(BPlusNode<K,P>* node, BPlusNode<K,P>*& newChild, K& newKey);
    void splitInternal(BPlusNode<K,P>* node, BPlusNode<K,P>*& newChild, K& newKey);
    void insertInternal(K& key, P& attrs, BPlusNode<K,P>* node, BPlusNode<K,P>*& newChild, K& newKey);
    bool deleteEntry(BPlusNode<K,P>* node, const K& key);
    void borrowLeaf(BPlusNode<K,P>* node);
    void mergeLeaf(BPlusNode<K,P>* node);
    void printTree(BPlusNode<K,P>* node, int level);

public:
    BPlusTree();
    void insert(K& key, const P& attrs);  // 修改为 const 引用
    void update(K& key, P& attrs);
    void remove(K& key);
    P search(K& key);
    vector<P> rangeQuery(K lowKey, K highKey);
    bool contains(K& key) const;
    void print();
    void printLeaves();
    
    template<typename Fn>
    void forEachLeaf(Fn&& f) const {
        const BPlusNode<K,P>* node = root;
        while (!node->isLeafNode()) {
            node = node->getChildren().front();
        }
        while (node) {
            for (const auto& e : static_cast<const LeafNode<K,P>*>(node)->getEntries()) f(e);
            node = node->getNext();
        }
    }
};

#endif