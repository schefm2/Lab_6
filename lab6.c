/*  
	Names: Sydney Bahs, Tom Saad, Matthew Scheffer
    Section: 2
    Date: 12/11/17
    File name: lab6.c
    Description: The following program controls a gondola's steering mechanism
    while it is mounted on a turntable. Direction that the gondola is pointing
    in is monitored by an electronic compass on the I2C Bus. When there is a 
    difference in current and desired compass heading, pulse width modulation
    is performed using a PCA counter and interrupt service routine to adjust
    the left and right fans' thrusts to correct the difference. Capture/Compare 
    modules 1, 2, and 3 are used to control the thrust angle, left and right 
    fan thrusts, respectively. Heading error is stored in memory before the 
    next heading error calculation takes place. Current heading error is 
    multiplied by a user-set proportional gain and added to a pulse width that 
    sets the thrusts fan to neutral. The difference between the current heading 
    error and previous error is multiplied by a user-set differential gain and 
    added to the neutral pulse width as well. This functions to dampen the 
    natural oscillation of the system when it is run on solely proportional 
    control. Proportional gain, differential gain, and desired heading are 
    input by the user at the start of the program, either through the LCD 
    keypad or by SecureCRT terminal. An ultrasonic ranger is added to the I2C 
    Bus for additional user control of the desired heading. At a detected range 
    of 50 cm, the original-input desired heading will remain unchanged. 
    Increasing the detected range will result in the desired heading increasing 
    until a range of 90 cm or above is detected, at which point the desired 
    heading is changed to 180 degrees + original heading. Likewise occurs in 
    the opposite direction when decreasing the detected range until saturation 
    at 10 cm, at which point heading is changed to 
    original heading - 180 degrees. The program monitors battery voltage by 
    routinely performing an A/D conversion using the ADC1 on the C8051. Both 
    the desired and current heading, the heading error, the range, the 
    battery voltage, and motor pulse width and thrust angle pulse widths are 
    routinely printed to the SecureCRT terminal for later analysis. Once the 
    Motor pulse width for the thrust fan is calculated, it is inverted for one 
    of the thrust fans so that the fans will create thrust in antiparallel 
    directions, inducing rotational movement. This is accomplished by finding 
    the difference between the thrust motor pulse width and its neutral value, 
    then subtracting this difference from the neutral pulse width value before 
    assigning it to the other thrust motor. Thrust angle is adjusted once at 
    the beginning of the program, after gains have been set.
*/
#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>

#define RANGER_ADDR 0xE0
#define COMPASS_ADDR 0xC0
#define PING_CM 0x51
#define PCA_START 28672     //PCA0 value for a pulse of ~20 ms

//Left and right pulse widths set so that Servo isn't strained
#define SERVO_LEFT_PW 2425
#define SERVO_CENTER_PW 3085
#define SERVO_RIGHT_PW 3245

#define MOTOR_REVERSE_PW 2027 
#define MOTOR_NEUTRAL_PW 2765
#define MOTOR_FORWARD_PW 3502

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

//Initialization functions
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init(void);
void Interrupt_Init(void);
void ADC_Init();
void SMB_Init();

//Low Level Functions
void Read_Compass(void);
void Read_Ranger(void);
void Set_Desired_Heading(void);
void Set_Motor_PWM(void);
void Pause(void);
void Wait(void);
unsigned int pow(unsigned int a, unsigned char b);
unsigned int calibrate(void);
unsigned int Calculate_Voltage(void);
unsigned char parallel_input(void);
unsigned char read_AD_input(unsigned char pin_number);

//High Level Functions
void Start_Parameters(void);
void Calibrate_Angle(void);
void Calibrate_Fans(void);
void Set_Motion(void);
void Print_Data(void);
void Read_Print(void);

//Global Variables
unsigned char Data[5];	//Data array used to read and write to I2C Bus slaves
signed long Motor_PW;
unsigned int current_heading, range, battery_voltage, Servo_PW;
unsigned char keyboard, keypad, r_count, c_count, print_count, wait_count;
signed int desired_heading, heading_error, previous_error = 0;
__xdata unsigned int kp = 0, kd = 0, original_heading;    //Slow write, fast read memory for gains

