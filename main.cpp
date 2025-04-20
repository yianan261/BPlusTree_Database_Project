#include "LeaderDB.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

void printHelp() {
    cout << "\nAvailable commands:\n"
         << "1. get <key>     - Retrieve value for a specific key\n"
         << "2. set           - Add new key-value pair\n"
         << "3. delete <key>  - Delete a specific key\n"
         << "4. prefix <str>  - Find all entries with given prefix\n"
         << "5. help         - Show this help message\n"
         << "6. exit         - Exit the program\n\n";
}

int main() {
    LeaderDB db;
    string path = "./dataset_project/data.csv";
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open data.csv\n";
        return 1;
    }

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
    cout << "Welcome to LeaderDB Command Line Interface!\n";
    printHelp();

    string command, input;
    while (true) {
        cout << "\nEnter command (type 'help' for commands): ";
        cin >> command;

        if (command == "exit") {
            break;
        }
        else if (command == "help") {
            printHelp();
        }
        else if (command == "get") {
            cout << "Enter key to look up: ";
            cin >> input;
            string result = db.get(input);
            if (!result.empty())
                cout << "Value: " << result << "\n";
            else
                cout << "Key not found.\n";
        }
        else if (command == "set") {
            string key, value;
            cout << "Enter key: ";
            cin >> key;
            cout << "Enter value: ";
            cin.ignore();
            getline(cin, value);
            db.set(key, value);
            cout << "Record added successfully.\n";
        }
        else if (command == "delete") {
            cout << "Enter key to delete: ";
            cin >> input;
            db.deleteKey(input);
            cout << "Record deleted (if existed).\n";
        }
        else if (command == "prefix") {
            cout << "Enter prefix to search: ";
            cin >> input;
            vector<string> results = db.getPrefix(input);
            if (results.empty()) {
                cout << "No matches found.\n";
            } else {
                cout << "Matching entries:\n";
                for (const auto& result : results) {
                    cout << result << "\n";
                }
                cout << "Total matches: " << results.size() << "\n";
            }
        }
        else {
            cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }

    cout << "Thank you for using LeaderDB!\n";
    return 0;
}
