#ifndef CSVPARSER_H
#define CSVPARSER_H
#include <string>
#include <vector>

using namespace std;

struct Place {
    string placeId;
    string name;
    string address;
    string latitude;
    string longitude;
};

vector<Place> parseCsvFile(const string& filepath);

#endif