#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <future>
#include <list>
#include "semaphore.h"

namespace zenith
{
	namespace util
	{
		namespace thread
		{

			template<class T, class Ret, size_t NumWorkers = 1>
			class task_queue
			{
				struct task
				{

					T taskInfo;
					volatile float priority;
					std::packaged_task<Ret(T&)> pfnExecute;

					task(T &&t, float priority, std::packaged_task<Ret(T&)> &&pfnExecute) : taskInfo(std::move(t)), priority(priority), pfnExecute(std::move(pfnExecute)) {}
					task(task &&task) : taskInfo(std::move(task.taskInfo)), priority(task.priority), pfnExecute(std::move(task.pfnExecute)){}
					
					//Task &operator =(const Task &) = delete;
					task &operator =(task && task)
					{
						taskInfo = std::move(task.taskInfo);
						priority = task.priority;
						pfnExecute = task.pfnExecute;
						return *this;
					}

					std::future<Ret> get_future()
					{
						return pfnExecute.get_future();
					}
					
				};

				std::list<task> q_;

				mutable std::mutex qmutex_;
				mutable zenith::util::thread::semaphore signalUpdate_;
				volatile bool stop_ = false;
				std::thread worker_[NumWorkers];
				
				task_queue(const task_queue<T, Ret, NumWorkers> &) = delete;
				task_queue(task_queue<T, Ret, NumWorkers> &&) = delete;
				task_queue<T, Ret, NumWorkers> &operator =(const task_queue<T, Ret, NumWorkers> &) = delete;
				task_queue<T, Ret, NumWorkers> &operator =(task_queue<T, Ret, NumWorkers> &&) = delete;

				static void workerJob_(task_queue<T, Ret, NumWorkers> &queue)
				{
					while (!queue.stop_)
					{					
						while (queue.empty())
						{
							queue.signalUpdate_.wait();
							if (queue.stop_)
							{
								queue.signalUpdate_.notify();
								return;
							}
						}				
						task cur = queue.pop_();
						cur.pfnExecute(cur.taskInfo);
					}
				}

				task pop_()
				{					
					std::lock_guard<std::mutex> lock(qmutex_);
					if (q_.empty())
						throw std::out_of_range("task_queue::retrieveTask: queue is empty!");

					auto iter = q_.begin();
					auto besti = q_.begin();
					float bestf = besti->priority;

					iter++;
					for (; iter != q_.end(); iter++)
						if (iter->priority > bestf)
						{
							besti = iter;
							bestf = besti->priority;
						}
					task t = std::move(*besti);
					q_.erase(besti);
					return t;
				}
			public:
				task_queue()
				{
					for (size_t i = 0; i < NumWorkers; i++)
						worker_[i] = std::thread(workerJob_, std::ref(*this));
				}
				~task_queue()
				{
					stop_ = true;
					signalUpdate_.notify();
					for (size_t i = 0; i < NumWorkers; i++)
						worker_[i].join();
				}
				
				template<class F>
				std::future<Ret> push(float priority, T &&taskInfo, F &&func)
				{
					std::lock_guard<std::mutex> lock(qmutex_);
					q_.emplace_back(std::move(taskInfo), priority, std::packaged_task<Ret(T&)>(std::move(func)));
					std::future<Ret> fut = q_.back().get_future();				
					signalUpdate_.notify();
					return fut;
				}
				bool empty() const
				{
					std::lock_guard<std::mutex> lock(qmutex_);
					return q_.empty();
				}
			};

		}
	}
}
