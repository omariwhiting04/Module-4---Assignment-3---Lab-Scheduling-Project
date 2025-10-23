#include <bits/stdc++.h>
using namespace std;


struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int priority;        // lower number = higher priority (when used)
    int remaining_time;  // for preemptive/RR
    int waiting_time = 0;
    int turnaround_time = 0;
    int deadline = -1;   // optional (e.g., EDF)
};

struct SimResult {
    vector<pair<string,int>> gantt; // (pid, cumulative_finish_or_switch_time)
    int total_time = 0;
};

static void printGantt(const vector<pair<string,int>>& gantt) {
    cout << "Gantt Chart: ";
    for (auto &e : gantt) cout << e.first << "(" << e.second << ") ";
    cout << "\n";
}

static void calcAndPrintMetrics(vector<Process>& ps, int total_time) {
    double avg_wait = 0.0, avg_turn = 0.0;
    int busy = 0;
    for (auto &p : ps) {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
        busy += p.burst_time;
    }
    avg_wait /= ps.size();
    avg_turn /= ps.size();
    double cpu_util = (total_time > 0) ? (100.0 * busy / total_time) : 0.0;
    double throughput = (total_time > 0) ? (double)ps.size() / total_time : 0.0;

    cout << "Avg Waiting Time: " << avg_wait << "\n";
    cout << "Avg Turnaround Time: " << avg_turn << "\n";
    cout << "CPU Utilization: " << cpu_util << "%\n";
    cout << "Throughput (jobs / time): " << throughput << "\n";
}

/* Default table (matches your doc) */
static vector<Process> defaultTable() {
    return {
        {"P1", 0, 8, 2, 8},
        {"P2", 1, 4, 1, 4},
        {"P3", 2, 9, 3, 9},
        {"P4", 3, 5, 4, 5},
    };
}

/* Optional CSV loader: id,arrival,burst,priority  (header optional) */
static vector<Process> loadCSV(const string& filename) {
    ifstream f(filename);
    if (!f) throw runtime_error("Failed to open input file: " + filename);
    vector<Process> ps;
    string line;
    bool first = true;
    while (getline(f, line)) {
        if (line.empty()) continue;
        // skip header rows containing non-digits in second column
        if (first) {
            first = false;
            // peek: if not a data row, continue (e.g., header)
            string tmp = line;
            replace(tmp.begin(), tmp.end(), ',', ' ');
            string id; string a,b,p;
            stringstream ss(tmp);
            if (!(ss >> id >> a >> b >> p)) continue;
            bool ad = all_of(a.begin(), a.end(), [](char c){ return c=='-' || isdigit((unsigned char)c); });
            if (!ad) continue; // header
        }
        string id; int a,b,p;
        char comma;
        stringstream ss(line);
        if (!(ss >> id)) continue;
        if (ss.peek()==',') ss >> comma;
        ss >> a;
        if (ss.peek()==',') ss >> comma;
        ss >> b;
        if (ss.peek()==',') ss >> comma;
        if (!(ss >> p)) p = 3; // default priority if missing
        ps.push_back({id, a, b, p, b});
    }
    if (ps.empty()) throw runtime_error("No processes parsed from " + filename);
    return ps;
}

