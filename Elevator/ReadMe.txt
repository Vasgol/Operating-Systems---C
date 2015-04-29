An elevator simulation for Linux OS processes:
-> Elevator represents a shared memory segment bound by the mother process
-> The mother process forks processes-users of the elevator and controls the simulation
   - Changes floors of the elevator 
   - Sends signals to wake-up the processes that reached their floor
-> Semaphores are used to 
   - Lock the access to the elevator before another process can use it
   - Lock elevator by the mother process to change floors

Compile:

> make 

Run:

> ./elevator <N> <F> <X> <S> 
	
  N: Number of processes to be forked (users of the elevator)
  F: Number of floors
  X: Steps of simulation (mother process locks and controls the elevator)
  S: Total number of steps of the simulation

Clean object files

> make clean 