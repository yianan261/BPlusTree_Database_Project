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
#include <iomanip>

using namespace std;

void uploadSavedPlaces(LeaderDB& db, const string& userId);
void viewTable(LeaderDB& db);
void printASCII();
void printHelp();
static vector<string> splitCSV(const string& line);

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

    // get headers
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

    // create tables for google takeout
    vector<string> requiredTables = { "users", "places", "savedlists", "listplaces" };
    for (const auto& table : requiredTables) {
        if (!db.hasTable(table)) {
            db.createTable(table);
        }
        // populate headers
        if (table == "users") {
            db.setTableHeaders("users", {"id", "email", "userId", "registeredAt"});
        } else if (table == "places") {
            db.setTableHeaders("places",
                {"placeId", "name", "address", "latitude", "longitude", "description"});
        }
        else if (table == "savedlists") {
            db.setTableHeaders("savedlists",
                {"userId", "title", "createdAt"});
        }
        else if (table == "listplaces") {
            db.setTableHeaders("listplaces",
                {"listId", "placeId"});
        }
    }



    printASCII();
    cout << "Welcome to Places Database Command Line Interface!\n";
    printHelp();

    string command;
    while (true) {
        cout << "[" << db.getCurrentTable() << "] ";
        if (!(cin >> command)) break;
        try{
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
                cout << "Create table or instance?";
                string type; cin >> type;
                if (type == "table") {
                    cout << "Enter table name: ";
                    string tableName; cin >> tableName;
                    if (tableName == "default") {
                        cout << "Cannot create table named 'default'.\n";
                    } else if (db.hasTable(tableName)) {
                        cout << "Table " << tableName << " already exists.\n";
                    } else {
                        cout << "Enter headers (comma separated): ";
                        cin.ignore();
                        string headerLine;
                        getline(cin, headerLine);
                        vector<string> headers = splitCSV(headerLine);
                        
                        if (headers.empty()) {
                            cout << "Headers cannot be empty.\n";
                        } else {
                            db.createTable(tableName);
                            db.setTableHeaders(tableName, headers);
                            cout << "Table " << tableName << " created with headers: ";
                            for (size_t i = 0; i < headers.size(); ++i) {
                                if (i > 0) cout << ", ";
                                cout << headers[i];
                            }
                            cout << "\n";
                        }
                    }
                } else if (type == "instance") {
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
                } else {
                    cout << "Command unavailable.\n";
                    continue;
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
                if (db.getCurrentTable() == "users"){
                    // 'deleteuser' currently unsupported
                    cout << "Delete user currently unsupported. \n";
                    continue;
                }
                string key; cin >> key;
                db.deleteKey(key);
                cout << "Record deleted.\n";
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

                vector<string> existingHeaders = db.getTableHeaders(db.getCurrentTable());
                bool hasExistingHeaders = !existingHeaders.empty();
                size_t expectedColumns = hasExistingHeaders ? existingHeaders.size() - 1 : 0; // -1 因为不包括主键列

                string headerLine;
                getline(file, headerLine);
                
                if (!hasExistingHeaders) {
                    stringstream ss(headerLine);
                    vector<string> headers;
                    string header;
                    while(getline(ss, header, ',')) {
                        headers.push_back(header);
                    }
                    
                    if(headers.empty()) {
                        cout << "No headers found in file.\n";
                        file.close();
                        continue;
                    }

                    db.setTableHeaders(db.getCurrentTable(), headers);
                    expectedColumns = headers.size() - 1;  // 设置期望的列数
                }
                
                bool hasError = false;
                string line;
                int lineNum = 1;  // 用于错误提示
                while (getline(file, line)) {
                    lineNum++;
                    auto fields = splitCSV(line);
                    if (fields.empty()) continue;

                    try {
                        if (fields.size() - 1 != expectedColumns) {  // -1 because first field is key
                            throw runtime_error("Line " + to_string(lineNum) + 
                                             ": Column count mismatch (expected " + 
                                             to_string(expectedColumns) + 
                                             " columns, got " + 
                                             to_string(fields.size() - 1) + ")");
                        }

                        string key = fields.front();
                        vector<string> attrs(fields.begin() + 1, fields.end());
                        db.create(key, attrs);
                    } catch(const exception& e) {
                        cout << "Error: " << e.what() << '\n';
                        hasError = true;
                    }
                }
                file.close();
                
                if (!hasError) {
                    cout << "Data loaded successfully into table: " << db.getCurrentTable() << "\n";
                } else {
                    cout << "Failed to load some records into table: " << db.getCurrentTable() << "\n";
                }
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
            /* ---------- SELECT ---------- */
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
            /* ---------- JOIN ---------- */
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
            /* ---------- SAVE ---------- */
            else if (command == "save") {
                
                string outputDir = "./output";
                filesystem::create_directory(outputDir);
                
                
                auto tables = db.listTables();
                for (const auto& table : tables) {
                    if (db.exportTableToCsv(table, outputDir)) {
                        cout << "Saved " << table << " to " << outputDir << "/" 
                            << table << ".csv\n";
                    } else {
                        cout << "Failed to save " << table << "\n";
                    }
                }
            }
            /* ---------- VIEW ---------- */
            else if (command == "view"){
                viewTable(db);
            }
            /* ---------- UNKNOWN ---------- */
            else {
                cout << "Unknown command. Type 'help' for help.\n";
            }
        }catch (const exception& e) {
            cout << "Error: " << e.what() << '\n';
        } catch (...) {
            cout << "Unknown Error occurred.\n";
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
                } // only insert if not in DB already
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

void viewTable(LeaderDB& db){
    cout << "View Table (truncated): \n";
    const size_t MAX_WIDTH = 30;
    auto& tree = db.getCurrentIndex().raw();

    if(tree.size() == 0){
        cout << "(Table is empty)\n";
        return;
    }
    vector<vector<string>> rows;
    size_t numCols = 0;

    // get 10 rows only, use lambda
    tree.forEachLeaf([&](const auto& entry){
        if (rows.size() >= 10) return;
        vector<string> row = entry.attrs;
        row.insert(row.begin(), to_string(entry.key));
        rows.push_back(row);
        numCols = max(numCols, row.size());
    });
    //populate table headers
    vector<string> headers = db.getTableHeaders(db.getCurrentTable());

    /*
    if (headers.size() + 1 == numCols) {
        // if header doesn't have PK column, add
        headers.insert(headers.begin(), "PrimaryKey");
    }
    else if (headers.size() == numCols) {
    }
    else {
        headers.clear();
        headers.push_back("PrimaryKey");
        for (size_t i = 1; i < numCols; ++i)
            headers.push_back("col" + to_string(i - 1));
    }
    */


    vector<size_t> colWidths(numCols, 0);
    for(size_t i=0; i < numCols; ++i){
        colWidths[i] = min(headers[i].length(), MAX_WIDTH);
        for (const auto& row: rows){
            if (i < row.size()){
                colWidths[i] = max(colWidths[i], min(row[i].length(), MAX_WIDTH));
            }
        }
    }
    // headers
    cout << "| ";
    for (size_t i = 0; i < numCols; ++i) {
        string header = headers[i];
        if (header.length() > MAX_WIDTH) {
            header = header.substr(0, MAX_WIDTH - 3) + "...";
        }
        cout << setw(colWidths[i]) << left << header << " | ";
    }
    cout << "\n| ";
    for (size_t i = 0; i < numCols; ++i) {
        cout << string(colWidths[i], '-') << " | ";
    }
    cout << "\n";
    //rows
    for (const auto& row : rows) {
        cout << "| ";
        for (size_t i = 0; i < numCols; ++i) {
            string val = (i < row.size()) ? row[i] : "";
            if (val.length() > MAX_WIDTH) {
                val = val.substr(0, MAX_WIDTH - 3) + "...";
            }
            cout << setw(colWidths[i]) << left << val << " | ";
        }
        cout << "\n";
    }
   
}

void printASCII(){
        cout << R"(

    ________  ___       ________  ________  _______   ________                          
    |\   __  \|\  \     |\   __  \|\   ____\|\  ___ \ |\   ____\                         
    \ \  \|\  \ \  \    \ \  \|\  \ \  \___|\ \   __/|\ \  \___|_                        
    \ \   ____\ \  \    \ \   __  \ \  \    \ \  \_|/_\ \_____  \                       
    \ \  \___|\ \  \____\ \  \ \  \ \  \____\ \  \_|\ \|____|\  \                      
    \ \__\    \ \_______\ \__\ \__\ \_______\ \_______\____\_\  \                     
        \|__|     \|_______|\|__|\|__|\|_______|\|_______|\_________\                    
                                                        \|_________|                    
                                                                                        
                                                                                        
    ________  ________  _________  ________  ________  ________  ________  _______      
    |\   ___ \|\   __  \|\___   ___\\   __  \|\   __  \|\   __  \|\   ____\|\  ___ \     
    \ \  \_|\ \ \  \|\  \|___ \  \_\ \  \|\  \ \  \|\ /\ \  \|\  \ \  \___|\ \   __/|    
    \ \  \ \\ \ \   __  \   \ \  \ \ \   __  \ \   __  \ \   __  \ \_____  \ \  \_|/__  
    \ \  \_\\ \ \  \ \  \   \ \  \ \ \  \ \  \ \  \|\  \ \  \ \  \|____|\  \ \  \_|\ \ 
    \ \_______\ \__\ \__\   \ \__\ \ \__\ \__\ \_______\ \__\ \__\____\_\  \ \_______\
        \|_______|\|__|\|__|    \|__|  \|__|\|__|\|_______|\|__|\|__|\_________\|_______|
                                                                    \|_________|         
                                                                                        
                                                                                        
        )" << endl;
    
}
/** 
 * TODO: 
 * create: let users define headers first -> Zirui （check）
 * removed prefix and find -> yian (check)
 * add try catch, don't exit program when there are errors -> both (check)
 * save (CSV各式有問題), -> both (check)
 * update (when update key, segment fault), -> yian (when users update 2nd time doesn't work) (check)
 * explain createindex (unclear to users what it does) -> zirui
 * change load cout "data loaded successfully" when it's unsuccessful. -> zirui (check)
 * update viewtable to also show PK -> yian (check)
 * check recover (WAL),add try catch don't exit program, error libc++abi: terminating due to uncaught exception of type std::runtime_error: Key already exists. Use update instead. ->yian
 * update help menu -> zirui
 * added deleteuser -> yian (check)
 * */ 
void printHelp() {
    cout << "\nAvailable Commands:\n"
         << "----------------------------------------------\n"
         << "1.   get <key>                   - Retrieve one record (all attributes)\n"
         << "2.   create                      - Create a new record (or create table)\n"
         << "3.   update                      - Update an existing record\n"
         << "4.   delete <key>                - Delete a record by key\n"
         << "5.   drop <table>                - Delete a table\n"
         << "6.   use <table>                 - Switch to another table\n"
         << "7.   tables                      - List all tables\n"
         << "8.   current                     - Show current selected table\n"
         << "9.   load <filepath>             - Load data from CSV into current table\n"
         << "10.  save                        - Save all tables to CSV files\n"
         << "11.  view                        - View 10 records of a table\n"
         << "12.  createindex <col>           - Build a secondary index on column for faster select\n"
         << "13.  select <cols>|* where <col>=<val> - Query with projection and filtering\n"
         << "14.  recover                     - Recover from Write-Ahead Log (WAL)\n"
         << "15.  createuser                  - Create a new user (and upload Saved Places)\n"
         << "16.  join                        - Join two tables\n"
         << "17.  help                        - Show this help menu\n"
         << "18.  exit                        - Exit the program\n"
         << "----------------------------------------------\n";
}


// split csv 
static vector<string> splitCSV(const string& line) {
    vector<string> fields;
    string field;
    stringstream ss(line);
    while (getline(ss, field, ',')) fields.push_back(field);
    return fields;
}