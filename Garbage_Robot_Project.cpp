const float lengthOfBin = 40; //distance needed to reverse from wall
const float WHEEL_RADIUS = 2.75;
const float ROBOT_LENGTH = 20;
const float CONVERSION_FACTOR = (2*PI*WHEEL_RADIUS) / 360.0;

//Sets up all of the sensors and configures them
void sensorSetUp() 
{
	SensorType [S4] = sensorEV3_Gyro;
	wait1Msec (50);
	SensorMode[S4]= modeEV3Gyro_Calibration;
	wait1Msec (50);
	SensorMode [S4] = modeEV3Gyro_RateAndAngle;
	wait1Msec (50);
	resetGyro (S4);
	SensorType [S2] = sensorEV3_Touch;
	wait1Msec (50);
	nMotorEncoder [motorA] = 0;
	nMotorEncoder [motorD] = 0;
	nMotorEncoder [motorC] = 0;

}

//Displays text instructions on the screen
void giveInstructions() 
{
	displayTextLine(1, "place trash on platform"); //make sure not too long
	displayTextLine(2, "indicate bin it should go in");
	displayTextLine(3, "left = garbage");
	displayTextLine(4, "center = recycling");
	displayTextLine(5, "right = compost");
	wait1Msec(50);
}

//stores target bin colour depending on button pressed
int getBinColour() 
{
	clearTimer(T1); //resets timer at start of program
	while((!getButtonPress(buttonAny)))
	{}
	if(getButtonPress(buttonLeft)){
		wait1Msec(2000);
		return 1;
	}
	else if(getButtonPress(buttonEnter)){
		wait1Msec(2000);
		return 2;
	}
	else if(getButtonPress(buttonRight)){
			wait1Msec(2000);
		return 3;
	}
	else{
		displayTextLine(6,"Wrong button input, try again");
			wait1Msec(2000);
	}

	return -1;
}

void hitWall()
{
	motor[motorA] = motor[motorD] = 25;
	while(!SensorValue[S2] == 1) //until touch sensor is activated
	{}
	motor[motorA] = motor[motorD] = 0;
}

//Drives forwards until hits wall
float driveToWall() 
{
	float distanceToWall = 0;
	hitWall();
	distanceToWall = (abs(nMotorEncoder[motorA])*CONVERSION_FACTOR); //measure distance from origin to wall
	return distanceToWall;
}

//spins a given number of degrees
void spinXDegrees(int numDegrees, float factorA, float factorB) 
{
	resetGyro(S4);
	wait1Msec(50);
	motor[motorA] = 25*factorA;
	motor[motorD] = 25*factorB;
	while(abs(getGyroDegrees(S4)) < numDegrees)
	{}
	motor[motorA] = motor[motorD] = 0;
}

//reverses away from the wall and spins towards the bin
void awayFromWall() 
{
	nMotorEncoder[motorA] = 0;
	motor[motorA] = motor[motorD] = -25;
	while ((abs(nMotorEncoder[motorA])*CONVERSION_FACTOR)< lengthOfBin)
	{}
	motor[motorA] = motor[motorD] = 0;
	wait1Msec(200);
	spinXDegrees(85, -1, 1);
}

//check if colour sensor detected the desired colour
bool checkBinColor(s) 
{
	while(true)
		{
		if((getColorName(S1)==colorRed) && (binColor == 1))
		{
			return true;
		}
		else if((getColorName(S1)==colorGreen) && (binColor == 3))
		{
			return true;
		}
		else if((getColorName(S1)==colorBlue) && (binColor == 2))
		{
			return true;
		}
	}
	return false;
}

//drives towards the bin until correct colour has been detected and spins towards bin
float driveToBin(int binColor) 
{
	nMotorEncoder[motorD] = 0;

	motor[motorA] = 25;
	motor[motorD] = 26;
	if(checkBinColor(binColor)== true)
	{
		motor[motorA] = motor[motorD] = 0;
	}
	float distanceToBin = (abs(nMotorEncoder[motorA])*CONVERSION_FACTOR);
	spinXDegrees(90, 1, -1);
	return distanceToBin;
}

//tilts platform down to dump trash
void tipPlatform() 
{
	hitWall();
	motor[motorC] = 10; //tilts down
	wait1Msec(2000);
	motor[motorC] = 0; //waits for garbage to fall
	wait1Msec(2000);
	motor[motorC] = -10; //returns platform to rest position
	wait1Msec(2000);
	motor[motorC] = 0;
}

//reverse away from the bin
void reverse(float distanceToWall) 
{
	motor[motorA] = motor[motorD] = -25;
	nMotorEncoder[motorA] = 0;
	while ((abs(nMotorEncoder[motorA])*CONVERSION_FACTOR)< (distanceToWall-lengthOfBin)) //reverse until in line with start position of robot
	{}
	motor[motorA] = motor[motorD] = 0;
}

//returns robot to the starting position
float returnToStart(float distanceToBin, int binColour) 
{
	spinXDegrees(90, 1, -1);
	nMotorEncoder[motorD] =0;
	motor[motorA] = motor[motorD] = 25;
	//continue moving until the distance
	while(abs(nMotorEncoder[motorD])*CONVERSION_FACTOR < distanceToBin+ROBOT_LENGTH)
	{}
	// Stop the motors after the correct distance is reached
	motor[motorA] = motor[motorD] = 0;
	spinXDegrees(90, -1, 1);
	float time = time1[T1];
  clearTimer(T1);

  // Return the total time taken for the entire trip
  return time;
}

//prints feedback questions and waits for button feedback
void askForFeedback(int binColour, bool shutDown) 
{
	displayTextLine(6, "Did the garbage go into"); //ask if the garbage went into the correct bin (based on binColour)
	displayTextLine(7, "correct bin?");
	wait1Msec(20);
	displayTextLine(8, "Press UP for YES, DOWN for NO"); //promt user to press up for yes or down for no
	while(!getButtonPress(buttonAny))
	{}
	if(getButtonPress(buttonUp))
	{
		displayTextLine(9, "SHRIMPY is very happy to help sort your garbage!");
	}
	else
	{
		displayTextLine(9, "SHRIMPY made a mistake and is very sorry.");
	}
	displayTextLine(10, "Would you like to go again?"); //ask if wants to go again, whole code in while loop so we can repeat
	if(getButtonPress(buttonDown))
	{
		shutDown = 1;	//otherwise stop the code
	}
}

//displays time taken to complete entire operation
void displayTime(float timeSpent) 
{
	float time = timeSpent/6000;
	displayTextLine(11, "The robot spent %f seconds", time); //check syntax to make sure the float integration is correct
	wait1Msec(5000);
}

//combines all functions in correct order to run complete program
void runProgram(int binColor, float distanceToWall, float disToBin, bool shutDown, float timeSpent) 
{
		sensorSetUp();
		giveInstructions();
		binColor = getBinColour();
		distanceToWall = driveToWall();
		awayFromWall();
		disToBin = driveToBin(binColor);
		tipPlatform();
		reverse(distanceToWall); //make from measured ndistance
		timeSpent = returnToStart(disToBin, binColor);
		askForFeedback(binColor, shutDown);
		displayTime(timeSpent);
}


task main() 
{
	int binColor = 0; //check to make sure valid type
	float timeSpent = 0, distanceToWall = 0, distanceToBin = 0;
	int numDegrees = 90;
	bool colourFound = 0, shutDown = 0;
	while(!shutDown)
	{
		runProgram(binColor, distanceToWall, distanceToBin, shutDown, timeSpent);
	}
}
