//
// Created by jesse_wang on 6/23/17.
// jessejcw@gmail.com
//

/*
 * Simple producer/consumer multi-thread model
 * 1. Recycle threads:
 *    instead of starting a new thread for every incoming jobs, certain number
 *    of threads would be pre-invoked and put into wait. (cpu No. minus 1)
 *    Once job is coming, idle thread will be wake up and work; sleep otherwise.
 *    Each thread's life-cycle is the same as the whole process. (or "setExit()")
 *
 * 2. Interface:
 *    i. createWorkers(): init the mechanism
 *    ii. JobPkg: pair of a. custom data b. custom callback
 *    iii. enqueueJob(JobPkg& ): push job into job queue
 *    iv. setExit(): push the killer function into job queue
 *
 * 3. Misc:
 *    i. wait condition:
 *      a. producer wait: more than 99 jobs in queue
 *      b. consumer (worker): no jobs
 *    iii. awake condition:
 *      a. producer
 *
 *
 */

#ifndef SIMPLESERVER_WORKER_H
#define SIMPLESERVER_WORKER_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>


#define ERROR_NONE  0
#define ERROR_SOME -1

typedef int (*CB_FUNC) (void* data, int fd);
typedef std::tuple<void*, CB_FUNC, int> JobPkg;

class Worker {

    // based on cpu core
    int thread_count;

    // invoke thread
    std::mutex worker_mutex;
    std::mutex job_mutex;

    unsigned invoked_worker = 0;
    bool worker_invoked_cond_var = false;
    bool producer_wait = false;

    std::condition_variable producer_cv;
    std::condition_variable worker_cv;
    std::condition_variable job_cv;
    std::vector<std::thread> worker_threads;

    // critical section
    std::queue<JobPkg> job_queue;
    int idle_workr =0;

    // main worker function
    static void funcWorkrMain(void* data, int id) {
        Worker* mgr = static_cast<Worker*>(data);

        // spawn the consumer threads and wait
        mgr->invokeWorkr();

        std::cout<< "thread:" <<id <<std::endl;

        JobPkg curr_job;
        while (true) {
            // executed here when curr thread is done
            // with prev job and available for next job

            // blocking when job queue is empty
            mgr->dequeueJob(curr_job);

            // call back to requester
            mgr->processJob(curr_job);


            if(mgr->isExit()) {
                std::cout << "thread:" << id << " exit..."<<std::endl;
                break;
            }

            mgr->wakeUpProducer();
        }
    }

    void wakeUpProducer() {
        if (producer_wait == true) {
            // potential data race! dont care
            if (job_queue.size() < 99 ) {
                producer_cv.notify_one();
            }
        }
    }
    void processJob(JobPkg job) {
        std::get<1>(job)(std::get<0>(job), std::get<2>(job));
    }

    void dequeueJob(JobPkg &ret_pkg) {

        std::unique_lock<std::mutex> lk(job_mutex);
        while (job_queue.empty()) {
            ++idle_workr;
            job_cv.wait(lk);
            --idle_workr;
        }
        ret_pkg = job_queue.front();
        job_queue.pop();
        if (!job_queue.empty() && idle_workr) {
            job_cv.notify_one();
        }
    }

    void invokeWorkr() {
        std::unique_lock<std::mutex> lk(worker_mutex);
        ++invoked_worker;
        if (worker_invoked_cond_var && invoked_worker == thread_count)
            worker_cv.notify_one();
    }

    bool isExit() {
        std::unique_lock<std::mutex> lk(job_mutex);
        if (job_queue.empty()) { return false; }

        JobPkg job = (std::tuple<void *, CB_FUNC, int> &&) job_queue.front();
        return std::get<2>(job) == -7777;
    }

public:

    // user call from the main thread
    // the queue will dispatch the job automatically
    void enqueueJob(JobPkg& job) {
        std::unique_lock<std::mutex> lock(job_mutex);
        while(job_queue.size() > 99) {
            // job queue has over 99 job haven't been process
            // blocking producer
            producer_wait = true;
            producer_cv.wait(lock);
            producer_wait = false;
        }
        job_queue.push(job);
        if (idle_workr != 0) {
            job_cv.notify_one();
        }
    }

    ~Worker() {
        for(int i =0; i < thread_count; ++i) {
            worker_threads[i].join();
        }
    }
    int createWorkers() {

        // num of threads that cpu support minus main thread
        thread_count = std::thread::hardware_concurrency()-1;
        worker_threads.resize(thread_count);
        for(int i =0; i < thread_count; ++i) {
            worker_threads[i] = std::thread(Worker::funcWorkrMain, (void*)this, i);
        }

        while (invoked_worker < thread_count) {
            std::unique_lock<std::mutex> lk(worker_mutex);
            worker_invoked_cond_var = true;
            worker_cv.wait(lk);
            worker_invoked_cond_var = false;
        }
        return ERROR_NONE;
    }


    // on handling "Exit" by pushing a killer into queue
    void setExit() {
        std::unique_lock<std::mutex> lk(job_mutex);
        job_queue.push(JobPkg(nullptr, nullptr, -7777));
    }

};


#endif //SIMPLESERVER_WORKER_H
