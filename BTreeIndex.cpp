#include "BTreeIndex.h"
#include <sstream>

void BTreeIndex::insert(const string& key, const vector<string>& attrs){
    tree.insert(stoi(key), attrs);   
}

vector<string> BTreeIndex::search(const string& key)
{
    return tree.search(stoi(key));
}

void BTreeIndex::remove(const string& key) {
    tree.remove(stoi(key));
}

void BTreeIndex::clear(){
    tree = BPlusTree(); // reinitialize new tree
}

void BTreeIndex::update(const string& key, const vector<string>& attrs){
    tree.update(stoi(key), attrs);
}

//TODO: maybe add later
vector<vector<string>> BTreeIndex::rangeQuery(const string& lowKey, const string& highKey) {
    return {};
}

void BTreeIndex::print() {
    tree.print();
}

bool BTreeIndex::contains(int key) const {
    return tree.contains(key);
}