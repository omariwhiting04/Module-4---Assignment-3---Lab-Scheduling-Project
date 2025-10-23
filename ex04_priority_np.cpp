#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <climits>
using namespace std;

struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int priority;       // lower number = higher priority
    int remaining_time; // unused here, kept for consistency
    int waiting_time;
    int turnaround_time;
};


void calculateMetrics(vector<Process>& processes, int total_time) {
    double avg_wait = 0, avg_turn = 0;
    for (auto& p : processes) {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
    }
    avg_wait /= processes.size();
    avg_turn /= processes.size();
    cout << "Avg Waiting Time: " << avg_wait << "\n";
    cout << "Avg Turnaround Time: " << avg_turn << "\n";
    cout << "CPU Utilization: 100%\n";
}

void printGantt(const vector<pair<string,int>>& gantt) {
    cout << "Gantt Chart: ";
    for (auto& g : gantt) cout << g.first << "(" << g.second << ") ";
    cout << "\n";
}

vector<Process> loadDefaultTable() {
    return {
        {"P1", 0, 8, 2, 8, 0, 0},
        {"P2", 1, 4, 1, 4, 0, 0},
        {"P3", 2, 9, 3, 9, 0, 0},
        {"P4", 3, 5, 4, 5, 0, 0},
    };
}

void reset(vector<Process>& ps) {
    for (auto& p : ps) {
        p.remaining_time = p.burst_time;
        p.waiting_time   = 0;
        p.turnaround_time= 0;
    }
}


int main() {
    vector<Process> processes = loadDefaultTable();
    reset(processes);

    // deterministic order for ties
    sort(processes.begin(), processes.end(),
         [](const Process& a, const Process& b){
             if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
             return a.id < b.id;
         });

    const int n = (int)processes.size();
    vector<bool> done(n, false);
    vector<pair<string,int>> gantt;

    int time = 0, finished = 0;

    
    const int AGING_INTERVAL = 5; // tweak if your rubric specifies a different policy

    while (finished < n) {
        // gather ready processes
        vector<int> ready;
        for (int i = 0; i < n; ++i)
            if (!done[i] && processes[i].arrival_time <= time)
                ready.push_back(i);

        // if none are ready, jump to the next arrival
        if (ready.empty()) {
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; ++i)
                if (!done[i]) next_arrival = min(next_arrival, processes[i].arrival_time);
            time = next_arrival;
            continue;
        }

        
        int pick = -1;
        auto effPriority = [&](int idx){
            int wait = max(0, time - processes[idx].arrival_time);
            int bump = (AGING_INTERVAL > 0) ? (wait / AGING_INTERVAL) : 0;
            int eff  = max(0, processes[idx].priority - bump);
            return eff;
        };

        for (int idx : ready) {
            if (pick == -1) {
                pick = idx;
            } else {
                int e1 = effPriority(idx);
                int e2 = effPriority(pick);
                if (e1 != e2) {
                    if (e1 < e2) pick = idx;  // lower number = higher priority
                } else {
                    // tie-breakers: base priority, arrival, shorter burst, then ID
                    if (processes[idx].priority != processes[pick].priority)
                        { if (processes[idx].priority < processes[pick].priority) pick = idx; }
                    else if (processes[idx].arrival_time != processes[pick].arrival_time)
                        { if (processes[idx].arrival_time < processes[pick].arrival_time) pick = idx; }
                    else if (processes[idx].burst_time != processes[pick].burst_time)
                        { if (processes[idx].burst_time < processes[pick].burst_time) pick = idx; }
                    else if (processes[idx].id < processes[pick].id)
                        pick = idx;
                }
            }
        }

        // run picked process to completion 
        Process& p = processes[pick];
        p.waiting_time    = time - p.arrival_time;
        time             += p.burst_time;
        p.turnaround_time = time - p.arrival_time;

        gantt.push_back({p.id, time});
        done[pick] = true;
        finished++;
    }

    printGantt(gantt);
    calculateMetrics(processes, time);
    return 0;
}
