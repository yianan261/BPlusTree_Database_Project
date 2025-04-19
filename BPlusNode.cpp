#include "BPlusNode.h"

BPlusNode::BPlusNode(bool leaf) : isLeaf(leaf), next(nullptr), prev(nullptr) {}

bool BPlusNode::isLeafNode() const {
    return isLeaf;
}

vector<int>& BPlusNode::getKeys() {
    return keys;
}

vector<BPlusNode*>& BPlusNode::getChildren() {
    return children;
}

vector<Entry>& BPlusNode::getEntries() {
    return entries;
}

BPlusNode* BPlusNode::getNext() const {
    return next;
}

BPlusNode* BPlusNode::getPrev() const {
    return prev;
}

void BPlusNode::setNext(BPlusNode* n) {
    next = n;
}

void BPlusNode::setPrev(BPlusNode* p) {
    prev = p;
}

InternalNode::InternalNode() : BPlusNode(false) {}

vector<Entry>& InternalNode::getEntries(){
    //throw logic_error("InternalNode does not store entries.");
    return entries; // This is just a placeholder. Internal nodes typically do not have entries.
}

bool InternalNode::isLeafNode() const {
    return false;
}

LeafNode::LeafNode() : BPlusNode(true) {}

bool LeafNode::isLeafNode() const {
    return true;
}