#include "DataInserter.h"
#include "LeaderDB.h"
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <functional> 

using namespace std;

const int MAX_NUM =  1000000;

string generateRandomListId(){
    int id = rand() % MAX_NUM;
    return to_string(id);
}

// hash placeID from string to int to save to DB
int hashPlaceId(const string& placeId) {
    hash<string> hasher;
    return static_cast<int>(hasher(placeId) & 0x7FFFFFFF); 
}

/**
 * @brief  Check if a place already exists in the "places" table
 * 
 * @param db LeaderDB address
 * @param placeId string address of placeId
 * @return true 
 * @return false 
 */
bool placeExists(LeaderDB& db, const string& placeId) {
    db.switchTable("places");
    int hashedId = hashPlaceId(placeId);
    return !db.get(to_string(hashedId)).empty();
}

/**
 * @brief Insert a Place into the "places" table if it doesn't exist
 * 
 * @param db LeaderDB address
 * @param place Place object
 */
void insertPlace(LeaderDB& db, const Place& p) {
    db.switchTable("places");

    int hashedId = hashPlaceId(p.placeId); 

    vector<string> attrs = {
        p.placeId,
        p.name,
        p.address,
        p.latitude,
        p.longitude,
        p.description
    };

    db.create(to_string(hashedId), attrs);
}

/**
 * @brief Insert a new SavedList into "savedLists" table
 * 
 * @param db address of LeaderDB
 * @param userId address of userId (string)
 * @param listId address of listId (string)
 * @param title address of title (string)
 */
void insertSavedList(LeaderDB& db, const string& userId, const string& listId, const string& title){
    db.switchTable("savedLists");
    time_t now = time(0);
    string createdAt = ctime(&now);
    if (! createdAt.empty() && createdAt.back() == '\n') createdAt.pop_back();

    vector<string> attrs = {
        userId, 
        title, 
        createdAt
    };
    db.create(listId, attrs);
}

// hash combined key to save as primary key to DB.
int generateCombinedKeyHash(const string& listId, const string& placeId) {
    string combinedStr = listId + "_" + placeId;
    hash<string> hasher;
    return static_cast<int>(hasher(combinedStr) & 0x7FFFFFFF);
}

/**
 * @brief Insert mapping of SavedList and Place into "listPlaces" table
 * 
 * @param db address of LeaderDB
 * @param listId address
 * @param placeId address
 */
void insertListPlace(LeaderDB& db, const string& listId, const string& placeId){
    db.switchTable("listPlaces");
    int combinedKey = generateCombinedKeyHash(listId, placeId);
    vector<string> attrs = {
        listId,
        placeId
    };
    db.create(to_string(combinedKey), attrs);
}
