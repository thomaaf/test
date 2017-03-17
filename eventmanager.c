#include "elev.h"
#include "eventmanager.h"
#include <stdio.h>
#include "lights.h"
#include "fsm.h"
#include "timer.h"



static int list[4][3]={ //[floor][button]		//reminder: 1st floor in top of list, 4th floor at bottom, need to fix (the asset.c/prinlist prints opposite -> 1st at bottom)
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0} //
};

static int currentDestination[4]={0,0,0,0};	//Current state order queue

static int currentFloor;					//private variable, holds current floor
int INTRANSITION;							//states if the elevator is in between two floors. 1 = true. 
static int state=0; 							//orderstate: 1 = picking upwards orders. -1 = picking downwards orders. 0 = idle
static int lastengineDirection=0;					//Holds the last known engine direction. 1 = DIRN_UP.   -1 = DIRN_DOWN



void em_Queue_Updater( int pressedFloor, int pressedButton ){ //updates the queue with new order
	if ((pressedFloor==currentFloor)&&(INTRANSITION==0)){
		em_Door_Control(pressedFloor);
		return;
	}
	if (list[pressedFloor][pressedButton]==1){
		return;
	}
	list [pressedFloor][pressedButton]=1;							//sets new order to the main queue
	lights_Set_Order_Button_Light(pressedFloor,pressedButton); 		//sets the lights of the pushed button													

	printf("queue\n");												//prints the order list.
	em_Print_Status ();
 	fsm_Finite_state_Machine ( pressedFloor,  pressedButton,  &state,  currentFloor,  lastengineDirection, currentDestination,list);
 	

	

}



void em_Floor_Control(int newFloor){ 						//Controls what happens when elevator arrives at floor. f.ex stopping at floor and clearing order. 	
	if (INTRANSITION==1){									//Marks that the elevator is in transition, and that it should not do anything
		
		return;
	}
	else{
		
		

		currentFloor = newFloor;							//Updates the current floor to the new arrived floor
		lights_Indicator_Floor_Lights( newFloor);			//Updates the floorindicator light with the newly arrived floor

		if (state == 1){								//if we are in state of picking people going up
			if ((currentDestination[currentFloor]==1)||(currentDestination[currentFloor]==2)){	//if theres an UP or COMMAND order at the newly arrived floor, elevator should stop

				em_Door_Control(currentFloor);			//fixes doors and lights.
				currentDestination[currentFloor]=0;		//clears currentDestination orders
				list[currentFloor][BUTTON_COMMAND]=0;	//clears Listorders
				list[currentFloor][BUTTON_CALL_UP]=0;	//clears Listorders
				printf("floorcontrol state =1\n");

				em_Print_Status ();							
			}
		}


		else if (state == -1){								//if we are in state of picking people going up
			if ((currentDestination[currentFloor]==1)||(currentDestination[currentFloor]==2)){ //if theres an DOWN or COMMAND order at the newly arrived floor, elevator should stop
				em_Door_Control(currentFloor);
				currentDestination[currentFloor]=0;
				list[currentFloor][BUTTON_COMMAND]=0;
				list[currentFloor][BUTTON_CALL_DOWN]=0;
				printf("floorcontrol state =-1\n");
				em_Print_Status ();

			}
		}
	}
}

void em_Queue_Check(){						//Checks the main queue list for any more orders that hasnt been handled yet.
	printf("queue is running \n");
	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++ ){
			if (list[y][x]==1){
				fsm_Finite_state_Machine(y,x,  &state,  currentFloor,  lastengineDirection, currentDestination,list); 
				//pushes the orders found in main list as parameters to the statemachine. From the statemachines POV, these are passed as regular buttonpushes
			}

		}
	}
	printf("queue done \n");

	em_Print_Status ();
}

void em_Set_Engine(int direction){

	if (direction==DIRN_UP){
		elev_set_motor_direction(DIRN_UP);
		if(elev_get_floor_sensor_signal()!=-1){
			lastengineDirection=1;	
		
		}
		

	}
	else if(direction==DIRN_DOWN){
		elev_set_motor_direction(DIRN_DOWN);
		if (elev_get_floor_sensor_signal()!=-1){
			lastengineDirection=-1;	
			
		}
		
	
	}
	else{ //pressedFloor==currentFloor
		elev_set_motor_direction(DIRN_STOP);
		
		
	}
}

