#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "data_manager.h"
#include <vector>
#include <string>

struct JourneyOption {
    std::vector<int> path_stops;
    std::vector<std::string> routes_used;
    int total_time_minutes;
    int total_transfers;
    int max_crowd_level;
    double composite_score;
    std::string recommendation_reason;
};

class TransportEngine {
private:
    DataManager dm;

public:
    TransportEngine(const DataManager& data_mgr);

    // 1. BFS for minimum edges
    JourneyOption computeBFSMinTransfers(int start_id, int end_id);

    // 2. DFS for reachability check
    bool isReachableDFS(int start_id, int end_id, std::unordered_set<int>& visited);

    // 3. Dijkstra for fastest path
    JourneyOption computeDijkstraFastest(int start_id, int end_id);

    // 4. Alternative route finder
    std::vector<JourneyOption> findAlternativeRoutes(int start_id, int end_id);

    // 5. Merge Sort algorithm implementation
    static void mergeSortReports(std::vector<CrowdReport>& reports, int left, int right);

    // 6. Linear Search
    int linearSearchReportByID(int report_id);

    // 7. Binary Search
    bool binarySearchStopExists(const std::vector<int>& sorted_ids, int target_id);

    // 8. Hash-based Duplicate Detection
    bool checkDuplicateReports(const std::vector<CrowdReport>& reports);

    // 9. Greedy Journey Ranker
    std::vector<JourneyOption> rankJourneysGreedy(std::vector<JourneyOption>& journeys, const std::string& priority);

    // 10. Dynamic Programming - Capacity Allocation / Crowd Penalty Knapsack
    int computeDPCrowdCapacity(int max_capacity, const std::vector<int>& item_weights, const std::vector<int>& item_values);
};

#endif // ALGORITHMS_H