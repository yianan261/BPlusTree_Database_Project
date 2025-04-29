#include "SecondaryIndex.h"
#include <algorithm>
#include <iostream>
void SecondaryIndex::insert(const vector<string>& attrs, int pk) {
    if (attrs.empty() || col < 0 || static_cast<size_t>(col) >= attrs.size()) 
        return;
    string val = attrs[col];

    vector<int> findit = tree.search(val);

    if (findit.empty()){
        vector<int> pks = {pk};  
        tree.insert(val, pks);
        return ;
    } 
    if (find(findit.begin(), findit.end(), pk) == findit.end())
        findit.push_back(pk);
    tree.remove(val);
    tree.insert(val, findit);
    
}

void SecondaryIndex::remove(const vector<string>& attrs, int pk)
{   
    if (col >= static_cast<int>(attrs.size()))
        return;

    string val = attrs[col];

    auto list = tree.search(val);
    if (list.empty())       
        return;

    list.erase(std::remove(list.begin(), list.end(), pk), list.end());
    tree.remove(val);
    if (!list.empty())
        tree.insert(val, list);
}

vector<int> SecondaryIndex::searchPK(const string& v)
{
    string key = v;
    return tree.search(key);
}


template<typename RowIter>
void SecondaryIndex::bulkBuild(RowIter first, RowIter last)
{
    for (auto it = first; it != last; ++it){
        insert(it->second, it->first);   
    }
        
}
template void SecondaryIndex::bulkBuild<vector<pair<int,vector<string>>>::iterator>(
    vector<pair<int,vector<string>>>::iterator,
    vector<pair<int,vector<string>>>::iterator);
