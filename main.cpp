#include "LeaderDB.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

/* ─────────────── CLI 帮助 ─────────────── */
void printHelp() {
    cout << "\nAvailable commands:\n"
         << "1. get <key>     - Retrieve one record (all attributes)\n"
         << "2. set           - Add/overwrite a record (comma‑separated attributes)\n"
         << "3. delete <key>  - Delete a specific key\n"
         << "4. prefix <str>  - List all records whose key starts with <str>\n"
         << "5. help          - Show this help message\n"
         << "6. exit          - Exit the program\n\n";
}

/* ─────────────── 把一行 CSV 切成字段 ─────────────── */
static vector<string> splitCSV(const string& line) {
    vector<string> fields;
    string field;
    stringstream ss(line);
    while (getline(ss, field, ',')) fields.push_back(field);
    return fields;
}

int main() {
    LeaderDB db;

    /* ① 批量加载 CSV */
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

        string            key   = fields.front();
        vector<string>    attrs(fields.begin() + 1, fields.end());

        db.set(key, attrs);                // 存入所有属性
    }
    file.close();
    cout << "Finished loading dataset.\n";

    /* ② 交互式 CLI */
    cout << "Welcome to LeaderDB Command Line Interface!\n";
    printHelp();

    string command;
    while (true) {
        cout << "\n> ";
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
            cin.ignore();              // flush '\n'
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
            auto rows = db.getPrefix(pre);          // vector<vector<string>>
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

        /* ---------- UNKNOWN ---------- */
        else {
            cout << "Unknown command. Type 'help' for help.\n";
        }
    }

    cout << "Bye!\n";
    return 0;
}
