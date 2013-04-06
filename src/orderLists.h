#define N_FLOORS 4
#define MAX_N_ELEVATORS 3

typedef enum {
	UPWARD = 1, 
	DOWNWARD = 2, 
	LOCKONOUTSIDE = 3, 
	NONE = 0
}Priorities; 	//used for find target.

typedef enum {
	up = 1,
	down = 0,
}dir;

typedef struct{
	int timeRegistered;
	bool targeted;
	bool registered;
}orderInfo;

typedef struct{
	Priorities priorety;
	bool registered;
	bool emergencyStop;
}elevatorStatus;

void initLists();
int registerElevator();
void deleteElevator(int elevator);
void clearTargetedOrderFromLists(int target , int elevator, dir elevatorDir);
int registerCallUp(int button);
int registerCallDown(int button);
int registerCallOrder(int floor, int elevator);
void setLights(double floor, int elevator);
