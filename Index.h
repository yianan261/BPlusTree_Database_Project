#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>

using namespace std;

class Index {
public:
    virtual void insert(int key, const vector<string>& value) = 0;
    virtual void remove(int key) = 0;
    virtual vector<string> search(int key) = 0;
    virtual ~Index() = default;
};

#endif
