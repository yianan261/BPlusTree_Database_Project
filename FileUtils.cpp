#include "FileUtils.h"
#include <filesystem>

namespace fs = filesystem;

vector<string> listCsvFiles(const string& folderPath){
    vector<string> csvFiles;
    for(const auto& entry : fs::directory_iterator(folderPath)){
        if(entry.is_regular_file() && entry.path().extension() == ".csv"){
            csvFiles.push_back(entry.path().string());
        }
    }
    return csvFiles;
}

string fileNameWithoutExtension(const string& filePath){
    return fs::path(filePath).stem().string();
}