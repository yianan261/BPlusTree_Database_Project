#include "CsvParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

vector<Place> parseCsvFile(const string& filePath){
    vector<Place> places;
    ifstream file(filePath);
    if (! file.is_open()){
        cerr << " Failed to open" << filePath << '\n';
        return places;
    }

    string line;
    getline(file, line);

    while(getline(file, line)){
        stringstream ss(line);
        Place p;
        getline(ss, p.placeId, ',');
        getline(ss, p.name, ',');
        getline(ss, p.address, ',');
        getline(ss, p.latitude, ',');
        getline(ss, p.longitude, ',');
        if (! p.placeId.empty()){
            places.push_back(p);
        }
    }
    file.close();
    return places;
}