#include "BPlusTree.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main() {
    BPlusTree tree;

    ifstream file("data.csv");
    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        return 1;
    }
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string keyStr, value;
        if(!getline(ss, keyStr, ',')) {
            cerr << "Error reading key from line: " << line << endl;
            continue;
        }
        if(!getline(ss, value)) {
            cerr << "Error reading value from line: " << line << endl;
            continue;
        }
        try{
            int key = stoi(keyStr);
            tree.insert(key, value);
            
        }   catch (const invalid_argument& e) {
            cerr << "Invalid key format in line: " << line << endl;
        } 
        
    }

    file.close();
    
    tree.insert(10, "ten");
    tree.insert(20, "twenty");
    tree.insert(5, "five");

    tree.print();
    cout << "Search 10: " << tree.search(10) << endl;

    tree.remove(10);
    tree.printLeaves();
    



    return 0;
}
