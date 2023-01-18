#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <thread>
#include <mutex>
#include <cassert>
#include <chrono>

using namespace std;

const int MAXT = 10;
int T = 0;

inline void readPoly(const string& filename, int &n, vector <int> &a, vector<int> &b)
{
    ifstream fin(filename);
    fin >> n;
    for(int i = 0; i < n; ++ i) {
        int x;
        fin >> x;
        a.push_back(x);
    }
    for(int i = 0; i < n; ++ i) {
        int x;
        fin >> x;
        b.push_back(x);
    }
}

void showPoly(vector<int> a, vector<int> b)
{
    for(auto it:a)
        cout << it << " ";
    cout << "\n";
    for(auto it:b)
        cout << it << " ";
    cout << "\n";
}


inline void extend_size(vector <int> &a, size_t n) {
    while(n & (n - 1))
        ++n;
    a.resize(n);
}


inline void mult(const vector <int> &a, const vector <int> &b, vector <int> &sol)
{
    if(a.size() == 1 && b.size() == 1)
    {
        sol[0] = a[0] * b[0];
        return;
    }
    
    unsigned long half = a.size() / 2 + a.size() % 2;
    // first part
    vector <int> a_lo(a.begin(), a.begin() + half);
    vector <int> a_hi(a.begin() + half, a.end());
    vector <int> b_lo(b.begin(), b.begin() + half);
    vector <int> b_hi(b.begin() + half, b.end());
    vector <int> lo(a_lo.size() + b_lo.size() - 1);
    vector <int> hi(a_hi.size() + b_hi.size() - 1);
    mult(a_lo, b_lo, lo);
    mult(a_hi, b_hi, hi);
    // copy first part
    for(int i = 0; i < lo.size(); ++i)
    {
        sol[i] += lo[i];
    }
    // middle
    for(int i = 0; i < a_hi.size(); ++i)
    {
        a_lo[i] += a_hi[i];
        b_lo[i] += b_hi[i];
    }
    vector <int> mid(a_lo.size() + b_lo.size() - 1);
    mult(a_lo, b_lo, mid);
    for(int i = 0; i < mid.size(); ++ i)
    {
        sol[i + half] += mid[i] - lo[i] - hi[i];
    }
    for(int i = 0; i < hi.size(); ++ i)
    {
        sol[i + 2 * half] += hi[i];
    }
}


inline void solveKaratsubaWithoutThreads(int n, vector<int> a, vector<int> b)
{
    auto start = std::chrono::high_resolution_clock::now();

    extend_size(a, a.size());
    extend_size(b, b.size());
    vector <int> sol(2 * n - 1);
    mult(a, b, sol);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "Karatsuba algorithm on took me " << elapsed.count() << " seconds\n";
    
//    for(auto it: sol)
//        cout<<it<<" ";
//    cout<<"\n";
}

void multWithThreads(const vector <int> &a, const vector <int> &b, vector <int> &sol) {
    if(a.size() == 1 && b.size() == 1)
    {
        sol[0] = a[0] * b[0];
        return;
    }
    unsigned long half = a.size() / 2 + a.size() % 2;
    
    // first part
    vector <int> a_lo(a.begin(), a.begin() + half);
    vector <int> a_hi(a.begin() + half, a.end());
    vector <int> b_lo(b.begin(), b.begin() + half);
    vector <int> b_hi(b.begin() + half, b.end());
    vector <int> lo(a_lo.size() + b_lo.size() - 1);
    vector <int> hi(a_hi.size() + b_hi.size() - 1);
    
    vector <thread> th;
    T += 2;
    if (T < MAXT) {
      th.push_back(thread([&a_lo, &b_lo, &lo](){multWithThreads(a_lo, b_lo, lo);}));
      th.push_back(thread([&a_hi, &b_hi, &hi](){multWithThreads(a_hi, b_hi, hi);}));
    } else {
      multWithThreads(a_lo, b_lo, lo);
      multWithThreads(a_hi, b_hi, hi);
    }
    
    // middle
    vector <int> a_lo2(a_lo);
    vector <int> b_lo2(b_lo);
    for(int i = 0; i < a_hi.size(); ++ i) {
      a_lo2[i] += a_hi[i];
      b_lo2[i] += b_hi[i];
    }
    vector <int> mid(a_lo2.size() + b_lo2.size() - 1);
    
    T += 1;
    if (T < MAXT) {
      th.push_back(thread([&a_lo2, &b_lo2, &mid](){multWithThreads(a_lo2, b_lo2, mid);}));
    } else {
      multWithThreads(a_lo2, b_lo2, mid);
    }
    
    // wait for threads to finish
    T -= th.size();
    for(int i = 0; i < th.size(); ++ i) {
      th[i].join();
    }
    // copy first part
    for(int i = 0; i < lo.size(); ++ i) {
      sol[i] += lo[i];
    }
    for(int i = 0; i < mid.size(); ++ i) {
      sol[i + half] += mid[i] - lo[i] - hi[i];
    }
    for(int i = 0; i < hi.size(); ++ i) {
      sol[i + 2 * half] += hi[i];
    }
    
}


inline void solveKaratasubaWithThreads(int n, vector<int> a, vector<int> b)
{
    auto start = std::chrono::high_resolution_clock::now();
  
    extend_size(a, a.size());
    extend_size(b, b.size());
    vector <int> sol(2 * n - 1);
    multWithThreads(a, b, sol);
    
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "Karatsuba algorithm (threaded) took me " << elapsed.count()<< " seconds\n";
//    for(auto it: sol)
//        cout<<it<<" ";
//    cout<<"\n";
}



inline void solveNaiveWithoutThreads(int n, vector<int> a, vector<int> b) {
    auto start = std::chrono::high_resolution_clock::now();
    
    vector <int> sol(2 * n - 1, 0);
    for(int i = 0; i < n; ++ i)
        for(int j = 0; j < n; ++ j)
            sol[i + j] += a[i] * b[j];
    
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "Naive algorithm took me " << elapsed.count() << " seconds\n";
//    for(auto it: sol)
//        cout<<it<<" ";
//    cout<<"\n";
}


int n;
vector <int> a, b, sol;
vector <thread> threads;
mutex mx;

inline void work(int idx) {
    mx.lock();
    for(int i = idx; i < n; i += T)
    
        for(int j = 0; j < n; j++)
            sol[i + j] += a[i] * b[j];
    
    mx.unlock();
}

inline void solveNaiveWithThreads(int n, vector<int> a, vector<int> b)
{
    auto start = std::chrono::high_resolution_clock::now();
  
    sol.resize(2 * n - 1, 0);
    for(int i = 0; i < min(n,T); ++ i) {
        threads.push_back(thread(work, i));
    }
    for(int i = 0; i < min(n,T); ++ i) {
        threads[i].join();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "Naive algorithm (thread) took me " << elapsed.count() << " seconds\n";
//    for(auto it: sol)
//        cout<<it<<" ";
//    cout<<"\n";
}


int main(int argc, const char * argv[])
{
    int n;
    vector <int> a, b;
    string path = "tests10000.in";
    readPoly(path,n, a, b);
    solveKaratsubaWithoutThreads(n, a, b);
    solveKaratasubaWithThreads(n, a, b);
    solveNaiveWithoutThreads(n, a, b);
    solveNaiveWithThreads(n, a, b);
//    showPoly(a, b);
    return 0;
}
