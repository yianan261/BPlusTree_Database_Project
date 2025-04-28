#include "BTreeIndex.h"
#include <sstream>
#include <algorithm>  

void BTreeIndex::insert(const string& key, const vector<string>& attrs){
    auto attrsCopy = attrs;
    int intKey = stoi(key);
    tree.insert(intKey, attrsCopy);   
}

vector<string> BTreeIndex::search(const string& key)
{
    int intKey = stoi(key);
    return tree.search(intKey);
}

void BTreeIndex::remove(const string& key) {
    int intKey = stoi(key);
    tree.remove(intKey);
}

void BTreeIndex::clear(){
    tree = PrimTree(); // reinitialize new tree
}

void BTreeIndex::update(const string& key, const vector<string>& attrs){
    auto attrsCopy = attrs;
    int intKey = stoi(key);
    tree.update(intKey, attrsCopy);
}

vector<vector<string>> BTreeIndex::rangeQuery(const string& lowKey, const string& highKey) {
    return tree.rangeQuery(stoi(lowKey),stoi(highKey));
}

void BTreeIndex::print() {
    tree.print();
}

bool BTreeIndex::contains(int key) const {
    return tree.contains(key);
}

int BTreeIndex::getColumnIndex(const string& columnName) const {
    auto it = find(headers.begin(), headers.end(), columnName);
    if(it == headers.end()) return -1;
    return it - headers.begin();
}
