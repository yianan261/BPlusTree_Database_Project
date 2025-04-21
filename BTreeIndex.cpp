#include "BTreeIndex.h"
#include <sstream>

void BTreeIndex::insert(const string& key, const string& value) {
    tree.insert(stoi(key), value);
}

string BTreeIndex::search(const string& key) {
    return tree.search(stoi(key));
}

void BTreeIndex::remove(const string& key) {
    tree.remove(stoi(key));
}

void BTreeIndex::clear(){
    tree = BPlusTree(); // reinitialize new tree
}

//TODO: maybe add later
vector<string> BTreeIndex::rangeQuery(const string& lowKey, const string& highKey) {
    return {};
}

void BTreeIndex::print() {
    tree.print();
}
