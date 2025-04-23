#include "LeaderDB.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

/* ─────────────── help ─────────────── */
void printHelp() {
     cout << "\nAvailable commands:\n"
         << "1.  get <key>           - Retrieve one record (all attributes)\n"
         << "2.  create              - Create a new record (fails if key exists)\n"
         << "3.  update              - Update an existing record (fails if key does not exist)\n"
         << "4.  delete <key>        - Delete a specific key\n"
         << "5.  prefix <str>        - List all records whose key starts with <str>\n"
         << "6.  create <table>      - Create a new table\n"
         << "7.  drop <table>        - Delete a table\n"
         << "8.  use <table>         - Switch to a table\n"
         << "9.  tables              - List all tables\n"
         << "10. current             - Show current table name\n"
         << "11. load <filepath>     - Load data from CSV file into current table\n"
         << "12. help                - Show this help message\n"
         << "13. exit                - Exit the program\n\n";
}

/* ─────────────── splitCSV ─────────────── */
static vector<string> splitCSV(const string& line) {
    vector<string> fields;
    string field;
    stringstream ss(line);
    while (getline(ss, field, ',')) fields.push_back(field);
    return fields;
}

int main() {
    LeaderDB db;

    
    const string path = "./dataset_project/data.csv";
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open " << path << '\n';
        return 1;
    }

    string line;
    while (getline(file, line)) {
        auto fields = splitCSV(line);
        if (fields.empty()) continue;

        string key = fields.front();
        vector<string> attrs(fields.begin() + 1, fields.end());
        try {
            db.create(key, attrs);
        } catch(const exception& e){
            cout << "Error: " << e.what() << '\n';       
        }
                        
    }
    file.close();
    cout << "Finished loading dataset.\n";

    
    cout << "Welcome to LeaderDB Command Line Interface!\n";
    printHelp();

    string command;
    while (true) {
        cout << "[" << db.getCurrentTable() << "] ";
        if (!(cin >> command)) break;

        if (command == "exit") break;
        else if (command == "help") printHelp();

        /* ---------- GET ---------- */
        else if (command == "get") {
            string key;  cin >> key;
            auto attrs = db.get(key);
            if (attrs.empty())
                cout << "Key not found.\n";
            else {
                cout << "Record: ";
                for (size_t i = 0; i < attrs.size(); ++i) {
                    if (i) cout << ", ";
                    cout << attrs[i];
                }
                cout << '\n';
            }
        }

        /* ---------- CREATE  ---------- */
        else if (command == "create") {
            string key;
            cout << "Key: ";
            cin  >> key;
            cout << "Attributes (comma separated): ";
            cin.ignore();              
            getline(cin, line);
            auto attrs = splitCSV(line);
            try {
                db.create(key, attrs);
                cout << "Record created.\n";
            } catch (const exception& e) {
                cout << "Error: " << e.what() << '\n';
            }
        }
        /* ---------- UPDATE ---------- */
        else if (command == "update"){
            string key;
            cout << "Key: "; cin >> key;
            cout << "Attributes (comma separated): ";
            cin.ignore(); getline(cin, line);
            auto attrs = splitCSV(line);
            try{
                db.update(key, attrs);
                cout << "Record updated.\n";
            }catch(const exception& e){
                cout << "Error: " << e.what() << '\n';
            }
        }

        /* ---------- DELETE ---------- */
        else if (command == "delete") {
            string key; cin >> key;
            db.deleteKey(key);
            cout << "Record deleted (if existed).\n";
        }

        /* ---------- PREFIX ---------- */
        else if (command == "prefix") {
            string pre; cin >> pre;
            auto rows = db.getPrefix(pre);          
            if (rows.empty()) {
                cout << "No matches.\n";
            } else {
                cout << "Matches:\n";
                for (const auto& attrs : rows) {
                    for (size_t i = 0; i < attrs.size(); ++i) {
                        if (i) cout << ", ";
                        cout << attrs[i];
                    }
                    cout << '\n';
                }
                cout << "Total: " << rows.size() << '\n';
            }
        }

        /* ---------- DROP ---------- */
        else if (command == "drop") {
            string tableName; cin >> tableName;
            if (tableName == "default") {
                cout << "Cannot drop default table.\n";
            } else if (!db.hasTable(tableName)) {
                cout << "Table does not exist.\n";
            } else {
                db.dropTable(tableName);
                cout << "Table dropped.\n";
            }
        }

        /* ---------- USE ---------- */
        else if (command == "use") {
            string tableName; cin >> tableName;
            if (!db.hasTable(tableName)) {
                cout << "Table does not exist.\n";
            } else {
                db.switchTable(tableName);
                cout << "Switched to table: " << tableName << "\n";
            }
        }

        /* ---------- TABLES ---------- */
        else if (command == "tables") {
            auto tables = db.listTables();
            cout << "Tables:\n";
            for (const auto& table : tables) {
                cout << "- " << table << "\n";
            }
        }

        /* ---------- CURRENT ---------- */
        else if (command == "current") {
            cout << "Current table: " << db.getCurrentTable() << "\n";
        }

        /* ---------- LOAD ---------- */
        else if (command == "load") {
            string filepath;
            cout << "Enter CSV file path: ";
            cin >> filepath;
            
            if (db.loadFromFile(db.getCurrentTable(), filepath)) {
                cout << "Data loaded successfully into table: " << db.getCurrentTable() << "\n";
            } else {
                cout << "Failed to load data from " << filepath << "\n";
            }
        }

        /* ---------- UNKNOWN ---------- */
        else {
            cout << "Unknown command. Type 'help' for help.\n";
        }
    }

    cout << "Bye!\n";
    return 0;
}
