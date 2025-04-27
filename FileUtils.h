#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
#include <vector>

using namespace std;

vector<string> listCsvFiles(const string& folderPath);
string fileNameWithoutExtension(const string& filePath);

#endif