#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <climits>
using namespace std;

struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int priority;       // not used by MLFQ
    int remaining_time;
    int waiting_time;
    int turnaround_time;
    int queue_level;    // 0=Q0, 1=Q1, 2=Q2
    int last_enq_time;  // for aging measurement
};

void calculateMetrics(vector<Process>& processes, int total_time){
    double aw=0, at=0; for(auto&p:processes){ aw+=p.waiting_time; at+=p.turnaround_time; }
    aw/=processes.size(); at/=processes.size();
    cout<<"Avg Waiting Time: "<<aw<<"\n";
    cout<<"Avg Turnaround Time: "<<at<<"\n";
    cout<<"CPU Utilization: 100%\n";
}
void printGantt(const vector<pair<string,int>>& g){ cout<<"Gantt Chart: "; for(auto&e:g) cout<<e.first<<"("<<e.second<<") "; cout<<"\n"; }
vector<Process> loadDefaultTable(){
    return {
        {"P1",0,8,2,8,0,0,0,0},
        {"P2",1,4,1,4,0,0,0,0},
        {"P3",2,9,3,9,0,0,0,0},
        {"P4",3,5,4,5,0,0,0,0},
    };
}
void reset(vector<Process>& ps){ for(auto& p:ps){ p.remaining_time=p.burst_time; p.waiting_time=p.turnaround_time=0; p.queue_level=0; p.last_enq_time=p.arrival_time; } }

int main(){
    const int Q0_Q = 3, Q1_Q = 6;              // RR quanta
    const int AGE_THRESHOLD = 12;               // promote if waited this long

    vector<Process> ps = loadDefaultTable(); reset(ps);
    sort(ps.begin(), ps.end(), [](auto&a, auto&b){ if(a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time; return a.id<b.id; });

    int n=ps.size(), i=0, t=0, done=0, last=-1;
    queue<int> Q0, Q1, Q2;
    vector<pair<string,int>> gantt;

    auto enqueue_arrivals = [&](int upto){
        while(i<n && ps[i].arrival_time<=upto){
            int idx=i++; ps[idx].queue_level=0; ps[idx].last_enq_time=t;
            Q0.push(idx);
        }
    };

    auto promote_if_aging = [&](){
        // promote from Q2->Q1 or Q1->Q0 if waited too long
        auto check_promote = [&](queue<int>& Qfrom, int fromLvl, queue<int>& Qto, int toLvl){
            int sz = Qfrom.size();
            while(sz--){
                int idx = Qfrom.front(); Qfrom.pop();
                if(t - ps[idx].last_enq_time >= AGE_THRESHOLD && ps[idx].queue_level==fromLvl){
                    ps[idx].queue_level = toLvl;
                    ps[idx].last_enq_time = t;
                    Qto.push(idx);
                } else {
                    Qfrom.push(idx);
                }
            }
        };
        // check deeper first so promotions bubble upward correctly in one pass
        check_promote(Q2, 2, Q1, 1);
        check_promote(Q1, 1, Q0, 0);
    };

    if(ps[0].arrival_time>0) t=ps[0].arrival_time;
    enqueue_arrivals(t);

    while(done<n){
        promote_if_aging();

        int idx = -1;
        int qlvl = -1;
        int quantum = 0;

        if(!Q0.empty()){ idx=Q0.front(); Q0.pop(); qlvl=0; quantum=Q0_Q; }
        else if(!Q1.empty()){ idx=Q1.front(); Q1.pop(); qlvl=1; quantum=Q1_Q; }
        else if(!Q2.empty()){ idx=Q2.front(); Q2.pop(); qlvl=2; quantum=INT_MAX; } // FCFS: run to completion
        else {
            // idle until next arrival
            if(i<n){ t=max(t, ps[i].arrival_time); enqueue_arrivals(t); continue; }
        }

        if(idx!=-1){
            if(idx!=last && last!=-1) gantt.push_back({ps[last].id, t});
            last = idx;

            int ran = 0;
            while(ps[idx].remaining_time>0 && ran<quantum){
                ps[idx].remaining_time--; t++; ran++;
                enqueue_arrivals(t);
                promote_if_aging();
            }

            if(ps[idx].remaining_time==0){
                ps[idx].turnaround_time = t - ps[idx].arrival_time;
                ps[idx].waiting_time    = ps[idx].turnaround_time - ps[idx].burst_time;
                gantt.push_back({ps[idx].id, t}); last=-1; done++;
            } else {
                // time slice expired → demote (unless already at bottom)
                ps[idx].queue_level = min(2, qlvl+1);
                ps[idx].last_enq_time = t;
                if(ps[idx].queue_level==0) Q0.push(idx);
                else if(ps[idx].queue_level==1) Q1.push(idx);
                else Q2.push(idx);
            }
        }
    }

    printGantt(gantt);
    calculateMetrics(ps, t);
    return 0;
}
