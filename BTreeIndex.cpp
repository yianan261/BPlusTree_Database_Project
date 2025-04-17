#include "BTreeIndex.h"
#include <sstream>

void BTreeIndex::insert(const std::string& key, const std::string& value) {
    tree.insert(std::stoi(key), value);
}

std::string BTreeIndex::search(const std::string& key) {
    return tree.search(std::stoi(key));
}

void BTreeIndex::remove(const std::string& key) {
    tree.remove(std::stoi(key));
}

//TODO: maybe add later
std::vector<std::string> BTreeIndex::rangeQuery(const std::string& lowKey, const std::string& highKey) {
    return {};
}

void BTreeIndex::print() {
    tree.print();
}
