/*
 * threadPool.h
 *
 *  Created on: 2021. 4. 5.
 *      Author: hsr
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <headers.h>

class ThreadPool
{
 public:
  //ThreadPool(size_t num_threads);
  ~ThreadPool();

  void ThreadPoolStart(size_t num_threads);

  // job 을 추가한다.
  void EnqueueJob(function<void()> job);

  int get_restThreadNum();

 private:
  // 총 Worker 쓰레드의 개수.
  size_t num_threads_;

  // Worker 쓰레드를 보관하는 벡터.
  vector<thread> worker_threads_;

  // 할일들을 보관하는 job 큐.
  queue<function<void()>> jobs_;

  // 위의 job 큐를 위한 조건변수, 뮤텍스
  condition_variable cv_job_q_;
  mutex m_job_q_;

  // 모든 쓰레드 종료
  bool stop_all;

  int restThreadNum;

  // Worker 쓰레드
  void WorkerThread();
};


#endif /* THREADPOOL_H_ */


