__bit servo_stop, motor_stop, compass_flag, ranger_flag, print_flag; //flags

//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
    //Initialize board
    Sys_Init();
    putchar(' '); //The quotes in this line may not format correctly
    Port_Init();
    XBR0_Init();
    Interrupt_Init();
    PCA_Init();
    SMB_Init();
    ADC_Init();	//Must come after PCA_Init to allow capacitors to charge

    Start_Parameters();	//Set gains and initial desired heading
    Calibrate_Angle();  //Set the thrust angle of the gondola
    Calibrate_Fans(); //Useful for diagnosing problems with the fans
    
	//Reset time/logic-keeping variables
	compass_flag = ranger_flag = print_flag = 0;
    r_count = c_count = print_count = 0;
    
	
    while(1)
    {
        
        battery_voltage = Calculate_Voltage();  //Constantly updates the value of battery_voltage
        Set_Motion();	//Reads compass/ranger and sets the fan pulse widths
        Print_Data();	//Prints data required for plotting control algorithm performance
        //Read_Print();   //Useful for testing if the compass and ranger function properly
        
    }
}

//HIGH LEVEL FUNCTIONS

//----------------------------------------------------------------------------
// Start_Parameters
//----------------------------------------------------------------------------
//
// Allows the user to set a proportional, differential gain for the gondola;
// additionally lets the user set an original desired heading.
//
void Start_Parameters(void)
{
    //Start with neutral pulse widths for motor and thrust angle
    Servo_PW = SERVO_CENTER_PW;
    Motor_PW = MOTOR_NEUTRAL_PW;
    PCA0CP1 = 0xFFFF - Servo_PW;            //Set thrust angle to neutral
    PCA0CP2 = PCA0CP3 = 0xFFFF - Motor_PW;  //Sets both fans to neutral

    Wait();         //Wait for 1 second
    lcd_clear();    //clear lcd screen
    lcd_print("Calibration:\nHello world!\n012_345_678:\nabc def ghij");	
	Wait();         //wait 1 second
	
	do
	{
		lcd_clear(); //clear screen
		lcd_print("Enter prop gain:\n"); //print instructions
		printf("\r\nSelect a proportional gain (0 to 50000). Press # to confirm.\r\n");
		kp = calibrate();	//take input
		Wait(); //wait a second
	}
	while (kp > 50000); //wait until you get appropriate gain
	printf("\r\nYou selected %u as your proportional gain", kp); //print prop. gain
    lcd_print("\nFinal value above");  
    Wait();
    
    do
	{
		lcd_clear(); //clear screen
		lcd_print("Enter diff gain:\n"); //print instructions
		printf("\r\nSelect a differential gain (0 to 50000). Press # to confirm.\r\n");
		kd = calibrate();	//take input
		Wait(); //wait a second
	}
	while (kd > 50000); //wait until you get appropriate gain
	printf("\r\nYou selected %u as your differential gain", kd); //print diff. gain
    lcd_print("\nFinal value above");
    
    do
	{
		lcd_clear(); //clear screen
		lcd_print("Enter heading:\n"); //print instructions
		printf("\r\nSelect a desired heading (0 to 3599). Press # to confirm.\r\n");
		original_heading = calibrate();	//take input
		Wait(); //wait a second
	}
	while (original_heading > 3599); //wait until you get appropriate heading
	printf("\r\nYou selected %u as your desired heading", original_heading); //print desired heading
    lcd_print("\nFinal value above");  
    Wait();
    
    Wait();
    lcd_clear();
}

//----------------------------------------------------------------------------
//Set_Motion
//----------------------------------------------------------------------------
//
// Used to prevent main function from appearing cluttered.
//
void Set_Motion(void)
{
	//read sensors and set pulse widths
    Read_Compass();
    Read_Ranger();
    Set_Motor_PWM();
}

