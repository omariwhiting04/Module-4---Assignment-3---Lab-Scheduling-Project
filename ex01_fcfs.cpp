#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
using namespace std;

struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int priority;       // lower number = higher priority
    int remaining_time;
    int waiting_time;
    int turnaround_time;
};

// ---------- helpers ----------
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
    cout << "CPU Utilization: 100%\n";  // simplified for now
}

void printGantt(const vector<pair<string,int>>& gantt) {
    cout << "Gantt Chart: ";
    for (auto& g : gantt)
        cout << g.first << "(" << g.second << ") ";
    cout << "\n";
}

vector<Process> loadDefaultTable() {
    vector<Process> ps = {
        {"P1", 0, 8, 2, 8, 0, 0},
        {"P2", 1, 4, 1, 4, 0, 0},
        {"P3", 2, 9, 3, 9, 0, 0},
        {"P4", 3, 5, 4, 5, 0, 0}
    };
    return ps;
}

void reset(vector<Process>& ps) {
    for (auto& p : ps) {
        p.remaining_time = p.burst_time;
        p.waiting_time = 0;
        p.turnaround_time = 0;
    }
}


int main() {
    vector<Process> processes = loadDefaultTable();
    reset(processes);

    sort(processes.begin(), processes.end(),
         [](const Process &a, const Process &b) {
             return a.arrival_time < b.arrival_time;
         });

    vector<pair<string,int>> gantt;
    int current_time = 0;

    for (auto &p : processes) {
        if (current_time < p.arrival_time)
            current_time = p.arrival_time;

        p.waiting_time = current_time - p.arrival_time;
        current_time += p.burst_time;
        p.turnaround_time = current_time - p.arrival_time;

        gantt.push_back({p.id, current_time});
    }

    printGantt(gantt);
    calculateMetrics(processes, current_time);
    return 0;
}
