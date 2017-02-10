1. Type 'make' to compile the GTThreads library 
2. Type 'make matrix' to compile the matrix program
3. ./bin/matrix 0 to run the matrix program with priority scheduler and ./bin/matrix 1 to run it with credit scheduler

Individual results and computed statistics are outputted to stdout and stderr for convenience.

NOTE!!! sometimes segfaults or stack smashing occurs - just `make clean`, then `make && make matrix` again and then finally `./bin/matrix 1` to try again. See report, issues section, for details on why this happens.