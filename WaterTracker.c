#include "stdio.h"
#include "WaterTracker.h"
#include "math.h"

#define true 1
#define false 0


#define Button_Delay 50 // delay after initial button input detected, change based on button quolity


/*
 * This function converts resistance to gram force for my FSR
 * please change the function to the one matching your sensor output
 */
int Resistence_To_Gramforce_Convertor(double R_In){
	 int G_Out = 2832.90950779/(R_In*sqrt(R_In));
	 return G_Out;
}
double ADC_Reading_Conversion(uint16_t ADC_Reading);

int System_State = 0;


//anti-viberation algorithm and input detection

int Button_Select_State = false;
int Button_Select_PrevSystick = 0;

int Button_Confirm_State = false;
int Button_Confirm_PrevSystick = 0;

void ButtonSet(int* Button_State, int* Button_PrevSystick);
void ButtonReset();



void UI_Welcome1();
void UI_Setup(int weight);
void UI_Confirm();
void UI_Progress(int progress);
void UI_Complete();


int System_Time_Second = 0;//system time in second, used for reset in the beginning of the day


int bodyweight = 0;
int target = 0;
int progress = 0;

int Cupweight = 0; //minimum reading for detection, no action if lower than this.
int TotalWeight = 0;// current weight on the coaster
int Prev_TotalWeight = 0;//previous weight

uint16_t ADC_BUF[4];//DMA buffer for ADC

void WeightConversion();



/*
 * initializing peripherals
 * Start ADC DMA
 * Display welcome page
 * modify the time variable to match the real world time
 */

void WTInit(){
	ssd1306_Init();
	UI_Welcome1();
	ssd1306_UpdateScreen();
	HAL_Delay(5000);

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) ADC_BUF, 4);

	System_Time_Second = (Current_Time_Hour * 60 + Current_Time_Minute)*60;

}

void WTMain(){

	// Update screen
	ssd1306_UpdateScreen();
	ssd1306_Fill(0);

	HAL_Delay(100);

	//reset button state if they were set
	ButtonReset();

	//get weight reading
	WeightConversion();

	/*
	 * reset detection
	 * if total second reach the total second in a day
	 * set UI, reset progress and reset time counter
	 */
	if (System_Time_Second >= 86400){
		progress = 0;
		System_State = 2;
		System_Time_Second = 0;
	}

	/*
	 * State 0 - setup state
	 * This state ask user for body weight and convert
	 * them to daily intake target
	 */
	if (System_State == 0){
		if (Button_Select_State){
			bodyweight += 10;
			if (bodyweight > 300){bodyweight = 0;}
		}

		if (Button_Confirm_State){
			System_State = 1;
			HAL_TIM_Base_Start_IT(&htim4);
			target = bodyweight * 35;
		}
		UI_Setup(bodyweight);
	}

	/*
	 * state 1 - info state
	 * show user their target
	 * get cup weight
	 */
	else if (System_State == 1){
		UI_Confirm();
		System_State = 2;

		Cupweight = TotalWeight + 50;
		Prev_TotalWeight = TotalWeight;

		ssd1306_UpdateScreen();
		HAL_Delay(5000);

	}

	/*
	 * state 2 - progress state
	 * show progress bar
	 * detect weight change and convert to progress
	 */
	else if (System_State == 2){
		if (TotalWeight > Cupweight){
			if (TotalWeight < (Prev_TotalWeight - 50)){
				HAL_Delay(1000);
				WeightConversion();
				progress += (Prev_TotalWeight - TotalWeight);
				Prev_TotalWeight = TotalWeight;
			}else if(TotalWeight > (Prev_TotalWeight + 50)){
				HAL_Delay(1000);
				WeightConversion();
				Prev_TotalWeight = TotalWeight;
			}

			if (progress >= target){
				System_State = 3;
			}
		}

		UI_Progress(progress);
	}

	/*
	 * state 3 - complete state
	 * show the congratulation to user
	 */
	else if (System_State == 3){
		UI_Complete();
	}

}



//add second passed to time counter variable
//trigger every second
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4){
	  System_Time_Second += 5;
  }
}



