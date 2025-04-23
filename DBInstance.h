#ifndef DBINSTANCE_H
#define DBINSTANCE_H
#include <string>
#include <vector>

using namespace std;

// abstract class
class DBInstance {
public:
    virtual ~DBInstance() = default;

    virtual void create(const string& key, const vector<string>& attrs) = 0;
    virtual void update(const string& key, const vector<string>& attrs) = 0;
    virtual vector<string> get(const string& key) = 0;
    virtual void deleteKey(const string& key) = 0;
    virtual vector<vector<string>> getPrefix(const string& prefixKey) = 0;
};

#endif