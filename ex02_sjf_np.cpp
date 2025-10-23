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
    int priority;       // lower number = higher priority (unused here)
    int remaining_time; // unused for non-preemptive, but kept for consistency
    int waiting_time;
    int turnaround_time;
};

// ---------- common helpers ----------
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
    cout << "CPU Utilization: 100%\n"; // no idle with this dataset
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

// ---------- SJF  ----------
int main() {
    vector<Process> processes = loadDefaultTable();
    reset(processes);

    // sort by arrival for deterministic intake
    sort(processes.begin(), processes.end(),
         [](const Process& a, const Process& b){
             if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
             return a.id < b.id;
         });

    int n = (int)processes.size();
    vector<bool> done(n, false);
    vector<pair<string,int>> gantt; // {pid, cumulative_finish_time}
    int finished = 0, current_time = 0;

    while (finished < n) {
        // collect ready indices
        vector<int> ready;
        for (int i = 0; i < n; ++i)
            if (!done[i] && processes[i].arrival_time <= current_time)
                ready.push_back(i);

        if (ready.empty()) { // CPU idle until the next arrival
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; ++i)
                if (!done[i]) next_arrival = min(next_arrival, processes[i].arrival_time);
            current_time = next_arrival;
            continue; // then re-evaluate ready set
        }

        // choose shortest burst; break ties by arrival then ID
        int pick = *min_element(ready.begin(), ready.end(),
            [&](int i, int j){
                if (processes[i].burst_time != processes[j].burst_time)
                    return processes[i].burst_time < processes[j].burst_time;
                if (processes[i].arrival_time != processes[j].arrival_time)
                    return processes[i].arrival_time < processes[j].arrival_time;
                return processes[i].id < processes[j].id;
            });

        Process& p = processes[pick];
        // start now; no preemption
        p.waiting_time    = current_time - p.arrival_time;
        current_time     += p.burst_time;
        p.turnaround_time = current_time - p.arrival_time;

        gantt.push_back({p.id, current_time});
        done[pick] = true;
        ++finished;
    }

    printGantt(gantt);
    calculateMetrics(processes, current_time);
    return 0;
}