//----------------------------------------------------------------------------
//Read_Print
//----------------------------------------------------------------------------
//
// Used to allow the car to continue making readings to print those readings
// while it is in non-normal operating conditions (i.e. encountering obstacle).
//
void Read_Print(void)
{
    Read_Compass();
    Read_Ranger();
    Print_Data();
}


//----------------------------------------------------------------------------
//Print_Data
//----------------------------------------------------------------------------
void Print_Data(void)
{
    if(print_flag)
		//Only prints every ~100 ms
    {
        printf("\r\n%d, %u, %d, %u, %u, %ld, %u", desired_heading, current_heading, heading_error, range, battery_voltage, Motor_PW, Servo_PW);
        lcd_clear();
        lcd_print("Error: %d\nRange: %u\nMotor: %u\nServo: %u", heading_error, range, Motor_PW, Servo_PW);
        
        print_flag = 0;     //reset flag and counter for printing to ensure 100 ms breaks
        print_count = 0;    //between prints
    }
}

//LOW LEVEL FUNCTIONS

//----------------------------------------------------------------------------
//Read_Compass
//----------------------------------------------------------------------------
//
// Note: this function performs heading_error calculations instead of Set_Servo_PWM
// to allow heading_error to update during non-normal operating conditions
// (i.e. encountering obstacle).
//
void Read_Compass(void)
{
    if (compass_flag)
        //Trigger every 40 ms
    {
        i2c_read_data(COMPASS_ADDR, 2, Data, 2);	//Read two byte, starting at reg 2
        current_heading =(((unsigned int)Data[0] << 8) | Data[1]); //Combine the two values
        //Heading has units of tenths of a degree
		
        //Stores last heading_error before a new heading_error is calculated
        previous_error = heading_error;
        
        Set_Desired_Heading();                  //Comment out to remove ranger function
        //desired_heading = original_heading;   //Uncomment for ranger function
        
		heading_error = (signed int)desired_heading - current_heading;
		//heading_error is now between -3599 and 3599

		//If the error is greater abs(1800) degree-tenths, then error is set to 
		//explementary angle of original error
        heading_error =
            (heading_error > 1800) ? heading_error - 3599 :
            (heading_error < -1800) ? 3599 + heading_error :
            heading_error;
        
        compass_flag = 0;   //Reset compass flag
        c_count = 0;        //Reset compass timer count
    }
}

//----------------------------------------------------------------------------
//Read_Ranger
//----------------------------------------------------------------------------
void Read_Ranger(void)
{
    if (ranger_flag)
        //Trigger every 80 ms
    {
        i2c_read_data(RANGER_ADDR, 2, Data, 2);
        range = (((unsigned int)Data[0] << 8) | Data[1]);
        //Overwrites prev data and updates range
        Data[0] = PING_CM;
        i2c_write_data (RANGER_ADDR, 0, Data, 1 );
        
        ranger_flag = 0;    //Reset Ranger flag
        r_count = 0; //r_count reset here to give ranger time to receive its ping
    }
}

//----------------------------------------------------------------------------
//Set_Desired_Heading
//----------------------------------------------------------------------------
//
// Uses a range detected with the ultrasonic ranger to modify desired_heading.
// Dead band is 48 to 52 cm, and changes of +/- 180 degrees to the 
// original_heading are possible. These changes saturate below 10 cm or above 90 cm.
// Note: the compass is mounted upside-down when the gondola is on the turntable, 
// meaning that an increased desired_heading results in a counterclockwise turn.
//
void Set_Desired_Heading(void)
{
    //Ensure that ranges greater than 90 or less than 10 are saturated
    range =
        (range < 10) ? 10 :
        (range > 90) ? 90 :
        range;

    //Low ranges increase the original_heading, high ranges decrease it
    if (range < 48) {desired_heading = original_heading + ((float)(48 - range)/38)*1800;}
    else if (range > 52) {desired_heading = original_heading - ((float)(range-52)/38)*1800;}
    else {desired_heading = original_heading;}  //When in deadband, desired_heading is unchanged

    //Prevents desired_heading from falling outside the compass's range of possible values
    desired_heading =
        (desired_heading<0) ? desired_heading+3599 :
        (desired_heading>3599) ? desired_heading-3599 :
        desired_heading;
}

