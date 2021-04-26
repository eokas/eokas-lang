#ifndef _EOKAS_ARCHAISM_ASYNC_H_
#define _EOKAS_ARCHAISM_ASYNC_H_

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
		mCond.notify_all(); // 唤醒所有线程执行
		for (std::thread& thread : mThreads) 
		{
			//thread.detach(); // 让线程“自生自灭”
			if (thread.joinable())
			{
				// 等待任务结束， 前提：线程一定会执行完
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
					// 获取一个待执行的 task
					Task task;
					{
						std::unique_lock<std::mutex> lock{ mMutex };
						mCond.wait(lock, [this]
						{
							return !mRunning || !mTasks.empty();
						});

						// wait 直到有 task
						if (!mRunning && mTasks.empty())
							return;

						// 按先进先出从队列取一个 task
						task = std::move(mTasks.front());
						mTasks.pop();
					}

					mIdleCount--;
					task(); // 执行任务
					mIdleCount++;
				}
			});

			mIdleCount++;
		}
	}

	// 执行一个任务
	// 调用.get()获取返回值会等待任务执行完, 获取返回值
	// 有两种方法可以实现调用类成员，
	// 1, bind: .exec(std::bind(&Dog::sayHello, &dog));
	// 2, mem_fn: .exec(std::mem_fn(&Dog::sayHello), this)
	template<typename F, typename... Args>
	auto exec(F&& f, Args&&... args) ->future<decltype(f(args...))>
	{
		if (!mRunning)
			throw runtime_error("commit on ThreadPool is stopped.");

		using RetType = decltype(f(args...)); 

		// 把函数入口及参数,打包(绑定)
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

		// 唤醒一个线程执行
		mCond.notify_one(); 

		return future;
	}

	//空闲线程数量
	int idle_size() const
	{ 
		return mIdleCount; 
	}

	//线程数量
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

#endif//_EOKAS_ARCHAISM_ASYNC_H_