/*
 * Button input detection and anti vibration
 * if button input detected, change the state to true, and reset it back to false after button delay
 * no action if button state is true to prevent multiple input
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_13){
		ButtonSet(&Button_Select_State, &Button_Select_PrevSystick);
	}
	if(GPIO_Pin == GPIO_PIN_12){
		ButtonSet(&Button_Confirm_State, &Button_Confirm_PrevSystick);
	}
}

void ButtonSet(int* Button_State, int* Button_PrevSystick){
	if(!*Button_State && ((HAL_GetTick() - *Button_PrevSystick) > Button_Delay)){
		*Button_State = true; *Button_PrevSystick = HAL_GetTick();
	}
}

void ButtonReset(){
	if(Button_Select_State && ((HAL_GetTick() - Button_Select_PrevSystick) > Button_Delay)){
		Button_Select_State = false; Button_Select_PrevSystick = HAL_GetTick();
	}
	if(Button_Confirm_State && ((HAL_GetTick() - Button_Confirm_PrevSystick) > Button_Delay)){
		Button_Confirm_State = false; Button_Confirm_PrevSystick = HAL_GetTick();
	}
}

void WeightConversion(){
	TotalWeight = 0;
		for(int i = 0; i < 4; i++){
			int weight = Resistence_To_Gramforce_Convertor(ADC_Reading_Conversion(ADC_BUF[i]));
			TotalWeight += weight;
		}

}

//convert ADC reading to resistance of the divider circuit
double ADC_Reading_Conversion(uint16_t ADC_Reading){
	double V_Analog = (ADC_Reading)*(3.3/4095.0);
	double R_Out = (3.3*Pull_Down_Resistor-V_Analog*Pull_Down_Resistor)/(V_Analog);
	return R_Out;
}


/*
 * UI to display on ssd1306 screen.
 */
void UI_Welcome1(){
	ssd1306_SetCursor(23, 22); 	ssd1306_WriteString("Welcome to ", Font_7x10, 1);
	ssd1306_SetCursor(20, 37);	ssd1306_WriteString("WaterTracker!", Font_7x10, 1);
}

void UI_Setup(int weight){
	ssd1306_SetCursor(0, 1);	ssd1306_WriteString("Enter your body weight", Font_6x8, 1);
	ssd1306_SetCursor(0, 16);	ssd1306_WriteString("Blue button-select", Font_6x8, 1);
	ssd1306_SetCursor(0, 31);	ssd1306_WriteString("Red button-confirm.", Font_6x8, 1);

	char str[8];
	sprintf(str, "%3d KG", weight);
	ssd1306_SetCursor(5, 46); 	ssd1306_WriteString(str, Font_7x10, 1);
}

void UI_Confirm(){
	char strC[20];
	sprintf(strC, "Aim for %4dml.", target);

	ssd1306_SetCursor(10, 16);	ssd1306_WriteString(strC, Font_7x10, 1);
	ssd1306_SetCursor(10, 31);	ssd1306_WriteString("Stay hydrated!", Font_7x10, 1);
	ssd1306_SetCursor(0, 46);	ssd1306_WriteString("Checking CupWeight", Font_7x10, 1);
}

void UI_Progress(int progress){

	ssd1306_DrawRectangle(0, 0, 127, 15, 1);
	ssd1306_FillRectangle(0, 0, (progress * 127/target), 15, 1);

	int presentage = (progress * 100 / target);
	char strp1[20];
	sprintf(strp1, "%4dml%10d%%", progress, presentage);
	ssd1306_SetCursor(0, 20);	ssd1306_WriteString(strp1, Font_7x10, 1);
	char strP2[30];
	sprintf(strP2, "%4dml To Go", (target - progress));
	ssd1306_SetCursor(10, 35);	ssd1306_WriteString(strP2, Font_7x10, 1);
	ssd1306_SetCursor(16, 50);	ssd1306_WriteString("Stay Hydrated!", Font_7x10, 1);

}

void UI_Complete(){
	ssd1306_SetCursor(16, 0);	ssd1306_WriteString("Congratulations", Font_7x10, 1);
	ssd1306_SetCursor(20, 20);	ssd1306_WriteString("Goal Reached", Font_7x10, 1);
	ssd1306_SetCursor(25, 40);	ssd1306_WriteString("Well Done!", Font_7x10, 1);
}