//----------------------------------------------------------------------------
//Set_Motor_PWM
//----------------------------------------------------------------------------
//
// Sets the left fan motor pulse width and sets the right fan motor pulse width 
// based on the left fan. Uses PD control algorithm.
//
void Set_Motor_PWM(void)
{
    //Equation from worksheet_11.c that was most reliable for calculating pulse width
    //Changed to use float casting so that float gains (after dividing by 50) are achievable
    Motor_PW = (float)MOTOR_NEUTRAL_PW + (float)kp/50*(float)heading_error + (float)kd/50*(float)(heading_error-previous_error);
	
    //Keeps the pulse widths from straining the motor
    Motor_PW = (Motor_PW > MOTOR_FORWARD_PW) ? MOTOR_FORWARD_PW : Motor_PW;
    Motor_PW = (Motor_PW < MOTOR_REVERSE_PW) ? MOTOR_REVERSE_PW : Motor_PW;
    
    //Set left, then right fans to opposite values
    PCA0CP2 = 0xFFFF - Motor_PW;
    Pause();    //Put this in here to stop the gondola from only spinning one fan
    PCA0CP3 = 0xFFFF - (2*MOTOR_NEUTRAL_PW - Motor_PW); //Condensed form of Neutral - (Motor - Neutral)
    
}

//----------------------------------------------------------------------------
//Pause
//----------------------------------------------------------------------------
void Pause(void)
{   
	//Stop for 40 ms
	wait_count = 0;
	while (wait_count < 2){}
    
}

//----------------------------------------------------------------------------
//Wait
//----------------------------------------------------------------------------
void Wait(void)
{   
	//Stop for 1000 ms
	wait_count = 0;
	while (wait_count < 50){}
    
}

//----------------------------------------------------------------------------
//pow
//----------------------------------------------------------------------------
//
// Stripped back version of math.h power function, used in calibrate(). 
// Raises a to the power of b.
//
unsigned int pow(unsigned int a, unsigned char b)
{
    unsigned char i;
    unsigned char base = a;

    if (b == 0) return 1;
    for(i = 1; i < b; i++)
        a = a*base;
    return a;
}

//----------------------------------------------------------------------------
//calibrate
//----------------------------------------------------------------------------
unsigned int calibrate(void)
{
	unsigned char isPress = 0;
	unsigned char pressCheck = 0;
	unsigned int value = 0;	//Final value to be returned
	
    //Clears the Data array to prevent leftover data from interfering
    while (pressCheck < 5) {
        Data[pressCheck++] = 0;
    }
    pressCheck = 0; //Reset pressCheck for normal use

	while(1)
	{
		keyboard = getchar_nw();	//This constantly sets keyboard to whatever char is in the terminal
		keypad = read_keypad();		//This constantly sets the keypad to whatever char is on the LCD
		Pause();					//Pause necessary to prevent overreading the keypad

		if (keyboard == '#' || keypad == '#') //# is a confirm key, so it will finish calibrate()
        {
            for (pressCheck = 0; 0 < isPress; isPress--)
                //Highest sig. digit is multiplied by 10 raised to power determined
                //by the total number of digits; walks through data array while
                //decrementing this power by one 
            {
                value += Data[pressCheck++]*pow(10,isPress - 1);
            }
            //Returns joined decimal value of digits in Data array
			return value;
        }

		if (isPress > pressCheck && keypad == 0xFF && keyboard == 0xFF)	//Only increments pressCheck if held key is released
			pressCheck++;
		

		if (pressCheck == 6)	//If a 6th key is pressed, then released
		{
            //Clears the Data array
            for (pressCheck = 0;pressCheck < 5;pressCheck++)
                Data[pressCheck] = 0;
            
			isPress = pressCheck = 0;	//Reset the flags
			lcd_print("\b\b\b\b\b\b");	//Clear value displayed on LCD, needs an extra \b for some reason?
			printf("\r      \r");	//Clear value displayed on terminal
			
		}

		
		if (isPress == pressCheck)	//pressCheck must be equal to isPress, only occurs if no key is held down
		{
			if (keypad != 0xFF)		//When an actual key is held down
			{
				lcd_print("%c",keypad);	//Adds pressed key to LCD screen
				printf("%c", keypad);	//Adds pressed key to computer terminal
				Data[isPress] = ((unsigned int)(keypad - '0')); //Converts char val. to int val. and stores in Data
				isPress++;
			}
			if (keyboard != 0xFF)	//When an actual key is held down
			{
				lcd_print("%c",keyboard);	//Adds pressed key to LCD screen
				//printf("%c", keyboard); this line is not necessary as getchar_nw automatically executes a putchar()
				Data[isPress] = ((unsigned int)(keyboard - '0'));   //Converts char val. to int val. and stores in Data
				isPress++;	
			}
		}
	}
}

