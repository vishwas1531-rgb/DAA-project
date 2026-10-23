#include "algorithms.h"
#include <queue>
#include <stack>
#include <limits>
#include <algorithm>
#include <cmath>

TransportEngine::TransportEngine(const DataManager& data_mgr) : dm(data_mgr) {}

// 1. BFS to find minimum transfer (minimum edges) path
JourneyOption TransportEngine::computeBFSMinTransfers(int start_id, int end_id) {
    JourneyOption result;
    result.total_time_minutes = 0;
    result.total_transfers = 0;
    result.max_crowd_level = 1;
    result.composite_score = 0.0;
    result.recommendation_reason = "BFS Minimum Stops Path";

    if (!dm.isValidStop(start_id) || !dm.isValidStop(end_id)) return result;

    std::queue<int> q;
    std::unordered_map<int, int> parent;
    std::unordered_map<int, Connection> edge_used;
    std::unordered_set<int> visited;

    q.push(start_id);
    visited.insert(start_id);

    bool found = false;
    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        if (curr == end_id) {
            found = true;
            break;
        }

        for (const auto& conn : dm.connections) {
            if (conn.from_stop_id == curr && visited.find(conn.to_stop_id) == visited.end()) {
                visited.insert(conn.to_stop_id);
                parent[conn.to_stop_id] = curr;
                edge_used[conn.to_stop_id] = conn;
                q.push(conn.to_stop_id);
            }
        }
    }

    if (!found) return result;

    // Backtrack path
    std::vector<int> rev_path;
    std::vector<std::string> rev_routes;
    int curr = end_id;
    rev_path.push_back(curr);

    int total_time = 0;
    int max_crowd = 1;
    std::string last_route = "";
    int transfers = 0;

    while (curr != start_id) {
        Connection conn = edge_used[curr];
        total_time += conn.travel_time_minutes;
        int stop_crowd = dm.getAverageCrowdLevelForStop(curr);
        if (stop_crowd > max_crowd) max_crowd = stop_crowd;

        if (!last_route.empty() && last_route != conn.route_id) {
            transfers++;
        }
        last_route = conn.route_id;
        rev_routes.push_back(conn.route_id);

        curr = parent[curr];
        rev_path.push_back(curr);
    }

    std::reverse(rev_path.begin(), rev_path.end());
    std::reverse(rev_routes.begin(), rev_routes.end());

    result.path_stops = rev_path;
    result.routes_used = rev_routes;
    result.total_time_minutes = total_time;
    result.total_transfers = transfers;
    result.max_crowd_level = max_crowd;
    return result;
}

// 2. DFS for graph reachability
bool TransportEngine::isReachableDFS(int start_id, int end_id, std::unordered_set<int>& visited) {
    if (start_id == end_id) return true;
    visited.insert(start_id);

    for (const auto& conn : dm.connections) {
        if (conn.from_stop_id == start_id && visited.find(conn.to_stop_id) == visited.end()) {
            if (isReachableDFS(conn.to_stop_id, end_id, visited)) return true;
        }
    }
    return false;
}

// 3. Dijkstra's Algorithm for Minimum Travel Time
JourneyOption TransportEngine::computeDijkstraFastest(int start_id, int end_id) {
    JourneyOption result;
    result.total_time_minutes = 0;
    result.total_transfers = 0;
    result.max_crowd_level = 1;
    result.composite_score = 0.0;
    result.recommendation_reason = "Dijkstra Fastest Journey";

    if (!dm.isValidStop(start_id) || !dm.isValidStop(end_id)) return result;

    using pii = std::pair<int, int>; // (dist, stop_id)
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;
    std::unordered_map<int, int> dist;
    std::unordered_map<int, int> parent;
    std::unordered_map<int, Connection> edge_used;

    for (const auto& pair : dm.stops) {
        dist[pair.first] = std::numeric_limits<int>::max();
    }

    dist[start_id] = 0;
    pq.push({0, start_id});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;
        if (u == end_id) break;

        for (const auto& conn : dm.connections) {
            if (conn.from_stop_id == u) {
                int v = conn.to_stop_id;
                int weight = conn.travel_time_minutes; // Non-negative weight
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    edge_used[v] = conn;
                    pq.push({dist[v], v});
                }
            }
        }
    }

    if (dist[end_id] == std::numeric_limits<int>::max()) return result;

    // Backtrack path
    std::vector<int> rev_path;
    std::vector<std::string> rev_routes;
    int curr = end_id;
    rev_path.push_back(curr);

    int max_crowd = 1;
    std::string last_route = "";
    int transfers = 0;

    while (curr != start_id) {
        Connection conn = edge_used[curr];
        int stop_crowd = dm.getAverageCrowdLevelForStop(curr);
        if (stop_crowd > max_crowd) max_crowd = stop_crowd;

        if (!last_route.empty() && last_route != conn.route_id) {
            transfers++;
        }
        last_route = conn.route_id;
        rev_routes.push_back(conn.route_id);

        curr = parent[curr];
        rev_path.push_back(curr);
    }

    std::reverse(rev_path.begin(), rev_path.end());
    std::reverse(rev_routes.begin(), rev_routes.end());

    result.path_stops = rev_path;
    result.routes_used = rev_routes;
    result.total_time_minutes = dist[end_id];
    result.total_transfers = transfers;
    result.max_crowd_level = max_crowd;
    return result;
}

