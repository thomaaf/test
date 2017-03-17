#include "lights.h"
#include <stdio.h>
#include "elev.h"

void  lights_Indicator_Floor_Lights(int floor){
	if (floor==-1){
		return;
	}
	elev_set_floor_indicator( floor);
}

void lights_Set_Order_Button_Light (int floor, int button){
	if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
		elev_set_button_lamp( button, floor, 1);
	}
}

void lights_Clear_Order_Button_Light(int floor, int button){
	if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
		elev_set_button_lamp( button, floor, 0);
	}
}