//----------------------------------------------------------------------------
//parallel_input
//----------------------------------------------------------------------------
//
// Function designed to take input from either the keyboard or keypad; must be
// called multiple times in a while loop until desired value is input.
//
unsigned char parallel_input(void)
{
    unsigned char keypad;
    unsigned char keyboard;

	keyboard = getchar_nw();	//This constantly sets keyboard to whatever char is in the terminal
	keypad = read_keypad();		//This constantly sets the keypad to whatever char is on the LCD
	Pause();					//Pause necessary to prevent overreading the keypad

	//Returns the value of the respective input that has a key pressed
	if (keyboard != 0xFF)
		return keyboard;
	if (keypad != 0xFF)
		return keypad;
	else
		return 0;	//Return 0 if no key is pressed
}

//----------------------------------------------------------------------------
//read_AD_input
//----------------------------------------------------------------------------
unsigned char read_AD_input(unsigned char pin_number)
{
    AMX1SL = pin_number;		//Sets multiplexer to convert correct pin
    ADC1CN &= ~0x20;			//Clears the A/D conversion complete bit
    ADC1CN |= 0x10;				//Starts A/D conversion
    while(!(ADC1CN & 0x20));	//Waits until conversion completes 
    return ADC1;				//returns converted input, 0-255 inclusive
}

//----------------------------------------------------------------------------
//ADC_Init
//----------------------------------------------------------------------------
void ADC_Init(void)
{
    REF0CN = 0x03;	//Sets V_ref as 2.4V
    ADC1CN = 0x80;	//Enables AD/C converter

    //Gives capacitors in A/D converter time to charge
    r_count = 0;
    while(r_count < 6);

    //Sets gain to 1
    ADC1CF |= 0x01;
    ADC1CF &= 0xFD;
}

//-----------------------------------------------------------------------------
//Port_Init
//-----------------------------------------------------------------------------
void Port_Init()
{
	P0MDOUT |= 0xF0;	//Set output pin for CEX0 to CEX3 in push-pull mode for P0.4 to P0.7
	
    P1MDOUT &= ~0x08;	//Set battery voltage pin (P1.3) to open drain
	P1 |= 0x08;			//Set impedance high on P1.3
	P1MDIN &= ~0x08;	//Set P1.3 to analog input
}

//-----------------------------------------------------------------------------
// Interrupt_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Interrupt_Init(void)
{
    // IE and EIE1
    EA = 1;
    EIE1 |= 0x08;
}

//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar.
//
void XBR0_Init(void)
{
    XBR0 = 0x25;	//configure crossbar with UART, SPI, SMBus, and CEX channels as
					//in worksheet
}

//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array.
//
void PCA_Init(void)
{
    PCA0MD = 0x81;  //Use a 16 bit counter with SYSCLK/12.
    PCA0CPM0 = PCA0CPM1 = PCA0CPM2 = PCA0CPM3 = 0xC2;   //Sets CCM0 thru CCM3 in 16-bit compare mode, enables PWM
    PCA0CN = 0x40; //Enable PCA counter
}

