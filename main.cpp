#include "LeaderDB.h"
#include "CsvParser.h"
#include "FileUtils.h"
#include "DataInserter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <filesystem>

using namespace std;

void uploadSavedPlaces(LeaderDB& db, const string& userId);

void printHelp() {
    cout << "\nAvailable Commands:\n"
         << "----------------------------------------------\n"
         << "1.   get <key>                   - Retrieve one record (all attributes)\n"
         << "2.   create                      - Create a new record (or create table)\n"
         << "3.   update                      - Update an existing record\n"
         << "4.   delete <key>                - Delete a record by key\n"
         << "5.   prefix <str>                - List all records whose key starts with <str>\n"
         << "6.   range                       - List all records whose key falls within a range\n"
         << "7.   createindex <col>           - Build a secondary index on column number <col>\n"
         << "8.   find <col> <value>          - Search records by indexed attribute\n"
         << "9.   select <cols>|* where <col>=<val> - Query with projection and filtering\n"
         << "10.  load <filepath>             - Load data from CSV into current table\n"
         << "11.  createuser                  - Create a new user (and upload Saved Places)\n"
         << "12.  recover                     - Recover the database from Write-Ahead Log (WAL)\n"
         << "13.  tables                      - List all tables\n"
         << "14.  current                     - Show current selected table\n"
         << "15.  use <table>                 - Switch to another table\n"
         << "16.  drop <table>                - Delete a table\n"
         << "17.  help                        - Show this help menu\n"
         << "18.  exit                        - Exit the program\n"
         << "19.  save                        - Save all tables to CSV files\n"
         << "----------------------------------------------\n";
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
    srand(0);
    
    const string path = "./dataset_project/data.csv";
    //const string path = "./output/listplaces.csv";
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open " << path << '\n';
        return 1;
    }

    // 读取表头
    string headerLine;
    getline(file, headerLine);
    stringstream ss(headerLine);
    vector<string> headers;
    string header;
    while(getline(ss, header, ',')) {
        headers.push_back(header);
    }
    
    if(!headers.empty()) {
        db.setTableHeaders("default", headers);
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

    // create tables for google takeout
    vector<string> requiredTables = { "users", "places", "savedlists", "listplaces" };
    for (const auto& table : requiredTables) {
        if (!db.hasTable(table)) {
            db.createTable(table);
            cout << "✅ Created missing table: " << table << endl;
        }
    }


    
    cout << "Welcome to LeaderDB Command Line Interface!\n";
    printHelp();

    string command;
    while (true) {
        cout << "[" << db.getCurrentTable() << "] ";
        if (!(cin >> command)) break;
        
        if (command == "exit") break;
        else if (command == "help") printHelp();
        else if (command == "recover") {
            db.recoverFromWAL();
            cout << "Database recovered from WAL\n";
        }

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

        /* ---------- CREATE ---------- */
        else if (command == "create") {
            string type; cin >> type;
            if (type == "table") {
                string tableName; cin >> tableName;
                if (tableName == "default") {
                    cout << "Cannot create table named 'default'.\n";
                } else if (db.hasTable(tableName)) {
                    cout << "Table " << tableName << " already exists.\n";
                } else {
                    db.createTable(tableName);
                    cout << "Table " << tableName << " created.\n";
                }
            } else {
                string key;
                cout << "Key: ";
                cin >> key;
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
            
            ifstream file(filepath);
            if (!file.is_open()) {
                cout << "Failed to open file.\n";
                continue;
            }

            // 读取表头
            string headerLine;
            getline(file, headerLine);
            stringstream ss(headerLine);
            vector<string> headers;
            string header;
            while(getline(ss, header, ',')) {
                headers.push_back(header);
            }
            
            if(headers.empty()) {
                cout << "No headers found in file.\n";
                continue;
            }

            db.setTableHeaders(db.getCurrentTable(), headers);
            
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
            cout << "Data loaded successfully into table: " << db.getCurrentTable() << "\n";
        }
        /* ---------- RANGE ---------- */
         else if (command == "range") {
            string low, high;
            cout << "Enter start range(inclusive): ";
            cin >> low;
            cout << "Enter end range(inclusive): ";
            cin >> high;

            auto results = db.getRange(low, high);
            if (results.empty()) {
                cout << "No matches found.\n";
            } else {
                cout << "Results in range [" << low << ", " << high << "]:\n";
                for (const auto& row : results) {
                    for (size_t i = 0; i < row.size(); ++i) {
                        if (i > 0) cout << ", ";
                        cout << row[i];
                    }
                    cout << '\n';
                }
                cout << "Total: " << results.size() << '\n';
            }
        }
        /* build secondary index */
        else if (command == "createindex") {
            string columnName;
            cout << "Enter column name: ";
            cin >> columnName;
            
            int col = db.getColumnIndex(db.getCurrentTable(), columnName);
            if(col == -1) {
                cout << "Column " << columnName << " not found.\n";
                continue;
            }
            
            
            col = col - 1;  // -1 to skip key column
            if(col < 0) {
                cout << "Cannot create index on key column.\n";
                continue;
            }
            
            db.createSecondaryIdx(col);
            cout << "Secondary index built on column " << columnName << ".\n";
        }
        /* indexed find */
        else if (command == "find") {
            string columnName, val;
            cout << "Enter column name: ";
            cin >> columnName;
            cout << "Enter value to find: ";
            cin >> val;

            int col = db.getColumnIndex(db.getCurrentTable(), columnName);
            if(col == -1) {
                cout << "Column " << columnName << " not found.\n";
                continue;
            }

            
            col = col - 1;  // -1 to skip key column
            if(col < 0) {
                cout << "Cannot search on key column.\n";
                continue;
            }

            auto rows = db.findByAttr(col, val);
            if (rows.empty()) {
                cout << "No match.\n";
            } else {
                // 获取表头用于显示列名
                auto headers = db.getTableHeaders(db.getCurrentTable());
                for (auto& row : rows) {
                    for (size_t i = 0; i < row.size(); i++){
                        if(i) cout << ", ";
                        if (i < headers.size()) {
                            cout << headers[i] << ": ";
                        }
                        cout << row[i];
                    }
                    cout << '\n';
                }
                cout << "Total: " << rows.size() << " records\n";
            }
        }
        else if (command == "select") {
            cin.ignore();          
            getline(cin, line);    
            stringstream ss(line);
        
            string colPart;
            ss >> colPart;                             
            vector<int> proj;
            if (colPart != "*") {
                stringstream cs(colPart);
                string columnName;
                while (getline(cs, columnName, ',')) {
                    int colIdx = db.getColumnIndex(db.getCurrentTable(), columnName);
                    if(colIdx != -1) {
                        colIdx = colIdx - 1;  
                        if(colIdx >= 0) {  
                            proj.push_back(colIdx);
                        }
                    } else {
                        cout << "Column " << columnName << " not found.\n";
                    }
                }
            }
        
            string whereKw;
            ss >> whereKw;
            if (whereKw != "where") {
                cout << "Syntax error: expected 'where'.\n"; continue;
            }
        
            string cond;
            getline(ss, cond);                        
            cond = cond.substr(cond.find_first_not_of(" \t"));
            
            auto pos = cond.find('=');
            if (pos == string::npos) {
                cout << "Syntax error: expected '='.\n"; continue;
            }

            string whereColName = cond.substr(0, pos);
            whereColName = whereColName.substr(0, whereColName.find_last_not_of(" \t") + 1);
            
            int whereCol = db.getColumnIndex(db.getCurrentTable(), whereColName);
            if(whereCol == -1) {
                cout << "Column " << whereColName << " not found.\n";
                continue;
            }
            whereCol = whereCol - 1; 
            if(whereCol < 0) {
                cout << "Cannot search on key column.\n";
                continue;
            }

            string whereVal = cond.substr(pos + 1);
            whereVal = whereVal.substr(whereVal.find_first_not_of(" \t"));
            
            auto rows = db.selectWhere(proj, whereCol, whereVal);
            if (rows.empty()) { cout << "No match.\n"; continue; }
        
            for (auto& r : rows) {
                for (size_t i=0;i<r.size();++i){
                    if(i) cout << ", ";
                    cout << r[i];
                }
                cout << '\n';
            }
            cout << "Total: " << rows.size() << '\n';
        } /* ---------- CREATEUSER ---------- */
        else if (command == "createuser"){
            if(!db.hasTable("users")){
                db.createTable("users");
            }
            db.switchTable("users");

            vector<string> usersHeaders = {"id", "email", "userId", "registeredAt"};
            db.setTableHeaders("users", usersHeaders);

            string email;
            cout << "Enter user email: ";
            cin >> email;

            bool exists = false;
            auto rows = db.getCurrentIndex().raw();
            // check if email exists
            rows.forEachLeaf([&](const Entry<int, vector<string>>& e) {
                if (!e.attrs.empty() && e.attrs[0] == email) {
                    exists = true;
                }
            });

            if (exists) {
                cout << "Email already registered.\n";
                db.switchTable("default");
                continue;
            }

            int userId;
            while (true) {
                userId = rand();
                // Check if random userId is already taken
                if (db.get(to_string(userId)).empty()) break;
            }

            time_t timestamp;
            struct tm* ti;
            time(&timestamp);
            ti = localtime(&timestamp);
            string timeStr = asctime(ti);
            if (!timeStr.empty() && timeStr.back() == '\n'){
                timeStr.pop_back();
            }
            
            // insert
            vector<string> userRec = { email, to_string(userId), timeStr};
            try{
                db.create(to_string(userId), userRec);
                cout << "User created successfullly. \n";
            }catch(const exception& e){
                cout << "Error: " << e.what() << '\n';
            }
            
            cout << "Would you like to upload your Saved Places? (yes/no): ";
            string ans;
            cin >> ans;
            if (ans == "yes") {
                uploadSavedPlaces(db, to_string(userId));
                db.switchTable("default");
            }

        }
        else if (command == "join") {
            // example: join A.1 B.2 0,2 *    
            string tcolA, tcolB; cin >> tcolA >> tcolB;
            string projA = "*", projB = "*";
            if (cin.peek()==' ') cin >> projA;
            if (cin.peek()==' ') cin >> projB;
        
            auto parseTC = [](const string& s){      // "A.1"
                auto dot=s.find('.');
                return pair<string,int>{ s.substr(0,dot),
                                         stoi(s.substr(dot+1)) };
            };
            auto [tA,cA]=parseTC(tcolA);
            auto [tB,cB]=parseTC(tcolB);
        
            auto toVec = [](const string& s){
                vector<int> v;
                if (s=="*") return v;
                stringstream ss(s); string tok;
                while(getline(ss,tok,',')) v.push_back(stoi(tok));
                return v;
            };
            auto res = db.join(tA,cA,tB,cB,toVec(projA),toVec(projB));
            if(res.empty()) cout<<"No match.\n";
            else{
                for(auto& r:res){
                    for(size_t i=0;i<r.size();++i){
                        if(i) cout<<", ";
                        cout<<r[i];
                    } cout<<'\n';
                }
                cout<<"Total: "<<res.size()<<'\n';
            }
        }
        else if (command == "save") {
            
            string outputDir = "./output";
            filesystem::create_directory(outputDir);
            
            
            auto tables = db.listTables();
            for (const auto& table : tables) {
                if (db.exportTableToCsv(table, outputDir)) {
                    cout << "✅ Saved " << table << " to " << outputDir << "/" 
                         << table << ".csv\n";
                } else {
                    cout << "❌ Failed to save " << table << "\n";
                }
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

void uploadSavedPlaces(LeaderDB& db, const string& userId) {
    cout << "Enter folder path with Saved Places csv file: ";
    string folderPath;
    cin >> folderPath;

    vector<string> csvFiles;
    try {
        csvFiles = listCsvFiles(folderPath);
    } catch (const filesystem::filesystem_error& e) {
        cout << "Filesystem error: " << e.what() << '\n';
        return;
    }

    if (csvFiles.empty()){
        cout << "No files in folder.\n";
        return;
    }

    for(const auto& filePath: csvFiles) {
        auto places = parseCsvFile(filePath);
        if(places.empty()) {
            cout << "Warning: " << filePath << "is empty or failed to parse.\n";
            continue;
        }
        string listTitle = fileNameWithoutExtension(filePath);
        string listId = generateRandomListId();
        
        
        db.switchTable("savedlists");
        vector<string> savedListHeaders = {"listId", "userId", "title", "createdAt"};
        db.setTableHeaders("savedlists", savedListHeaders);
        insertSavedList(db, userId, listId, listTitle);

        for (const auto& place : places) {
            
            db.switchTable("places"); 
            vector<string> placesHeaders = {"hashedId", "placeId", "name", "address", "latitude", "longitude", "description"};
            db.setTableHeaders("places", placesHeaders);
            try {
                if (!placeExists(db, place.placeId)) {
                    insertPlace(db, place);
                } else {
                    cout << "Place already exists in database. Skipping insert.\n";
                }
            } catch (const exception& e) {
                cout << "Exception during place insertion: " << e.what() << endl;
            }

            
            db.switchTable("listplaces");
            vector<string> listPlacesHeaders = {"combinedKey", "listId", "placeId"};
            db.setTableHeaders("listplaces", listPlacesHeaders);
            try {
                insertListPlace(db, listId, place.placeId);
            } catch (const exception& e) {
                cout << "Exception during list-place insertion: " << e.what() << endl;
            }
        }
        cout << "Uploaded list: " << listTitle << "( " << places.size() << " places)\n";
    }
}