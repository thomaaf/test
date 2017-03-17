
#include <stdio.h>
#include "fsm.h"
#include "elev.h"
#include "eventmanager.h"




void fsm_Finite_state_Machine (int pressedFloor, int pressedButton, int *state, int currentFloor, int lastEngineDirection, int currentDestination[4],int list[4][3]){
	

	if (*state ==0){												//*state=0 => Elevator is stationary
	
		if (pressedFloor>currentFloor){								//If there's an order above the current floor
			*state = 1;												//simplification. Assumes that the order is an UP or Command
		
			if (pressedButton==BUTTON_CALL_DOWN){					//if the order is not a UP or COMMAND, then the *state should be -1, which is asserted here
				*state = -1;																		
				currentDestination[pressedFloor]=2; 				//To keep a tab on the upper command of DOWNgoing orders. As long as there's a 2 in the list
				em_Set_Engine(DIRN_UP);								//and the *state is -1, then the elevator will move UPwards. 
				return;


			}
			else {
				currentDestination[pressedFloor]=1; 				//The order was just a regular UP or COMMAND, and the elevator moves UP with *state =1 
				em_Set_Engine(DIRN_UP);
				return; 
			
			}

			

		}													
		else if(pressedFloor<currentFloor){							//If there's an order below the current floor
			*state = -1;											//simplification. Assumes that the order is an DOWN or COMMAND

			if (pressedButton==BUTTON_CALL_UP){						//if the order is not a DOWN or COMMAND, then the *state should be 1, which is asserted here
				*state = 1;												
				currentDestination[pressedFloor]=2; 				//To keep a tab on the lower command of UPgoing orders. As long as there's a 2 in the list
				em_Set_Engine(DIRN_DOWN);							//and the *state is 1, then the elevator will move DOWNwards
				return;

			}
			else{
				currentDestination[pressedFloor]=1;					//The order was just a regular DOWN or COMMAND, and the elevator moves DOWN with *state =-1 
				em_Set_Engine(DIRN_DOWN);
				return;

			}


		}
		else if (pressedFloor == currentFloor){						//Should only happen after a STOPprocedure, as the elevator is stuck between two floors, 
																	//we need to check where it should move next
			if (INTRANSITION==1){									


				if (lastEngineDirection==1){													//if we haven driven a bit over the last floor
					em_Set_Engine(DIRN_DOWN);													//then we have to go back DOWN to the prev floor
					if ((pressedButton==BUTTON_COMMAND)||(pressedButton==BUTTON_CALL_DOWN)){	
						*state=-1;
						currentDestination[pressedFloor]=1;	
					}
					else if (pressedButton==BUTTON_CALL_UP){
						*state=1;
						currentDestination[pressedFloor]=2;										//we are going down to pick up a person going up. 2 marks the turning point
					}
				}

				else if(lastEngineDirection==-1){												//if we haven driven a bit below the last floor
					em_Set_Engine(DIRN_UP);														//then we have to go back UP to the prev floor
					if((pressedButton=BUTTON_COMMAND)||(pressedButton==BUTTON_CALL_UP)){		
						*state = 1;
						currentDestination[pressedFloor]=1;	
					}
					else if(pressedButton==BUTTON_CALL_DOWN){
						*state=-1;
						currentDestination[pressedFloor]=2;										//We are going UP to pick up a person going DOWN. 2 marks the turning point.	
					}
				}
				
				return;

			}
			else{
					list[pressedFloor][pressedButton]=0;
					em_Door_Control(pressedFloor);
					return;
			}
		}
		printf("Exception: Has reached end of *state == 0. Should exit before this.\n");
		return;
	}

	else if(*state ==1){												//*state = 1 => Elevator is picking up people going UP.


		if ((pressedFloor>currentFloor)&&((pressedButton== BUTTON_CALL_UP)||(pressedButton==BUTTON_COMMAND))){  //Picking people who's going up. And these orders are above current location
			currentDestination[pressedFloor]=1; 
			return;
		}
		else if((pressedFloor<currentFloor)&&(pressedButton==BUTTON_CALL_UP)){	//exeptioncase. The order is below current elevator position, Elevator needs to go down to pick up
		
			for (int x=0; x<4; x++){
				if ((currentDestination[x]==2)&&(pressedFloor<x)){	//Checks if there's any more orders further below the current new order.
					currentDestination[x]=1;						//if yes, then the found order should be reset to 1, and the new order should have the turningpoint and be = 2; 
					currentDestination[pressedFloor]=2;				//Updates the destination list with the order
					return;
				}

			}

			return;
		}
		else if (pressedFloor==currentFloor){						//Should also be handled by Queuehandler; 
			if (lastEngineDirection==-1){
				currentDestination[pressedFloor]=1;
			}
			else if(lastEngineDirection==1){
				currentDestination[pressedFloor]=2;
			}
			return;
		}
		return;
	}

	else if (*state==-1){											//*state = -1 => Elevator is picking up people going DOWN.
	

		if ((pressedFloor>currentFloor)&&(pressedButton==BUTTON_CALL_DOWN)){	//exeptioncase. The order is above current elevator position, Elevator needs to go UP to pick up
			for (int x=0; x<4; x++){
				if ((currentDestination[x]==2)&&(pressedFloor>x)){	//Checks if there's any more orders further above the current new order.
					currentDestination[x]=1;						//if yes, then the found order should be reset to 1, and the new order should have the turningpoint and be =2;
					currentDestination[pressedFloor]=2;				//Updates the destination list with the order
					return;											//Returns, to make the operation 1 or 2 iteration faster.. wopwop
				}
			}
			return;
		}
		else if((pressedFloor<currentFloor)&&((pressedButton== BUTTON_CALL_DOWN)||(pressedButton==BUTTON_COMMAND))){  //Picking people who's going dwon. And these orders are below current location
			currentDestination[pressedFloor]=1;
			return;	
		}
		else if (pressedFloor==currentFloor){
			if (lastEngineDirection==-1){
				currentDestination[pressedFloor]=2;
			}
			else if(lastEngineDirection==1){
				currentDestination[pressedFloor]=1;
			}
			
			return;
		}
		return;
	}
}