//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt.
//
void PCA_ISR ( void ) __interrupt 9
{
    if(CF)
    {
        CF=0; //clear flag
        PCA0 = PCA_START;//determine period to 20 ms
        //Increment counts
        r_count++;
		c_count++;
        print_count++;
        wait_count++;
        
        ranger_flag = (r_count > 4) ? 1 : ranger_flag ; //Ranger flag raised every ~80 ms
        compass_flag = (c_count > 2) ? 1 : compass_flag ;   //Compass flag raised every ~40 ms
        print_flag = (print_count > 5) ? 1 : print_flag ; //print flag raised every ~100 ms
    }
    PCA0CN &= 0x40; //Handle other interupt sources
}

//-----------------------------------------------------------------------------
// SMB_Init
//-----------------------------------------------------------------------------
//
// Set up the I2C Bus.
//
void SMB_Init()
{
    SMB0CR = 0x93;	//Sets SCL to 100 kHz (actually ~94594 Hz)
    ENSMB = 1;		//Enables SMB
}

//-----------------------------------------------------------------------------
// Calibrate_Angle
//-----------------------------------------------------------------------------
//
// Allows the user to manually set the thrust angle of the gondola.
//
void Calibrate_Angle(void)
{
    unsigned char temp = 0; //Used for storing the pressed key
    printf("\r\nPlease press l and r to rotate the thrust angle of the gondola.\r\nPress q to confirm the angle.\r\n");
    while (temp != 'q')
        //Runs until the user presses the confirm key 'q'
    {
        temp = getchar();
        if (temp == 'l')
            Servo_PW -= 10;
        if (temp == 'r')
            Servo_PW += 10;
        
        //Prevent servo from straining pulse widths
        Servo_PW =
            (Servo_PW < SERVO_LEFT_PW) ? SERVO_LEFT_PW :
            (Servo_PW > SERVO_RIGHT_PW) ? SERVO_RIGHT_PW :
            Servo_PW;
        
        PCA0CP1 = 0xFFFF - Servo_PW;    //Sets the modified pulse width
    }
    printf("\r\nYour final pulse width was: %u", Servo_PW);
}

//-----------------------------------------------------------------------------
// Calibrate_Fans
//-----------------------------------------------------------------------------
//
// Allows the user to manually ramp up or ramp down the thrust of the fans. 
// Particularly useful for debugging involving fans that seem to not work.
//
void Calibrate_Fans(void)
{
    unsigned char temp = 0; //Used for storing the pressed key
    printf("\r\nPlease press l and r to change the motor PW.\r\nPress q to confirm the PW.\r\n");
    while (temp != 'q')
    {
        temp = getchar();
        if (temp == 'l')
            Motor_PW -= 10;
        if (temp == 'r')
            Motor_PW += 10;
        
        //Prevent servo from straining pulse widths
        Motor_PW =
            (Motor_PW < MOTOR_REVERSE_PW) ? MOTOR_REVERSE_PW :
            (Motor_PW > MOTOR_FORWARD_PW) ? MOTOR_FORWARD_PW :
            Motor_PW;

        //Set left, then right fans to opposite values
        PCA0CP2 = 0xFFFF - Motor_PW;
        PCA0CP3 = 0xFFFF - (2*MOTOR_NEUTRAL_PW - Motor_PW); //Condensed form of Neutral - (Motor - Neutral)
        printf("\r\nThis is the CCM3: %u", PCA0CP3);    //Printed to ensure that CCM3 was properly set
    }
    printf("\r\nYour final pulse width was: %u", Motor_PW);
}


//-----------------------------------------------------------------------------
// Calculate_Voltage
//-----------------------------------------------------------------------------
//
// Returns the voltage of the battery in mV.
//
unsigned int Calculate_Voltage(void)
{
    return ((float)read_AD_input(3) / 255) * 7400;
}

//-----------------------------------------------------------------------------



