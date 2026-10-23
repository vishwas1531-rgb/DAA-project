#include "data_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

static std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) {
        // Trim carriage return or whitespace
        while (!cell.empty() && (cell.back() == '\r' || cell.back() == '\n' || cell.back() == ' ')) cell.pop_back();
        size_t start = cell.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) cell = cell.substr(start);
        else if (cell.empty()) cell = "";
        result.push_back(cell);
    }
    return result;
}

bool DataManager::loadStops(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = parseCSVLine(line);
        if (tokens.size() < 4) continue;
        try {
            BusStop stop;
            stop.stop_id = std::stoi(tokens[0]);
            stop.stop_name = tokens[1];
            stop.latitude = std::stod(tokens[2]);
            stop.longitude = std::stod(tokens[3]);
            stops[stop.stop_id] = stop;
        } catch (...) { continue; }
    }
    return true;
}

bool DataManager::loadRoutes(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = parseCSVLine(line);
        if (tokens.size() < 4) continue;
        BusRoute r;
        r.route_id = tokens[0];
        r.route_name = tokens[1];
        try {
            r.origin_stop_id = std::stoi(tokens[2]);
            r.destination_stop_id = std::stoi(tokens[3]);
            routes[r.route_id] = r;
        } catch (...) { continue; }
    }
    return true;
}

bool DataManager::loadConnections(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = parseCSVLine(line);
        if (tokens.size() < 6) continue;
        try {
            Connection conn;
            conn.connection_id = tokens[0];
            conn.from_stop_id = std::stoi(tokens[1]);
            conn.to_stop_id = std::stoi(tokens[2]);
            conn.route_id = tokens[3];
            conn.travel_time_minutes = std::stoi(tokens[4]);
            conn.distance_km = std::stod(tokens[5]);
            
            // Validate foreign keys
            if (isValidStop(conn.from_stop_id) && isValidStop(conn.to_stop_id)) {
                connections.push_back(conn);
            }
        } catch (...) { continue; }
    }
    return true;
}

bool DataManager::loadCrowdReports(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = parseCSVLine(line);
        if (tokens.size() < 8) continue;
        try {
            CrowdReport rep;
            rep.report_id = std::stoi(tokens[0]);
            rep.route_id = tokens[1];
            rep.stop_id = std::stoi(tokens[2]);
            rep.crowd_level = std::stoi(tokens[3]);
            rep.passenger_count = std::stoi(tokens[4]);
            rep.report_time = tokens[5];
            rep.source = tokens[6];
            rep.confidence = std::stod(tokens[7]);
            
            if (rep.crowd_level >= 1 && rep.crowd_level <= 5) {
                crowd_reports.push_back(rep);
            }
        } catch (...) { continue; }
    }
    return true;
}

bool DataManager::loadAllData(const std::string& dataDir) {
    std::string sep = "/";
    if (!dataDir.empty() && dataDir.back() == '/') sep = "";
    bool s = loadStops(dataDir + sep + "stops.csv");
    bool r = loadRoutes(dataDir + sep + "routes.csv");
    bool c = loadConnections(dataDir + sep + "connections.csv");
    bool cr = loadCrowdReports(dataDir + sep + "crowd_reports.csv");
    return s && r && c && cr;
}

bool DataManager::isValidStop(int stop_id) const {
    return stops.find(stop_id) != stops.end();
}

std::vector<int> DataManager::getSortedStopIDs() const {
    std::vector<int> ids;
    for (const auto& pair : stops) {
        ids.push_back(pair.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int DataManager::getAverageCrowdLevelForStop(int stop_id) const {
    int total = 0, count = 0;
    for (const auto& rep : crowd_reports) {
        if (rep.stop_id == stop_id) {
            total += rep.crowd_level;
            count++;
        }
    }
    return count > 0 ? (total / count) : 2; // Default to 2 (Moderate) if no data
}

int DataManager::getAverageCrowdLevelForRoute(const std::string& route_id) const {
    int total = 0, count = 0;
    for (const auto& rep : crowd_reports) {
        if (rep.route_id == route_id) {
            total += rep.crowd_level;
            count++;
        }
    }
    return count > 0 ? (total / count) : 2;
}