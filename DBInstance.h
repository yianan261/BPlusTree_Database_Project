#ifndef DBINSTANCE_H
#define DBINSTANCE_H
#include <string>
#include <vector>

using namespace std;

// abstract class
class DBInstance {
public:
    virtual ~DBInstance() = default;

    virtual void set(const string& key, const string& value) = 0;
    virtual string get(const string& key) = 0;
    virtual void deleteKey(const string& key) = 0;
    virtual vector<string> getPrefix(const string& prefixKey) = 0;
};

#endif