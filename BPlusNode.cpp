#include "BPlusNode.h"


template<typename K, typename P>
BPlusNode<K,P>::BPlusNode(bool leaf) : isLeaf(leaf), next(nullptr), prev(nullptr) {}

template<typename K, typename P>
bool BPlusNode<K,P>::isLeafNode() const {
    return isLeaf;
}

template<typename K, typename P>
vector<K>& BPlusNode<K,P>::getKeys() {
    return keys;
}

template<typename K, typename P>
const vector<K>& BPlusNode<K,P>::getKeys() const {
    return keys;
}

template<typename K, typename P>
vector<BPlusNode<K,P>*>& BPlusNode<K,P>::getChildren() {
    return children;
}

template<typename K, typename P>
const vector<BPlusNode<K,P>*>& BPlusNode<K,P>::getChildren() const {
    return children;
}

template<typename K, typename P>
vector<Entry<K, P>>& BPlusNode<K,P>::getEntries() {
    return entries;
}

template<typename K, typename P>
const vector<Entry<K, P>>& BPlusNode<K,P>::getEntries() const {
    return entries;
}

template<typename K, typename P>
BPlusNode<K,P>* BPlusNode<K,P>::getNext() const {
    return next;
}

template<typename K, typename P>
BPlusNode<K,P>* BPlusNode<K,P>::getPrev() const {
    return prev;
}

template<typename K, typename P>
void BPlusNode<K,P>::setNext(BPlusNode<K,P>* n) {
    next = n;
}

template<typename K, typename P>
void BPlusNode<K,P>::setPrev(BPlusNode<K,P>* p) {
    prev = p;
}

template<typename K, typename P>
InternalNode<K, P>::InternalNode() : BPlusNode<K,P>(false) {}

template<typename K, typename P>
vector<Entry<K, P>>& InternalNode<K, P>::getEntries(){
    return this->entries; 
}

template<typename K, typename P>
const vector<Entry<K, P>>& InternalNode<K, P>::getEntries() const {
    return this->entries;
}

template<typename K, typename P>
bool InternalNode<K, P>::isLeafNode() const {
    return false;
}

template<typename K, typename P>
LeafNode<K, P>::LeafNode() : BPlusNode<K,P>(true) {}

template<typename K, typename P>
vector<Entry<K, P>>& LeafNode<K, P>::getEntries(){
    return this->entries;
}

template<typename K, typename P>
const vector<Entry<K, P>>& LeafNode<K, P>::getEntries() const {
    return this->entries;
}

template<typename K, typename P>
bool LeafNode<K, P>::isLeafNode() const {
    return true;
}


template struct Entry<int, vector<string>>;
template class BPlusNode<int, vector<string>>;
template class InternalNode<int, vector<string>>;
template class LeafNode<int, vector<string>>;

template struct Entry<string, vector<int>>;
template class  BPlusNode<string, vector<int>>;
template class  InternalNode<string, vector<int>>;
template class  LeafNode<string, vector<int>>;