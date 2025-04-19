#include "LeaderDB.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

int main() {
    LeaderDB db;
    string path = "./dataset_project/data.csv";
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open data.csv\n";
        return 1;
    }


    /*
    string headerLine;
    getline(file, headerLine);


    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        //cout << "Line: " << line << "\n";
        string invoiceNo, stockCode, description, quantity, invoiceDate, unitPrice, customerID, country;

        getline(ss, invoiceNo, ',');
        getline(ss, stockCode, ',');
        getline(ss, description, ',');
        getline(ss, quantity, ',');
        getline(ss, invoiceDate, ',');
        getline(ss, unitPrice, ',');
        getline(ss, customerID, ',');
        getline(ss, country, ',');

        db.set(invoiceNo, description);
    }

    */
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string key, value;

        getline(ss, key, ',');
        getline(ss, value, ',');

        db.set(key, value);
    }
    file.close();

    cout << "Finished loading dataset.\n";

    string input;
    while (true) {
        cout << "Enter InvoiceNo to look up (or 'exit'): ";
        cin >> input;
        if (input == "exit") break;

        string result = db.get(input);
        if (!result.empty())
            cout << "Description: " << result << "\n";
        else
            cout << "Key not found.\n";
    }

    return 0;
}
