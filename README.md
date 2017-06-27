# SimpleServer
Description:

This implementation is more focus on multi-threaded producer/consumer model;
a recycled threaded mechanism along with a easy interface which marshall
incoming connections to child thread.


Classes:

SimpleServer : main server body, start the server, start the worker,
               coupling the I/O with Workers.
               
Worker.h     : Simple producer/consumer multi-thread model

1. Recycle threads:
Instead of starting a new thread for every incoming jobs, certain number of threads would be pre-invoked and put into wait. (cpu No. minus 1) Once job is coming, idle thread will be wake up and work; sleep otherwise. Each thread's life-cycle is the same as the whole process. (or "setExit()")

2. Interface:
i. createWorkers(): init the mechanism
ii. JobPkg: pair of a. custom data b. custom callback
iii. enqueueJob(JobPkg& ): push job into job queue
iv. setExit(): push the killer function into job queue

3. Misc:
i. wait condition:
   a. producer wait: more than 99 jobs in queue
   b. consumer (worker): no jobs


Usage: (Ubuntu)

1. under SimpleServer/> make
2. IP addr : 127.0.0.1 port is expecting 9800
3. could afford multiple client app to connect.
