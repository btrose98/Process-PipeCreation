# Process-PipeCreation
Process/Pipe Creation and Inter-Process Communication

This assignment was a part of Operating Systems.

The goals of this assignment were to practice:
  1.  Process creating in Linux using fork() and exec() system calls
  2.  Thread creation in Linux with the pthread API
  3.  inter-process communication using pipes.
  4.  Practice C coding.
 
Description:
  Three process were created, one parent (User) and two children (Router and Server). User and Server were not allowed to directly communicate with each other. Instead, any information passed between the two of them had to go through the Router child. To do this, four pipes were used; Two of these pipes were used between User and Router. One pipe was dedicated for User to write to and Router to read from, while the other pipe was dedicated for User to read from and Router to write to. The same was done with the remaining to pipes connecting Router and Server. By having two pipes between each pair of processes, this ensured no deadlock would occur where two processes would both be attempting to either read or write to the pipe at the same time. Once the processes and pipes were set up, it was required to have User send a querry time signal, t, at least two times to Server. Server responded by sending back to User information regarding the current time. User took this information and printed the current time to the terminal. Once this was completed, User sent a new command, q, which told Server to quit. Upon retreval of a PID status signal of 0 from Server, User would then kill the Router process, close all pipes, and end the program.
