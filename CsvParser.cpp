#include "CsvParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

string parseField(stringstream& ss) {
    string field;
    
    
    while (ss.peek() == ' ' || ss.peek() == '\t') {
        ss.get();
    }
    
    if (ss.peek() == '"') {
        ss.get();  
        
        
        bool inQuote = true;
        while (inQuote && ss.peek() != EOF) {
            char c = ss.get();
            if (c == '"') {
                if (ss.peek() == '"') {  
                    field += '"';
                    ss.get();  
                } else {
                    inQuote = false;  
                }
            } else {
                field += c;
            }
        }
        
        
        while (ss.peek() == ' ' || ss.peek() == '\t') ss.get();
        if (ss.peek() == ',') ss.get();
        
    } else {
        
        while (ss.peek() != ',' && ss.peek() != EOF) {
            field += ss.get();
        }
        if (ss.peek() == ',') ss.get();
        
        
        while (!field.empty() && (field.back() == ' ' || field.back() == '\t')) {
            field.pop_back();
        }
    }
    
    return field;
}

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
        p.placeId = parseField(ss);
        p.name = parseField(ss);
        p.address = parseField(ss);
        p.latitude = parseField(ss);
        p.longitude = parseField(ss);
        p.description = parseField(ss);

        if (!p.placeId.empty()) {
            places.push_back(p);
        }
    }
    file.close();
    return places;
}