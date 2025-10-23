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
    int priority;       // unused here
    int remaining_time;
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
    cout << "CPU Utilization: 100%\n";
}

void printGantt(const vector<pair<string,int>>& gantt) {
    cout << "Gantt Chart: ";
    for (auto& g : gantt)
        cout << g.first << "(" << g.second << ") ";
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

// ---------- SRTF (preemptive) ----------
int main() {
    vector<Process> processes = loadDefaultTable();
    reset(processes);

    // Sort by arrival for deterministic ordering
    sort(processes.begin(), processes.end(),
         [](const Process& a, const Process& b) {
             if (a.arrival_time != b.arrival_time)
                 return a.arrival_time < b.arrival_time;
             return a.id < b.id;
         });

    const int n = (int)processes.size();
    vector<int> finish(n, -1);
    vector<pair<string,int>> gantt; // (pid, cumulative_time_at_switch_or_finish)
    int time = 0, completed = 0, lastPick = -1;

    while (completed < n) {
        int pick = -1;

        // Find ready processes
        for (int i = 0; i < n; ++i) {
            if (processes[i].arrival_time <= time && processes[i].remaining_time > 0) {
                if (pick == -1 ||
                    processes[i].remaining_time < processes[pick].remaining_time ||
                    (processes[i].remaining_time == processes[pick].remaining_time &&
                     processes[i].arrival_time < processes[pick].arrival_time)) {
                    pick = i;
                }
            }
        }

        // If none ready, jump to next arrival
        if (pick == -1) {
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; ++i)
                if (processes[i].remaining_time > 0)
                    next_arrival = min(next_arrival, processes[i].arrival_time);
            time = next_arrival;
            lastPick = -1;
            continue;
        }

        // Context switch (record last slice)
        if (pick != lastPick && lastPick != -1)
            gantt.push_back({processes[lastPick].id, time});
        lastPick = pick;

        // Run selected process for 1 time unit
        processes[pick].remaining_time--;
        time++;

        // If finished, record completion time
        if (processes[pick].remaining_time == 0) {
            finish[pick] = time;
            completed++;
        }
    }

    // Record last running process end time
    if (lastPick != -1)
        gantt.push_back({processes[lastPick].id, time});

    // Compute turnaround and waiting times
    for (int i = 0; i < n; ++i) {
        processes[i].turnaround_time = finish[i] - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
    }

    printGantt(gantt);
    calculateMetrics(processes, time);
    return 0;
}