/* Random generator  */
static vector<Process> generateRandom(int n, unsigned seed=42) {
    mt19937 rng(seed);
    uniform_int_distribution<int> A(0, 20), B(1, 12), P(1, 4);
    vector<Process> ps;
    for (int i=1;i<=n;i++) {
        int a=A(rng), b=B(rng), p=P(rng);
        ps.push_back({"P"+to_string(i), a, b, p, b});
    }
    sort(ps.begin(), ps.end(), [](auto&a, auto&b){
        if (a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time;
        return a.id<b.id;
    });
    return ps;
}


class Scheduler {
public:
    virtual ~Scheduler() = default;
    // Implementations must fill ps[*].waiting_time & turnaround_time
    virtual SimResult run(vector<Process> ps) = 0;
    virtual string name() const = 0;
};


class FCFSScheduler : public Scheduler {
public:
    string name() const override { return "FCFS"; }
    SimResult run(vector<Process> ps) override {
        sort(ps.begin(), ps.end(), [](auto&a, auto&b){
            if (a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time;
            return a.id<b.id;
        });
        SimResult R;
        int t=0;
        for (auto &p : ps) {
            if (t < p.arrival_time) t = p.arrival_time;
            p.waiting_time = t - p.arrival_time;
            t += p.burst_time;
            p.turnaround_time = t - p.arrival_time;
            R.gantt.push_back({p.id, t});
        }
        R.total_time = t;
        calcAndPrintMetrics(ps, R.total_time);
        printGantt(R.gantt);
        return R;
    }
};


class SJFScheduler : public Scheduler {
public:
    string name() const override { return "SJF"; }
    SimResult run(vector<Process> ps) override {
        sort(ps.begin(), ps.end(), [](auto&a, auto&b){
            if (a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time;
            return a.id<b.id;
        });
        const int n=ps.size();
        vector<bool> done(n,false);
        SimResult R;
        int fin=0, t=0;
        while (fin<n) {
            // find ready
            vector<int> ready;
            for (int i=0;i<n;i++) if (!done[i] && ps[i].arrival_time<=t) ready.push_back(i);
            if (ready.empty()) {
                int nxt=INT_MAX; for (int i=0;i<n;i++) if(!done[i]) nxt=min(nxt, ps[i].arrival_time);
                t=nxt; continue;
            }
            // pick shortest burst
            int pick = *min_element(ready.begin(), ready.end(), [&](int i,int j){
                if (ps[i].burst_time!=ps[j].burst_time) return ps[i].burst_time<ps[j].burst_time;
                if (ps[i].arrival_time!=ps[j].arrival_time) return ps[i].arrival_time<ps[j].arrival_time;
                return ps[i].id<ps[j].id;
            });
            auto &p = ps[pick];
            p.waiting_time = t - p.arrival_time;
            t += p.burst_time;
            p.turnaround_time = t - p.arrival_time;
            R.gantt.push_back({p.id, t});
            done[pick]=true; fin++;
        }
        R.total_time=t;
        calcAndPrintMetrics(ps, R.total_time);
        printGantt(R.gantt);
        return R;
    }
};

/* ---------- Round Robin (preemptive, quantum) ---------- */
class RRScheduler : public Scheduler {
    int quantum;
public:
    explicit RRScheduler(int q): quantum(q>0?q:4) {}
    string name() const override { return "RR(q="+to_string(quantum)+")"; }

    SimResult run(vector<Process> ps) override {
        // init remaining
        for (auto &p: ps) p.remaining_time = p.burst_time;
        sort(ps.begin(), ps.end(), [](auto&a, auto&b){
            if (a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time;
            return a.id<b.id;
        });

        SimResult R;
        queue<int> q; int n=ps.size(), i=0, t=0, done=0; int last=-1;

        auto enq_up_to = [&](int upto){
            while (i<n && ps[i].arrival_time<=upto) { q.push(i); i++; }
        };

        if (ps[0].arrival_time>0) t = ps[0].arrival_time;
        enq_up_to(t);

        while (done<n) {
            if (q.empty()) {
                if (i<n) { t=max(t, ps[i].arrival_time); enq_up_to(t); }
                continue;
            }
            int idx=q.front(); q.pop();
            if (idx!=last && last!=-1) R.gantt.push_back({ps[last].id, t});
            last=idx;

            int ran=0;
            while (ran<quantum && ps[idx].remaining_time>0) {
                ps[idx].remaining_time--; ran++; t++;
                enq_up_to(t);
            }
            if (ps[idx].remaining_time==0) {
                ps[idx].turnaround_time = t - ps[idx].arrival_time;
                ps[idx].waiting_time    = ps[idx].turnaround_time - ps[idx].burst_time;
                R.gantt.push_back({ps[idx].id, t}); last=-1; done++;
            } else {
                q.push(idx);
            }
        }
        R.total_time = t;
        calcAndPrintMetrics(ps, R.total_time);
        printGantt(R.gantt);
        return R;
    }
};


static unique_ptr<Scheduler> makeScheduler(const string& kind, int quantum) {
    string k = kind;
    // normalize
    for (auto &c : k) c = tolower((unsigned char)c);

    if (k=="rr" || k=="roundrobin")   return make_unique<RRScheduler>(quantum);
    if (k=="fcfs")                    return make_unique<FCFSScheduler>();
    if (k=="sjf")                     return make_unique<SJFScheduler>();

    throw runtime_error("Unknown scheduler: " + kind +
        " (supported: fcfs, sjf, rr)");
}


static void usage(const char* prog) {
    cerr << "Usage:\n"
         << "  " << prog << " [--input tasks.csv | --random N] "
         << " --scheduler {fcfs|sjf|rr} [--quantum Q]\n\n"
         << "If no input is provided, uses the lab's default 4-process table.\n";
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string inputFile;
    int randomN = -1;
    string schedulerKind = "rr";
    int quantum = 4;

    // parse args
    for (int i=1; i<argc; ++i) {
        string a = argv[i];
        if (a=="--input" && i+1<argc)       { inputFile = argv[++i]; }
        else if (a=="--random" && i+1<argc) { randomN = stoi(argv[++i]); }
        else if (a=="--scheduler" && i+1<argc) { schedulerKind = argv[++i]; }
        else if (a=="--quantum" && i+1<argc) { quantum = stoi(argv[++i]); }
        else if (a=="-h" || a=="--help")    { usage(argv[0]); return 0; }
        else { cerr << "Unknown/invalid arg: " << a << "\n"; usage(argv[0]); return 1; }
    }

    vector<Process> processes;
    try {
        if (!inputFile.empty()) {
            processes = loadCSV(inputFile);
        } else if (randomN > 0) {
            processes = generateRandom(randomN);
        } else {
            processes = defaultTable();
        }
    } catch (const exception& e) {
        cerr << e.what() << "\n"; return 1;
    }

    // Ensure remaining_time is set
    for (auto &p : processes) p.remaining_time = p.burst_time;

    try {
        auto sched = makeScheduler(schedulerKind, quantum);
        cout << "Scheduler: " << sched->name() << "\n";
        // (Run returns SimResult but we already print inside; kept for later logging)
        SimResult res = sched->run(processes);
        (void)res;
    } catch (const exception& e) {
        cerr << e.what() << "\n"; return 1;
    }

    return 0;
}
