typedef enum {
  UP = 1,
	DOWN = 2,
	WAIT = 3,
	DOOR = 4,
	STOP = 5
} States; //state, DOWN = 2 ect.

States elevatorOperator(double floor, int *ptarget , States state);
States initializeOperator();
double findFloor(double floorDouble, States state);
void printState(double floorDouble, States state, int target);
