Project:

Simulations of Memory Management Unit (MMU) algorithms:
-> LFU (Least Frequent Used)
-> WS (Working Set)
-> Hashed Page Table

The algorithms are applied to these two trace files:
bzip.trace
gcc.trace


Compile and Run:

Visual Studio Project:
1. Compile
2. Run from Command Prompt with following parameters:

>path_of_executable/MMU.exe <alg> (<ws>) <frames> <q> (<max>)

alg:  	LFU or WS
ws:   	Size of window for WS algorithm if working set algorithm is chosen
frames: Number of frames of main memory
q:    	Set of reference traces to be read each time from each file
max: 	Maximum number of references to be read from the trace files

Result is the calculation the following statistics after applying the algorithms to the trace files:
Number of writes into hard disk 
Number of reads from hard disk 
Number of page faults 
Number of references read  
Number of memory frames 
