/*
 * threadPool.cpp
 *
 *  Created on: 2021. 4. 6.
 *      Author: hsr
 */


#include <threadPool.h>


/*Server 클래스에서 안불러지길래 생성자 -> 함수로 대체
ThreadPool::ThreadPool(size_t num_threads)
    : num_threads_(num_threads), stop_all(false)
{
  worker_threads_.reserve(num_threads_);

  for (size_t i = 0; i < num_threads_; ++i)
  {
	worker_threads_.emplace_back([this]()
	{
		this->WorkerThread();
	});
  }
}
*/

void ThreadPool::ThreadPoolStart(size_t num_threads)
{
	stop_all = false;
	num_threads_ = num_threads;
	restThreadNum = 0;

	worker_threads_.reserve(num_threads_);

	  for (size_t i = 0; i < num_threads_; ++i)
	  {
		worker_threads_.emplace_back([this]()
		{
			this->WorkerThread();
		});
	  }
}

void ThreadPool::WorkerThread()
{
	while (true)
	{
		unique_lock<mutex> lock(m_job_q_);

		restThreadNum++;
		cout << "thread Ready to work " << restThreadNum << endl;
		cv_job_q_.wait(lock, [this]() {
			return !this->jobs_.empty() || stop_all;
		});
		if (stop_all && this->jobs_.empty()) {
			return;
		}

		cout << "스레드 " << this_thread::get_id() << " Working" << endl;

		// 맨 앞의 job 을 뺀다.
		function<void()> job = move(jobs_.front());
		jobs_.pop();
		lock.unlock();

		restThreadNum--;

		cout << "restThreadNum : " << restThreadNum << endl;

		// 해당 job 을 수행한다 :)
		job();
	}
}


int ThreadPool::get_restThreadNum()
{
	return restThreadNum;
}



ThreadPool::~ThreadPool() {
  stop_all = true;
  cv_job_q_.notify_all();

  for (auto& t : worker_threads_) {
    t.join();
  }

  cout << "Thread Pool Closed" << endl;
}

void ThreadPool::EnqueueJob(std::function<void()> job)
{
  if (stop_all)
  {
    throw runtime_error("ThreadPool 사용 중지됨");
  }

  {
    lock_guard<mutex> lock(m_job_q_);
    jobs_.push(move(job));
  }
  cv_job_q_.notify_one();
}






















