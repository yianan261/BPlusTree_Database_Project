#ifndef CSVPARSER_H
#define CSVPARSER_H
#include <string>
#include <vector>

struct Place {
    std::string placeId;
    std::string name;
    std::string address;
    std::string latitude;
    std::string longitude;
};

std::vector<Place> parseCsvFile(const std::string& filepath);

#endif