# pthreads-SingleLaneTunnelSimulation

The following program is a simulation of a single lane tunnel/bridge. Traffic must wait until it is their queue to travel, as periodically the direction changes to allow traffic from both sides to propogate. 

The purpose of this program was to gain knowledge of multi-threaded programming and mutexes. 


All car information is stored in a vector of structs.

Program use of pthreads:

1 mutex

1 pthread conditional

3 functions for pthreads

	1.) Tunnel
	
	2.) Cars heading towards Whittier
	
	3.) Cars heading towards Bear Valley


The tunnel function is simple, broadcasts when status changes every 5 seconds.

The car functions are identical for everything apart from the direction in which a car is headed.

The car functions do use mutex and rely on two conditions:

	1.) Tunnel direction matches car direction (default waiting)
	
	2.) Tunnel has space for a car (tunnel is full, increment waiting car count)

Car thread operations:

	1.) Sleep until car arrives
	
	2.) Lock mutex and output car arrival
	
	3.) Check conditions to allow passage, if condition fails, the mutex is relinquished 
	and a cond_wait is triggered.
	
	4.) Output car entering tunnel and increment count of cars in tunnel.
	
	5.) Unlock mutex
	
	6.) Sleep until car propogates
	
	7.) Lock mutex
	8.) Output car leaving tunnel and decrement count of cars in tunnel
	
	9.) Broadcast that space for another car is now available.
	
	10.) Unlock mutex followed by end of function

To compile use:

g++ -std=c++11 -fpermissive main.cpp -o source -lpthread

To run use:

./source <file name>
