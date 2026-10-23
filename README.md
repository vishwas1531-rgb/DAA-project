# Public Transport Crowd Estimator (Pune PMPML)

A Data Structures & Algorithms (DAA) college project built in C++17 and Python Flask for intelligent bus route planning and crowd estimation.

## Algorithms Implemented & Time Complexities

1. **Dijkstra's Algorithm**: $O((V + E) \log V)$ — Finds minimum travel time paths.
2. **Breadth-First Search (BFS)**: $O(V + E)$ — Finds minimum edge/transfer paths.
3. **Depth-First Search (DFS)**: $O(V + E)$ — Validates network reachability.
4. **Merge Sort**: $O(N \log N)$ — Sorts crowd reports chronologically.
5. **Binary Search**: $O(\log V)$ — Quick stop ID lookup.
6. **Greedy Ranker**: $O(K \log K)$ — Multi-objective score optimization ($W_{\text{time}} + W_{\text{crowd}} + W_{\text{transfers}}$).
7. **0/1 Knapsack (DP)**: $O(N \cdot C)$ — Crowd/Passenger capacity allocation planning.

## Setup & Execution (Windows 11)

1. Ensure **MinGW / g++** (C++17) and **Python 3.x** are installed and added to your `PATH`.
2. Double click or run `run.bat` in Command Prompt / PowerShell.
3. Open `http://127.0.0.1:5000` in your web browser.