void em_Door_Control(int floor){
	if (state == 1){												//If the state is =1 then the only orders that should be handled are UP and Command
		lights_Clear_Order_Button_Light(floor, BUTTON_COMMAND);		//therefore they are the only lights that are cleared, given stop at a floor
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_UP);
		timer_Toggle_Timer(1);										//toggles the door timer
	}
	else if (state == -1){
		lights_Clear_Order_Button_Light(floor, BUTTON_COMMAND);		//If the state is =-1 then the only orders that should be handled are DOWN and Command
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_DOWN);
		timer_Toggle_Timer(1);										//toggles the door timer
	}
	else if (state ==0){											//exception case, where the queue has run, and finds an order from the same floor.
		lights_Clear_Order_Button_Light(floor, BUTTON_COMMAND);		//this order needs to be cleared, and can be both up or down. And since state is = 0, 
																	//we cant use the state for assertion about if the command is up or down
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_DOWN);	//This doesnt matter, as if this case happens, then there should be NO orders left for the floor, and
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_UP);		//all orders for that floor should be cleared. 
		currentDestination[currentFloor]=0;
		list[currentFloor][BUTTON_COMMAND]=0;
		list[currentFloor][BUTTON_CALL_DOWN]=0;
		list[currentFloor][BUTTON_CALL_UP]=0;
	}
	
	em_Set_Engine(DIRN_STOP);										//stops the engine at the floor
					
}

void em_Stop_Button(){

	elev_set_stop_lamp(1);						//sets the stop lamp 								
	for (int y=0; y<4; y++){					//clears all orders
		currentDestination[y]=0;
		for (int x=0; x<3; x++){
			list[y][x]=0;
			lights_Clear_Order_Button_Light(y,x);	//clears all orderlights
		}
	}
	
 	elev_set_motor_direction(DIRN_STOP);		//Stops the elevator

	while (elev_get_stop_signal()==1){			//loops untill the stopbutton is released, this asserts that no new orders can be recieve while stop button is pressed.
		if (elev_get_floor_sensor_signal()>-1){		//opens the door IF the sensors asserts that we are at a floor
			timer_Toggle_Timer(1);
		}
	}

	
	elev_set_stop_lamp(0);					//resets the elevator
	state=0;
	printf("stop button =1\n");
	em_Print_Status ();

	
}




int em_Check_Special_Case( ){
	if (state == 1){							//if the state is = 1, and there's and order labeled 2, then the elevator should proceed DOWN
		for (int x=0; x<4; x++){
			if (currentDestination[x]==2){		

				em_Set_Engine(DIRN_DOWN);
				return 1;
			}
		}
	}
	else if(state == -1){						//if the state is = -1, and there's and order labeled 2, then the elevator should proceed UP
		for (int x=0; x<4; x++ ){
			if (currentDestination[x]==2){

				em_Set_Engine(DIRN_UP);
				return 1;
			}
		}
	}
	return 0;
}

int em_Check_For_More_Orders(){
	if (state == 1){							//if the state is = 1, and there's and order labeled 1, then the elevator should proceed UP
		for (int x = 0 ; x<4; x++ ){ 			
			
			if (currentDestination[x]==1){
				em_Set_Engine(DIRN_UP);
				return 1;
			}
			
		}
	}
	else if (state == -1){
		for (int x = 0 ; x<4; x++ ){ 			//if the state is = -1, and there's and order labeled 1, then the elevator should proceed DOWN
			if (currentDestination[x]==1){
				em_Set_Engine(DIRN_DOWN);
				return 1;
			}
			
		}
	}
	state =0;
	printf("check for more orders = return 0\n");
	em_Print_Status ();
	em_Queue_Check(); //if there's no orders in currentDestination, then we need to check for new orders in the main list, and "restart" the elevator to state =0 
	return 0;

}

void em_Print_Status(){

	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++){
			printf("| %d",list[y][x] );
			printf(" ");
		}
		printf("\n");

	}
	printf(" |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("lsdir: % d",lastengineDirection); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("trans: % d",INTRANSITION); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("floor: % d",currentFloor); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("state: % d",state); printf("       |\n");
	printf("|----------------|\n");
	for (int x=0; x<4; x++){
		printf("| %d",currentDestination[x] );
		printf(" ");
	}
	printf(" |\n");
	printf("|----------------|\n");
	printf("\n");
	printf("\n");
	printf("\n");
}
