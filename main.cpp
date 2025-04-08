#include "BPlusTree.h"
#include "BPlusNode.h"
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
    /*
        tree.insert(10, "ten");
        tree.insert(20, "twenty");
        tree.insert(5, "five");
    */


    int number;
    cout << "please enter the number of key-value pairs you want to insert" << endl;
    cin >> number;
    while(number--){
        int a;
        string b;
        cout << "please enter the key number you want to insert(it must be an integer)" << endl;
        cin >> a;
        cout << "please enter the value you want to insert(it must be a string)" << endl;
        cin >> b;
        tree.insert(a, b);
    }
    cout << "Here is the B+ tree after inserting:" << endl;


    tree.print();
    //cout << "Search 10: " << tree.search(10) << endl;

    //tree.remove(10);
    //tree.printLeaves();

    return 0;
}
