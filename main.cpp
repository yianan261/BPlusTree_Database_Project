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
         << "1. get <key>           - Retrieve one record (all attributes)\n"
         << "2. set                 - Add/overwrite a record (comma‑separated attributes)\n"
         << "3. delete <key>        - Delete a specific key\n"
         << "4. prefix <str>        - List all records whose key starts with <str>\n"
         << "5. create <table>      - Create a new table\n"
         << "6. drop <table>        - Delete a table\n"
         << "7. use <table>         - Switch to a table\n"
         << "8. tables              - List all tables\n"
         << "9. current             - Show current table name\n"
         << "10. help               - Show this help message\n"
         << "11. exit               - Exit the program\n\n";
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

        db.set(key, attrs);                
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

        /* ---------- SET ---------- */
        else if (command == "set") {
            string key;
            cout << "Key: ";
            cin  >> key;
            cout << "Attributes (comma separated): ";
            cin.ignore();              
            getline(cin, line);
            auto attrs = splitCSV(line);
            db.set(key, attrs);
            cout << "Record added/updated.\n";
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

        /* ---------- CREATE ---------- */
        else if (command == "create") {
            string tableName; cin >> tableName;
            if (db.hasTable(tableName)) {
                cout << "Table already exists.\n";
            } else {
                db.createTable(tableName);
                cout << "Table created.\n";
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

        /* ---------- UNKNOWN ---------- */
        else {
            cout << "Unknown command. Type 'help' for help.\n";
        }
    }

    cout << "Bye!\n";
    return 0;
}
