#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

#include "libheis/elev.h"

#include "operator.h"

#define SPEED 1000
#define WAITTIME 100
#define DOORTIME 3

//Private variables:

long int systemStartTime;


bool obstructionStored;
bool stopSignal;
int floorStored;

pthread_mutex_t mutexFloorStored;
pthread_mutex_t mutexObstructionStored;
pthread_mutex_t mutexStopSignal;


//Private functions:

static void setStopSignal(int dummy1, int dummy2){
	pthread_mutex_lock( &mutexStopSignal );
	stopSignal = 1;
	pthread_mutex_unlock( &mutexStopSignal );
}

static void storeObstructionSignal(int dummy, int value) {
	pthread_mutex_lock( &mutexObstructionStored );
	obstructionStored = value;
	pthread_mutex_unlock( &mutexObstructionStored );
}

static void storeFloorSignal(int floor, int value){
	elev_set_floor_indicator(floor);
	pthread_mutex_lock(&mutexFloorStored);


	if (value){
		floorStored=floor;
	}else{
		floorStored=-1;
	}

	pthread_mutex_unlock(&mutexFloorStored);

}

static int getFloor(){

	thread_mutex_lock(&mutexFloorStored);
	int temp = floorStored;
	pthread_mutex_unlock(&mutexFloorStored);

	return temp;
}

static bool getStopSignal(){

	pthread_mutex_lock(&mutexStopSignal);
	bool temp = stopSignal;
	pthread_mutex_unlock(&mutexStopSignal);

	return temp;
}

static void resetStopSignal() {
	pthread_mutex_lock(&mutexStopSignal);	
	stopSignal = 0;
	pthread_mutex_unlock(&mutexStopSignal);
}

static bool getObstructionSignal() {
	pthread_mutex_lock(&mutexObstructionStored);	
	bool temp=obstructionStored;
	pthread_mutex_unlock(&mutexObstructionStored);

	return temp;
}

static void printSystemRunTime(int mode) {
	int systemRunTime = time(NULL) - systemStartTime;
	int hours  = systemRunTime/3600;
	int min = (systemRunTime%3600)/60;
	int sec = (systemRunTime%3600)%60;

	if (mode == 0) {
		printf("SYSTEM RUN TIME: %i HOURS, %i MINUTES AND %i SECOUNDS.\n", hours,min,sec);
	} else {
		printf("Run time: \t%i:%i:%i\n", hours,min,sec);
	}
}

// Public functions:
// target = -1 er det samme som at heisen ikke har et target

States initializeOperator(){

	elev_register_callback( SIGNAL_TYPE_SENSOR, &storeFloorSignal );
	elev_register_callback( SIGNAL_TYPE_STOP, &setStopSignal );
	elev_register_callback( SIGNAL_TYPE_OBSTR, &storeObstructionSignal );

	States state = WAIT;
	systemStartTime = time(NULL);
return state;
}

States elevatorOperator(double floor, int *ptarget, States state){  // tar in ptarget i stedenfor target for og kunne teste operatoren
	int target=*ptarget;
	static int doorTime; 


	if ( getStopSignal() != 0 && state != STOP ){

		state = STOP;
		elev_set_speed(0);
		elev_set_stop_lamp(1);

		// hjernen må vite at heisen er i nødstop
		// I tilleg bør muligens commands slettes for denne heisen når man går inn i stop
		*ptarget=-1;


	}else{

		switch(state){
			case UP:

				if(target == floor){

					state = WAIT;
					elev_set_speed(0);

				}
			break;

			case DOWN:

				if(target == floor && target > -1){
					state = WAIT;
					elev_set_speed(0);
				};

			break;

			case WAIT:

				if (getObstructionSignal() == 0){ //goes back into DOOR if obstruction.

					state = DOOR;
					doorTime = time(NULL);
					elev_set_door_open_lamp(1);

				}else{
					if (target == floor){

						doorTime = time(NULL);
						state = DOOR;
						elev_set_door_open_lamp(1);

					}
					if (target >  floor){

						state = UP;
						elev_set_speed(SPEED);

					}
					if (target < floor && target > -1){

						state = DOWN;
						elev_set_speed(-SPEED);

					}
				}

			break;

			case DOOR:

				if ( getObstructionSignal() == 0)

					doorTime = time(NULL);

				if ((int) (time(NULL)) >= doorTime + DOORTIME){ 	//waits DOORTIME secounds befor closing

					state = WAIT;
					elev_set_door_open_lamp(0);
					*ptarget=-1;	// trenger en funktion som sier fra at at target har blit betjent


				};

			break;

			case STOP:

				if (target > floor && target != -1){ 		//if elevator has a target, then it will go out of NØD_STOP

					state = UP;
					resetStopSignal();
					elev_set_stop_lamp(0);
					elev_set_speed(SPEED);
					elev_set_door_open_lamp(0);


				}else if(target < floor && target != -1){

					state = DOWN;
					resetStopSignal();
					elev_set_stop_lamp(0);
					elev_set_speed(-SPEED);
					elev_set_door_open_lamp(0);

				}else if(target == floor && target != -1){

					state = WAIT;
					resetStopSignal();
					elev_set_stop_lamp(0);

				}

			break;
		}
	}

	return state;
}

double findFloor(double floorDouble, States state){ // eller er findFloor et bedre navn på denne?

	int floor = getFloor();

	if (floor == -1 && (floorDouble - (int) (floorDouble)) == 0 ){

		if (state == UP){         //1 = UP
			floorDouble = floorDouble + 0.5;
		}else if (state == DOWN){    //2 = DOWN
			floorDouble = floorDouble - 0.5;
		}

	}else if (floor != -1){
		floorDouble = floor;
	}

	return floorDouble;
}

void printState(double floorDouble, States state, int target){
		//prints information from list


		if(state == UP)
			printf("State: \t\tUP\n");
		else if (state == DOWN)
			printf("State: \t\tDOWN\n");
		else if (state == WAIT)
			printf("State: \t\tWAIT\n");
		else if (state == DOOR)
			printf("State: \t\tDOOR\n");
		else if (state == STOP)
			printf("State: \t\tSTOP\n");

		printf("Target:\t\t%i \n", target);
		printf("Current floor:\t%g \n", floorDouble);
		printSystemRunTime(1);
		printf("N_FLOORS: \t%i\n", N_FLOORS);
		printf("Door time: \t%i sec\n", DOORTIME);
		if(state == UP)
			printf("Speed:\t\t%i\n", SPEED);
		else if(state == DOWN)
			printf("Speed:\t\t-%i\n", SPEED);
		else
			printf("Speed:\t\t0\n");
		printf("------------------------------------\n");
};