// 4. Alternative Feasible Routes Generation
std::vector<JourneyOption> TransportEngine::findAlternativeRoutes(int start_id, int end_id) {
    std::vector<JourneyOption> alternatives;

    // Route 1: Dijkstra Fastest
    JourneyOption d1 = computeDijkstraFastest(start_id, end_id);
    if (!d1.path_stops.empty()) {
        d1.recommendation_reason = "Fastest Route (Dijkstra)";
        alternatives.push_back(d1);
    }

    // Route 2: BFS Minimum Transfers
    JourneyOption b1 = computeBFSMinTransfers(start_id, end_id);
    if (!b1.path_stops.empty()) {
        b1.recommendation_reason = "Minimum Edge/Transfer Route (BFS)";
        // Avoid adding duplicate path
        if (alternatives.empty() || alternatives[0].path_stops != b1.path_stops) {
            alternatives.push_back(b1);
        }
    }

    return alternatives;
}

// 5. Merge Sort Implementation for Crowd Reports
static void merge(std::vector<CrowdReport>& reports, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    std::vector<CrowdReport> L(n1), R(n2);

    for (int i = 0; i < n1; i++) L[i] = reports[left + i];
    for (int j = 0; j < n2; j++) R[j] = reports[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].report_id <= R[j].report_id) {
            reports[k] = L[i];
            i++;
        } else {
            reports[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < n1) { reports[k] = L[i]; i++; k++; }
    while (j < n2) { reports[k] = R[j]; j++; k++; }
}

void TransportEngine::mergeSortReports(std::vector<CrowdReport>& reports, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortReports(reports, left, mid);
        mergeSortReports(reports, mid + 1, right);
        merge(reports, left, mid, right);
    }
}

// 6. Linear Search
int TransportEngine::linearSearchReportByID(int report_id) {
    for (size_t i = 0; i < dm.crowd_reports.size(); i++) {
        if (dm.crowd_reports[i].report_id == report_id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// 7. Binary Search on Sorted Stop IDs
bool TransportEngine::binarySearchStopExists(const std::vector<int>& sorted_ids, int target_id) {
    int low = 0, high = static_cast<int>(sorted_ids.size()) - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (sorted_ids[mid] == target_id) return true;
        if (sorted_ids[mid] < target_id) low = mid + 1;
        else high = mid - 1;
    }
    return false;
}

// 8. Hash-Based Duplicate Detection
bool TransportEngine::checkDuplicateReports(const std::vector<CrowdReport>& reports) {
    std::unordered_set<std::string> seen;
    for (const auto& r : reports) {
        std::string key = r.route_id + "_" + std::to_string(r.stop_id) + "_" + r.report_time;
        if (seen.find(key) != seen.end()) {
            return true; // Duplicate detected
        }
        seen.insert(key);
    }
    return false;
}

// 9. Greedy Strategy for Journey Ranking
std::vector<JourneyOption> TransportEngine::rankJourneysGreedy(std::vector<JourneyOption>& journeys, const std::string& priority) {
    for (auto& j : journeys) {
        // Scoring Formula:
        // Score = (TravelTime * W_time) + (CrowdLevel * W_crowd) + (Transfers * W_transfer)
        double w_time = 1.0, w_crowd = 10.0, w_transfer = 5.0;

        if (priority == "fastest") {
            w_time = 3.0; w_crowd = 2.0; w_transfer = 2.0;
        } else if (priority == "least_crowded") {
            w_time = 0.5; w_crowd = 20.0; w_transfer = 2.0;
        } else { // Balanced
            w_time = 1.0; w_crowd = 8.0; w_transfer = 6.0;
        }

        j.composite_score = (j.total_time_minutes * w_time) +
                            (j.max_crowd_level * w_crowd) +
                            (j.total_transfers * w_transfer);
    }

    std::sort(journeys.begin(), journeys.end(), [](const JourneyOption& a, const JourneyOption& b) {
        return a.composite_score < b.composite_score;
    });

    return journeys;
}

// 10. Dynamic Programming - Crowd Penalty 0/1 Knapsack optimization
int TransportEngine::computeDPCrowdCapacity(int max_capacity, const std::vector<int>& item_weights, const std::vector<int>& item_values) {
    int n = static_cast<int>(item_weights.size());
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(max_capacity + 1, 0));

    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= max_capacity; w++) {
            if (item_weights[i - 1] <= w) {
                dp[i][w] = std::max(dp[i - 1][w], dp[i - 1][w - item_weights[i - 1]] + item_values[i - 1]);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }
    return dp[n][max_capacity];
}