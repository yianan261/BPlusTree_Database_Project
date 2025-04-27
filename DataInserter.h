#ifndef DATAINSERTER_H
#define DATAINSERTER_H
#include <string>
#include "CsvParser.h"
using namespace std;
class LeaderDB;

string generateRandomListId();
bool placeExists(LeaderDB& db, const string& placeId);
// insert place if not exists
void insertPlace(LeaderDB& db, const Place& place);
void insertSavedList(LeaderDB& db, const string& userId, const string& listId, const string& title);
//Insert relation into ListPlaces
void insertListPlace(LeaderDB& db, const string& listId, const string& placeId);

#endif