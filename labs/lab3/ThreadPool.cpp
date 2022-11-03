#include "ThreadPool.hpp"

ThreadPool::ThreadPool(int noThreads): end(false)
{
	this->threads.reserve(noThreads);
	for (int i = 0; i < noThreads; ++i)
	{
		this->threads.emplace_back([this]() {this->run(); });
	}
}

void ThreadPool::close()
{
	std::unique_lock<std::mutex> lock(this->mutex);
	this->end = true;
	this->condVariable.notify_all();
}

void ThreadPool::enqueue(std::function<void()> threadFunction)
{
	std::unique_lock<std::mutex> lock(this->mutex);
	this->queue.push_back(std::move(threadFunction));
	this->condVariable.notify_one();
}

ThreadPool::~ThreadPool()
{
	close();
	for (std::thread& t : this->threads)
	{
		t.join();
	}
}

void ThreadPool::run()
{
	while (true)
	{
		std::function<void()> toExec;
		{
			std::unique_lock<std::mutex> lock(this->mutex);
			while (this->queue.empty() && !this->end)
			{
				this->condVariable.wait(lock);
			}
			if (this->queue.empty())
			{
				return;
			}
			toExec = std::move(this->queue.front());
			this->queue.pop_front();
		}
		toExec();
	}
}