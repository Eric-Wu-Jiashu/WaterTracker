# WaterTracker
## Description 
This project is a coaster that can track how much water the user has drank throughout the day. It will provide a recommendation for water intake amount and show how much progress has the user made to reach the recommended amount and how much should be drank. The project encourages users to have a healthy habit of drinking water. 

## Parts used 
* STM32 F103C8T6 blue pill
* 0.96 Inch OLED Display (Driver Used:)
* Force Sensitive Resistors X4
* Push Button X2
* resistors *6 

## Functionality
The Water intake is collected by detecting the weight change on the coaster through the four force-sensitive resistors. 
If the weight decreases, the decreased amount is collected as water intake, if the weight increases, the increased amount will be considered as water refilling. During setup, the user will be asked to put an empty cup on the coaster, the weight value of that will be treated as the zero point, and the system won't be taking any action to weigh below this. 

First, the system will guide the user through a setup process, which will ask for body weight for recommendations and detect the weight of the cup. 
Then the main program starts by showing a progress bar displaying how much progress has the user made and how much is left to complete to reach the recommendation amount. 
If the recommendation amount has been reached, the system will display a congratulations message. 
At the end of the day, the system will reset itself by clearing the progress amount and emptying the progress bar. 

