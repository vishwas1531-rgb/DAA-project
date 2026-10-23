#include "data_manager.h"
#include "algorithms.h"
#include <iostream>
#include <string>
#include <sstream>

// Lightweight JSON builder helper
static std::string escapeJSON(const std::string& s) {
    std::string res = "";
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else res += c;
    }
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "{\"error\": \"Invalid arguments. Usage: engine <data_dir> <start_id> <end_id> [priority]\"}\n";
        return 1;
    }

    std::string dataDir = argv[1];
    int start_id = std::stoi(argv[2]);
    int end_id = std::stoi(argv[3]);
    std::string priority = (argc >= 5) ? argv[4] : "balanced";

    DataManager dm;
    if (!dm.loadAllData(dataDir)) {
        std::cout << "{\"error\": \"Failed to load CSV datasets from path: " << escapeJSON(dataDir) << "\"}\n";
        return 1;
    }

    // Binary search validation for stop existence
    std::vector<int> sortedStops = dm.getSortedStopIDs();
    TransportEngine engine(dm);

    if (!engine.binarySearchStopExists(sortedStops, start_id) || 
        !engine.binarySearchStopExists(sortedStops, end_id)) {
        std::cout << "{\"error\": \"Origin or Destination Stop ID does not exist in network.\"}\n";
        return 0;
    }

    // DFS Reachability Check
    std::unordered_set<int> visited;
    bool reachable = engine.isReachableDFS(start_id, end_id, visited);
    if (!reachable) {
        std::cout << "{\"error\": \"Unreachable destination. No continuous bus route connects these stops.\"}\n";
        return 0;
    }

    // Compute Journey Options
    std::vector<JourneyOption> journeys = engine.findAlternativeRoutes(start_id, end_id);
    journeys = engine.rankJourneysGreedy(journeys, priority);

    if (journeys.empty()) {
        std::cout << "{\"error\": \"No feasible routes found between selected stops.\"}\n";
        return 0;
    }

    // Sort crowd reports using MergeSort demo
    engine.mergeSortReports(dm.crowd_reports, 0, static_cast<int>(dm.crowd_reports.size()) - 1);

    // Build JSON Response
    std::stringstream json;
    json << "{";
    json << "\"status\": \"success\",";
    json << "\"origin_id\": " << start_id << ",";
    json << "\"origin_name\": \"" << escapeJSON(dm.stops[start_id].stop_name) << "\",";
    json << "\"destination_id\": " << end_id << ",";
    json << "\"destination_name\": \"" << escapeJSON(dm.stops[end_id].stop_name) << "\",";
    json << "\"priority\": \"" << escapeJSON(priority) << "\",";
    
    // Recommended Best Journey
    const auto& best = journeys[0];
    json << "\"recommended\": {";
    json << "\"travel_time\": " << best.total_time_minutes << ",";
    json << "\"transfers\": " << best.total_transfers << ",";
    json << "\"crowd_level\": " << best.max_crowd_level << ",";
    json << "\"score\": " << best.composite_score << ",";
    json << "\"reason\": \"" << escapeJSON(best.recommendation_reason) << "\",";
    json << "\"path\": [";
    for (size_t i = 0; i < best.path_stops.size(); i++) {
        int sid = best.path_stops[i];
        json << "{\"stop_id\": " << sid << ", \"stop_name\": \"" << escapeJSON(dm.stops[sid].stop_name) << "\"}";
        if (i + 1 < best.path_stops.size()) json << ",";
    }
    json << "]";
    json << "},";

    // Alternatives
    json << "\"alternatives\": [";
    for (size_t k = 1; k < journeys.size(); k++) {
        const auto& alt = journeys[k];
        json << "{";
        json << "\"travel_time\": " << alt.total_time_minutes << ",";
        json << "\"transfers\": " << alt.total_transfers << ",";
        json << "\"crowd_level\": " << alt.max_crowd_level << ",";
        json << "\"reason\": \"" << escapeJSON(alt.recommendation_reason) << "\"";
        json << "}";
        if (k + 1 < journeys.size()) json << ",";
    }
    json << "]";

    json << "}";

    std::cout << json.str() << std::endl;
    return 0;
}