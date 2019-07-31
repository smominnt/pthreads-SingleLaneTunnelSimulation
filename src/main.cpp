#define HAVE_STRUCT_TIMESPEC
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

static int max_count = 0; //global max cars in tunnel
static int tunnel_count = 0; //global current cars in tunnel
static int wait_count = 0; //global waitcount

bool exec = 1; //boolean if cars are remaining
char dir; //global tunnel direction


static pthread_mutex_t traffic_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wake_up = PTHREAD_COND_INITIALIZER;

struct cars {
	cars(int ar, int pr, string bs) : arrive(ar), propogate(pr), bound(bs) {} //struct bracket enclosed constructor
	int arrive = 0;
	int propogate = 0;
	string bound;
};

vector <cars> allCars;

//tunnel thread, cycles through Whittier->Closed->Bear-Valley->Closed
void *tunnelThread(void *arg) {
	while (exec) {
		if (!exec) break;
		pthread_mutex_lock(&traffic_lock);
		printf("The tunnel now open to Whittier-bound traffic.\n");
		dir = 'W';
		pthread_cond_broadcast(&wake_up);
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);

		if (!exec) break;
		pthread_mutex_lock(&traffic_lock);
		printf("The tunnel is now closed to ALL traffic.\n");
		dir = 'N';
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);


		if (!exec) break;
		pthread_mutex_lock(&traffic_lock);
		printf("The tunnel now open to Bear-Valley bound traffic.\n");
		dir = 'B';
		pthread_cond_broadcast(&wake_up);
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);

		if (!exec) break;
		pthread_mutex_lock(&traffic_lock);
		printf("The tunnel is now closed to ALL traffic.\n");
		dir = 'N';
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);

	}
}

//cars going to Bear Valley carThread function
void *carThreadBB(void *arg) {
	int carNo;
	carNo = (int)arg;
	char bb[] = "Bear Valley";
	bool delay = 0;


	for (int i = carNo; i >= 0; i--) {
		sleep(allCars.at(i).arrive);
	} //sleep until arrival time of current car + all previous arrival times

	//LOCK MUTEX
	pthread_mutex_lock(&traffic_lock);
	printf("Car #%d going to %s arrives at the tunnel.\n", carNo + 1, bb);
	while (dir != 'B' || tunnel_count >= max_count) {
		if (tunnel_count >= max_count) {
			if (dir == 'B')
				delay = 1;
		}
		pthread_cond_wait(&wake_up, &traffic_lock);
	} //condition to check if tunnel is open in direction needed AND if there is space
	tunnel_count++;
	if (delay)
		wait_count++;
	printf("Car #%d going to %s enters the tunnel.\n", carNo + 1, bb);
	pthread_mutex_unlock(&traffic_lock);
	//UNLOCK MUTEX

	sleep(allCars.at(carNo).propogate); //sleep for propogation time through tunnel

	//MUTEX
	pthread_mutex_lock(&traffic_lock);
	printf("Car #%d going to %s exits the tunnel.\n", carNo + 1, bb);
	tunnel_count--;
	if (tunnel_count < max_count) //announce if there is space in the tunnel
		pthread_cond_broadcast(&wake_up);
	pthread_mutex_unlock(&traffic_lock);
	//MUTEX UNLOCK
}

//cars going to Whittier carThread function
void *carThreadWB(void *arg) {
	int carNo;
	carNo = (int)arg;
	char wb[] = "Whittier";
	bool delay = 0;


	for (int i = carNo; i >= 0; i--) {
		sleep(allCars.at(i).arrive);
	}//sleep until arrival time of current car + all previous arrival times
	
	//LOCK MUTEX
	pthread_mutex_lock(&traffic_lock);
	printf("Car #%d going to %s arrives at the tunnel.\n", carNo + 1, wb);
	while (dir != 'W' || tunnel_count >= max_count) {
		if (tunnel_count >= max_count) {
			if (dir == 'W')
				delay = 1;
		}
		pthread_cond_wait(&wake_up, &traffic_lock);
	}//condition to check if tunnel is open in direction needed AND if there is space
	tunnel_count++;
	if (delay)
		wait_count++;
	printf("Car #%d going to %s enters the tunnel.\n", carNo + 1, wb);
	pthread_mutex_unlock(&traffic_lock);
	//UNLOCK MUTEX

	sleep(allCars.at(carNo).propogate); //sleep for propogation time through tunnel

	//MUTEX LOCK
	pthread_mutex_lock(&traffic_lock);
	tunnel_count--;
	printf("Car #%d going to %s exits the tunnel.\n", carNo + 1, wb);
	if (tunnel_count < max_count) //announce if there is space in the tunnel
		pthread_cond_broadcast(&wake_up);
	pthread_mutex_unlock(&traffic_lock);
	//MUTEX UNLOCK
}

int main(int argc, char **argv) {

	FILE *fp;
	if (argc < 2) {
		cout << "Syntax ./source <file_name>\n";
		exit(1);
	}
	
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		cout << "File not found\n";
		exit(1);
	}
	
	fscanf(fp, "%d", &max_count); //get max cars in tunnel

	int arv; //arrive time
	char bd[2]; //direction
	int prop; //propogation time

	while (fscanf(fp, "%d %s %d", &arv, bd, &prop) != EOF) {
		allCars.push_back({arv, prop, bd});
	} //scan from file and store into vector of structs
	fclose(fp);


	int carNo = 0;
	int wbound = 0;
	int bbound = 0;

	pthread_t tunnel;
	pthread_t car_id[128]; //at most 128 cars from piazza comment from professor
	pthread_create(&tunnel, NULL, tunnelThread, (void *) 0);

	while (carNo < allCars.size()) {
		if (allCars.at(carNo).bound == "WB") {
			pthread_create(&car_id[carNo], NULL, carThreadWB, (void *)(carNo));
			wbound++;
		}
		else {
			pthread_create(&car_id[carNo], NULL, carThreadBB, (void *)(carNo));
			bbound++;
		}
		carNo++;
	}
	

	for (int i = 0; i < allCars.size(); i++) {
		pthread_join(car_id[i], NULL);
	}
	exec = 0;
	pthread_join(tunnel, NULL);

	cout << bbound << " car(s) going to Bear Valley arrived at the tunnel.\n";
	cout << wbound << " car(s) going to Whittier arrived at the tunnel.\n";
	cout << wait_count << " car(s) had to wait because the tunnel was full.\n";


	return 0;
}
