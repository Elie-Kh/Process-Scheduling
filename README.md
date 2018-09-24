# Process-Scheduling

Process-Scheduling in collaboration with Aida Bumbu

Objective:

Simulate a simplified version of Linux 2.6 - 2.6.63 O(1) scheduler.

Description:

We consider a single processor system able to run 1 process at a time. 
This creates a wait line for other processes to be able to run. 
Processes each have their own proper priority.

The scheduler described:

- The scheduler has 2 queues. 1 Queue flagged as the active queue, and the other queue flagged as the expired/dormant queue.
- Whenever a new process joins, it is stored in the dormant queue.
- Processes in the active queue may request the CPU.
- Every process has its own PID, arrival time, CPU burst and priority. 
- Priorities range from 1 to 139.
- Time quantums are assigned to processes based on the following equations:

	If priority < 100:
Time Quantum = (140 - priority) * 20 (milliseconds).
	If priority >= 100:
Time Quantum = (140 - priority) * 5 (milliseconds).
- After a process's time quantum passes, it is moved from the active queue to the expired queue.
- After the active queue is empty, switch the flag of the active queue to expired and the flag of the expired queue to active. Now, the previous expired queue becomes the active queue and the previous active queue becomes the expired queue.
- Priorities are updated throughout the program based on the following relationships:

Waiting time = Sum of waiting_times of the process.
bonus = ceiling(10 * waiting time/(current_time - arrival time))
New_priority = max(100, min((old_priority - bonus + 5) , 139))

The Program:

