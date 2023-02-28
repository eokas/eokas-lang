#ifndef _EOKAS_BASE_ASYNC_H_
#define _EOKAS_BASE_ASYNC_H_

#include "header.h"

#include <atomic>
#include <future>
#include <stdexcept>
#include <functional>

_BeginNamespace(eokas)

#define  THREADPOOL_MAX_NUM 16

class ThreadPool
{
public:
	inline ThreadPool(unsigned short size = 4)
	{ 
		this->expand(size); 
	}

	inline ~ThreadPool()
	{
		mRunning = false;
		mCond.notify_all();
		for (std::thread& thread : mThreads) 
		{
			//thread.detach();
			if (thread.joinable())
			{
				thread.join(); 
			}
		}
	}

	void expand(unsigned short size)
	{
		if (size <= 0)
			return;
		for (unsigned short i = 0; i < size && mThreads.size() < THREADPOOL_MAX_NUM; i++)
		{
			mThreads.emplace_back([this]
			{
				while (mRunning)
				{
					Task task;
					{
						std::unique_lock<std::mutex> lock{ mMutex };
						mCond.wait(lock, [this]
						{
							return !mRunning || !mTasks.empty();
						});

						if (!mRunning && mTasks.empty())
							return;

						task = std::move(mTasks.front());
						mTasks.pop();
					}

					mIdleCount--;
					task();
					mIdleCount++;
				}
			});

			mIdleCount++;
		}
	}

	// 1, bind: .exec(std::bind(&Dog::sayHello, &dog));
	// 2, mem_fn: .exec(std::mem_fn(&Dog::sayHello), this)
	template<typename F, typename... Args>
	auto exec(F&& f, Args&&... args) ->future<decltype(f(args...))>
	{
		if (!mRunning)
			throw runtime_error("commit on ThreadPool is stopped.");

		using RetType = decltype(f(args...)); 

		auto task = std::make_shared<std::packaged_task<RetType()>>(
			bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<RetType> future = task->get_future();
		{    
			std::lock_guard<std::mutex> lock{ mMutex };
			mTasks.emplace([task]() 
			{
				(*task)();
			});
		}

		if (mIdleCount < 1 && mThreads.size() < THREADPOOL_MAX_NUM)
		{
			this->expand(1);
		}

		mCond.notify_one(); 

		return future;
	}

	int idle_size() const
	{ 
		return mIdleCount; 
	}

	int size() const
	{ 
		return mThreads.size(); 
	}

private:
	using Task = std::function<void()>;
	std::vector<std::thread> mThreads;
	std::queue<Task> mTasks;
	std::mutex mMutex;
	std::condition_variable mCond;
	std::atomic<bool> mRunning { true };
	std::atomic<int>  mIdleCount { 0 };
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_ASYNC_H_
