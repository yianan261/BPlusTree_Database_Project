#ifndef SECONDARYINDEX_H
#define SECONDARYINDEX_H
#include "BPlusTree.h"
#include <vector>
#include <string>
using namespace std;


class SecondaryIndex {
    using PKList = vector<int>;
    using TreeType = BPlusTree<string, PKList>;   

    int col;     
    TreeType tree;

public:
    explicit SecondaryIndex(int column = 0) : col(column) {}  // 添加默认值

    void insert(const vector<string>& attrs, int pk);
    void remove(const vector<string>& attrs, int pk);

    vector<int> searchPK(const string& value);

    template<typename RowIter>
    void bulkBuild(RowIter first, RowIter last);
};

#endif