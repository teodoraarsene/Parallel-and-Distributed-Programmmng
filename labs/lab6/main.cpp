#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <thread>
#include <mutex>

using namespace std;

const int maxn = 35;
const int NR_THREADS = 2;
int n, m;
vector <int> g[maxn];

inline void readGraph(const string& filename)
{
    ifstream fin(filename);
    fin >> n >> m;
    for(int i = 0; i < m; ++ i)
    {
        int x, y;
        fin >> x >> y;
        g[x].push_back(y);
//        cout << x << " " << y << endl;
    }
}


inline bool isEdge(int x, int y)
{
    for(auto it : g[x])
    {
        if(it == y) {
            return true;
        }
    }
    return false;
}

bool isSafe(int n, vector <int> &ex){
    for(auto it: ex){
        if(it == n)
            return false;
    }
    return true;
}

int T = 0;
mutex mtx;

inline bool doIt(int node, vector <int> &sol)
{
    sol.push_back(node);
    if(sol.size() == n)
    {
        return isEdge(sol.back(), sol[0]);
    }
    
    vector <int> a(sol), b(sol); // create two copies
    bool sol1 = false;
    mtx.lock();
    if(T < NR_THREADS) {
        T += 2;
        mtx.unlock();
        thread t1([&](){
        for(int i = 0; i < g[node].size(); i += 2) {
            vector <int> aux(a);
            if(!isSafe(g[node][i],aux))
                continue;
            if(doIt(g[node][i], aux)) {
                sol1 = true;
                a = aux;
                return;
            }
        }
        });
    bool sol2 = false;
    thread t2([&]() {
        for(int i = 1; i < g[node].size(); i += 2) {
        vector <int> bux(b);
            if(!isSafe(g[node][i],bux))
                continue;
        if(doIt(g[node][i], bux)) {
            sol2 = true;
            b = bux;
            return ;
        }
      }
    });
    t1.join();
    t2.join();
    mtx.lock();
    T -= 2;
    mtx.unlock();
    if(sol1) {
        sol = a;
        return true;
    } else if(sol2) {
        sol = b;
        return true;
    }
    } else { // do it on current thread
      mtx.unlock();
      for(int i = 0; i < g[node].size(); i ++) {
          vector <int> bux(b);
          if(!isSafe(g[node][i],bux))
              continue;
          if(doIt(g[node][i], bux)) {
              sol = bux;
              return true;
        }
      }
    }
    return false;
}

int main()
{
    readGraph("2.in");
    cout << "Finding a hamiltonian cycle of " << n << " nodes\n";
    auto startTime0 = std::chrono::high_resolution_clock::now();
    vector <int> sol;
    bool ok = doIt(1, sol);
//    cout << n << '\n';
    cout << (ok ? "Found" : "Not found") << '\n';
    if(ok) {
        for(auto it : sol) {
            cout << it << ' ';
      }
      cout << '\n';
    }
    auto finishTime0 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> timeElapsed0 = finishTime0 - startTime0;
    cout << "Finding the hamiltonian cycle of " << n << " nodes took me " << timeElapsed0.count() << " seconds\n";
    return 0;
}
