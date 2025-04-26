#include "DataInserter.h"
#include "LeaderDB.h"
#include <cstdlib>
#include <ctime>
#include <sstream>

using namespace std;

const int MAX_NUM =  1000000;

string generateRandomListId(){
    srand(time(0));
    int id = rand() % MAX_NUM;
    return to_string(id);
}

/**
 * @brief  Check if a place already exists in the "places" table
 * 
 * @param db LeaderDB address
 * @param placeId string address of placeId
 * @return true 
 * @return false 
 */
bool placeExists(LeaderDB& db, const string& placeId){
    db.switchTable("places");
    return db.getCurrentIndex().contains(stoi(placeId));
}

/**
 * @brief Insert a Place into the "places" table if it doesn't exist
 * 
 * @param db LeaderDB address
 * @param place Place object
 */
void insertPlace(LeaderDB& db, const Place& place){
    db.switchTable("places");
    if (placeExists(db, place.placeId)) return;

    vector<string> attrs = {
        place.name,
        place.address,
        place.latitude,
        place.longitude
    };

    db.create(place.placeId, attrs);
}

/**
 * @brief Insert a new SavedList into "savedlists" table
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

/**
 * @brief Insert mapping of SavedList and Place into "listplaces" table
 * 
 * @param db address of LeaderDB
 * @param listId address
 * @param placeId address
 */
void insertListPlace(LeaderDB& db, const string& listId, const string& placeId){
    db.switchTable("listplaces");
    // use listId_placeId for unique Id
    string combinedKey = listId + "_" + placeId;
    vector<string> attrs = {
        listId,
        placeId
    };
    db.create(combinedKey, attrs);
}
