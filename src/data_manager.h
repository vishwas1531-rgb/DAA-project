#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct BusStop {
    int stop_id;
    std::string stop_name;
    double latitude;
    double longitude;
};

struct BusRoute {
    std::string route_id;
    std::string route_name;
    int origin_stop_id;
    int destination_stop_id;
};

struct Connection {
    std::string connection_id;
    int from_stop_id;
    int to_stop_id;
    std::string route_id;
    int travel_time_minutes;
    double distance_km;
};

struct CrowdReport {
    int report_id;
    std::string route_id;
    int stop_id;
    int crowd_level; // 1 (Low) to 5 (Critical)
    int passenger_count;
    std::string report_time;
    std::string source;
    double confidence;
};

class DataManager {
public:
    std::unordered_map<int, BusStop> stops;
    std::unordered_map<std::string, BusRoute> routes;
    std::vector<Connection> connections;
    std::vector<CrowdReport> crowd_reports;

    bool loadStops(const std::string& filepath);
    bool loadRoutes(const std::string& filepath);
    bool loadConnections(const std::string& filepath);
    bool loadCrowdReports(const std::string& filepath);

    bool loadAllData(const std::string& dataDir);
    
    bool isValidStop(int stop_id) const;
    std::vector<int> getSortedStopIDs() const;
    int getAverageCrowdLevelForStop(int stop_id) const;
    int getAverageCrowdLevelForRoute(const std::string& route_id) const;
};

#endif // DATA_MANAGER_H