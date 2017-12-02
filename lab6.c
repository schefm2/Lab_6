// LAB 4 CODE - EDIT FOR LAB 6
/*  
	Names: Sydney Bahs, Tom Saad, Matthew Scheffer
    Section: 2
    Date: 12/11/17
    File name: lab6.c
    Description: 
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
#define SERVO_CENTER_PW 2895
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
void Set_Servo_PWM(void);
void Set_Motor_PWM(void);
void Pause(void);
void Wait(void);
unsigned int pow(unsigned int a, unsigned char b);
unsigned int calibrate(void);
unsigned char parallel_input(void);
unsigned char read_AD_input(unsigned char pin_number);

//High Level Functions
void Start_Parameters(void);
void Set_Motion(void);
void Set_Neutral(void);
void Print_Data(void);
void Read_Print(void);

//Global Variables
unsigned char Data[5];	//Data array used to read and write to I2C Bus slaves
unsigned int desired_heading, current_heading, range, Servo_PW, Motor_PW;
unsigned char keyboard, keypad, r_count, c_count, print_count, wait_count;
signed int heading_error;
__xdata unsigned int kp = 0, kd = 0;    //Slow write, fast read memory for gains

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

    Car_Parameters();	//Sets gain, desired_heading, and initial_speed
    
	//Reset time/logic-keeping variables
	r_count = 0;
	print_count = 0;
    first_obstacle =0;
	
    while(1)
    {

        Set_Motion();	//Reads compass/ranger and sets their respective PW's
        Set_Neutral();	//If the slide switch is ON, the car is in neutral and steering is centered
        Print_Data();	//Prints data required for plotting control algorithm performance

        if ( range <= 50 && time >= 20 && first_obstacle == 0)
            //Detected something at/closer than 50, stop
        {
			Motor_PW = MOTOR_NEUTRAL_PW;
			PCA0CP2 = 0xFFFF - Motor_PW;	//Set car in neutral
            printf("Press 4 for left or 6 for right.\n\rPress space to return to normal path.\n\r");
            while((answer != '4') && (answer != '6'))
			{
				answer=parallel_input();	//Reads the terminal and LCD keypad for a char
				Read_Print();	//Prints data required for plotting control algorithm performance 
								//& reads compass/ranger
			}

            if(answer=='4')
            {
				Servo_PW = SERVO_LEFT_PW;	//Set steering hard left
            }
            if(answer=='6')
            {
				Servo_PW = SERVO_RIGHT_PW;	//Set steering hard right
            }
            PCA0CP0 = 0xFFFF - Servo_PW;	//Outside previous two if's to prevent redundancy
            Motor_PW = initial_speed;
            PCA0CP2 = 0xFFFF - Motor_PW;	//Resume forward motion
            while(getchar_nw() != ' ')
				//Waits for a space input on keyboard
            {
                Read_Print();	//Keeps printing and reading devices while waiting
            }
            answer = '0';	//Reset answer
            first_obstacle++;	//First obstacle has been passed
        }
        while ( range <= 35 && first_obstacle > 0)
			//Car has passed first obstacle and second obstacle is <35 cm away
        {
			Motor_PW=MOTOR_NEUTRAL_PW;
        	PCA0CP2 = 0xFFFF - Motor_PW;	//Set car to neutral
			Read_Print();	//Print and read data while stopped
        }
    }
}

//HIGH LEVEL FUNCTIONS

//----------------------------------------------------------------------------
// Start_Parameters
//----------------------------------------------------------------------------
// Allows the user to set a proportional and differential gain for the gondola.
//
void Start_Parameters(void)
{
    Servo_PW = SERVO_CENTER_PW;		//Initialize car to straight steering and no movement
    Motor_PW = MOTOR_NEUTRAL_PW;	//Set pulse to stop car
    PCA0CP0 = 0xFFFF - Servo_PW;	//tell hardware to use new servo pulse width
    PCA0CP2 = 0xFFFF - Motor_PW;	//tell hardware to use new motor pulse width

    Wait();         //Wait for 1 second
    lcd_clear();    //clear lcd screen
    lcd_print("Calibration:\nHello world!\n012_345_678:\nabc def ghij");	
	Wait();         //wait 1 second
	
	do
	{
		lcd_clear(); //clear screen
		lcd_print("Enter prop gain:\n"); //print instructions
		printf("\r\nSelect a proportional gain (0 to 1000). Press # to confirm.\r\n");
		kp = calibrate();	//take input
		Wait(); //wait a second
	}
	while (kp > 1000); //wait until you get appropriate gain
	printf("\r\nYou selected %u as your proportional gain", kp); //print steering gain
    lcd_print("\nFinal value above");  
    Wait();
    
    do
	{
		lcd_clear(); //clear screen
		lcd_print("Enter diff gain:\n"); //print instructions
		printf("\r\nSelect a differential gain (0 to 1000). Press # to confirm.\r\n");
		kd = calibrate();	//take 5 digit input
		Wait(); //wait a second
	}
	while (kd > 1000); //wait until you get appropriate gain
	printf("\r\nYou selected %u as your differential gain", kd); //print integral gain
    lcd_print("\nFinal value above");  
    Wait();
}

//----------------------------------------------------------------------------
//Set_Motion
//----------------------------------------------------------------------------
void Set_Motion(void)
{
	//read sensors and set pulse widths
    Read_Compass();
    Read_Ranger();
    Set_Servo_PWM();
    Set_Motor_PWM();
}

//----------------------------------------------------------------------------
//Set_Neutral
//----------------------------------------------------------------------------
void Set_Neutral(void)
{
	//set servo to center and stop motor
    if (SS)
    {
		PCA0CP0 = 0xFFFF - SERVO_CENTER_PW;
		PCA0CP2 = 0xFFFF - MOTOR_NEUTRAL_PW;

        while(SS) {}	//wait until slideswitch is turned OFF
    }
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
    if(print_count > 20)
		//Only prints ever ~400 ms
    {
		time += print_count/5;	//Ensures accurate time readings
        print_count = 0;
        printf("\r\n%u,%d,%u,%u", (int)time, heading_error, Servo_PW, range);
        lcd_clear();
        lcd_print("Heading is: %u\nRange is: %u\nServo Cycle: %u\nMotor Cycle: %u", current_heading, range, (int)(((float)Servo_PW/28672)*100), (int)(((float)Motor_PW/28672)*100));
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
    if (!(r_count % 2) && r_count != 0)
        //Trigger every 40 ms
    {
        i2c_read_data(COMPASS_ADDR, 2, Data, 2);	//Read two byte, starting at reg 2
        current_heading =(((unsigned int)Data[0] << 8) | Data[1]); //Combine the two values
        //Heading has units of tenths of a degree
		
		heading_error = (signed int)desired_heading - current_heading;
		//heading_error is now between -3599 and 3599
		
		//If the error is greater abs(1800) degree-tenths, then error is set to 
		//explementary angle of original error
		if (heading_error > 1800)
			heading_error = heading_error - 3599;
		if (heading_error < -1800)
			heading_error = 3599 + heading_error;
    }
}

//----------------------------------------------------------------------------
//Read_Ranger
//----------------------------------------------------------------------------
void Read_Ranger(void)
{
    if (r_count > 4)
        //Trigger every 80 ms
    {
		r_count = 0;	//r_count reset here to give ranger time to receive its ping
        i2c_read_data(RANGER_ADDR, 2, Data, 2);
        range = (((unsigned int)Data[0] << 8) | Data[1]);
        //Overwrites prev data and updates range
        Data[0] = PING_CM;
        i2c_write_data (RANGER_ADDR, 0, Data, 1 );
    }
}

//----------------------------------------------------------------------------
//Set_Servo_PWM
//----------------------------------------------------------------------------
void Set_Servo_PWM(void)
{
	//Servo_PW set to value based on heading_error modified by gain set in Car_Parameters()
	Servo_PW = gain*(heading_error) + SERVO_CENTER_PW;

    //Additional precaution: if Servo_PW somehow exceeds the limits set in Lab 3-1,
    //then Servo_PW is set to corresponding endpoint of PW range [SERVO_LEFT_PW, SERVO_RIGHT_PW]
	if (Servo_PW > SERVO_RIGHT_PW) Servo_PW = SERVO_RIGHT_PW;
	if (Servo_PW < SERVO_LEFT_PW) Servo_PW = SERVO_LEFT_PW;
	PCA0CP0 = 0xFFFF - Servo_PW;
}

//----------------------------------------------------------------------------
//Set_Motor_PWM
//----------------------------------------------------------------------------
void Set_Motor_PWM(void)
{
	//When car is not in neutral, it runs at speed set in Car_Parameters() at
	//beginning of program
	Motor_PW = initial_speed;
	PCA0CP2 = 0xFFFF - Motor_PW;
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
//Pow
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
	
    for (;pressCheck < 5;pressCheck++)
        Data[pressCheck] = 0;
    pressCheck = 0;
	while(1)
	{
		keyboard = getchar_nw();	//This constantly sets keyboard to whatever char is in the terminal
		keypad = read_keypad();		//This constantly sets the keypad to whatever char is on the LCD
		Pause();					//Pause necessary to prevent overreading the keypad

		if (keyboard == '#' || keypad == '#') //# is a confirm key, so it will finish calibrate()
        {
            for (pressCheck = 0; 0 < isPress; isPress--)
            {
                value += Data[pressCheck++]*pow(10,isPress - 1);
            }
            if (value > 0xFFFF)    //If the gain is set too high, set to saturation for the unsigned char gains
                return 0xFFFF;
			return value;	
        }

		if (isPress > pressCheck && keypad == 0xFF && keyboard == 0xFF)	//Only increments pressCheck if held key is released
			pressCheck++;
		

		if (pressCheck == 6)	//If a 6th key is pressed, then released
		{
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
				Data[isPress] = ((unsigned int)(keypad - '0'));	
				isPress++;
			}
			if (keyboard != 0xFF)	//When an actual key is held down
			{
				lcd_print("%c",keyboard);	//Adds pressed key to LCD screen
				//printf("%c", keyboard); this line is not necessary as getchar_nw automatically executes a putchar()
				Data[isPress] = ((unsigned int)(keyboard - '0'));
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
	//Initailize POT
	P1MDOUT |= 0x05;	//Set output pin for CEX0 and CEX2 in push-pull mode
	P1MDOUT &= ~0x80;	//Set potentiometer pin (P1.7) to open drain
	P1 |= 0x80;			//Set impedance high on P1.7
	P1MDIN &= ~0x80;	//Set P1.7 to analog input
	
	P3MDOUT &= ~0x80; //Pin 3.7 open drain
	P3 |= 0x80; //Pin 3.7 high impedance
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
// Set up the crossbar
//
void XBR0_Init(void)
{
    XBR0 = 0x27;	//configure crossbar with UART, SPI, SMBus, and CEX channels as
					//in worksheet
}

//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
    // reference to the sample code in Example 4.5 - Pulse Width Modulation implemented using
    // Use a 16 bit counter with SYSCLK/12.
    PCA0MD = 0x81;
    PCA0CPM0 = PCA0CPM2 = 0xC2;		//Sets both CCM0 and CCM2 in 16-bit compare mode, enables PWM
    PCA0CN = 0x40; //Enable PCA counter
}

//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR ( void ) __interrupt 9
{
    if(CF)
    {
        CF=0; //clear flag
        PCA0 = PCA_START;//determine period to 20 ms
        r_count++;
		c_count++;
        print_count++;
        wait_count++;
    }
    PCA0CN &= 0x40; //Handle other interupt sources
    // reference to the sample code in Example 4.5 -Pulse Width Modulation implemented using
}

//-----------------------------------------------------------------------------
// SMB_Init
//-----------------------------------------------------------------------------
//
// Set up the I2C Bus
//
void SMB_Init()
{
    SMB0CR = 0x93;	//Sets SCL to 100 kHz (actually ~94594 Hz)
    ENSMB = 1;		//Enables SMB
}
//-----------------------------------------------------------------------------



