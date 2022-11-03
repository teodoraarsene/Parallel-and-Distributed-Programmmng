#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <list>
#include <functional>

class ThreadPool {
private:
	std::vector<std::thread> threads;
	std::mutex mutex;
	std::condition_variable condVariable;
	bool end;
	std::list<std::function<void()>> queue;

	void run();

public:
	explicit ThreadPool(int noThreads);
	void close();
	void enqueue(std::function<void()> threadFunction);
	~ThreadPool();
};

