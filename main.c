/********************************************************************
 FileName:     main.c
 Dependencies: See INCLUDES section
 Processor:   PIC18 or PIC24 USB Microcontrollers
 Hardware:    The code is natively intended to be used on the following
        hardware platforms: PICDEM� FS USB Demo Board, 
        PIC18F87J50 FS USB Plug-In Module, or
        Explorer 16 + PIC24 USB PIM.  The firmware may be
        modified for use on other USB platforms by editing the
        HardwareProfile.h file.
 Complier:    Microchip C18 (for PIC18) or C30 (for PIC24)
 Company:   Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the �Company�) for its PIC� Microcontroller is intended and
 supplied to you, the Company�s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
 File Description:

 Change History:
  Rev   Date         Description
  1.0   11/19/2004   Initial release
  2.1   02/26/2007   Updated for simplicity and to use common
                     coding style
********************************************************************/


//	========================	INCLUDES	========================
#ifdef _VISUAL
#include "VisualSpecials.h"
#endif // VISUAL

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"

#include "mtouch.h"

#include "BMA150.h"

#include "oled.h"

#include "soft_start.h"

#include <stdio.h>
#include <stdlib.h>
//	========================	CONFIGURATION	========================

#if defined(PIC18F46J50_PIM)
   //Watchdog Timer Enable bit:
     #pragma config WDTEN = OFF          //WDT disabled (control is placed on SWDTEN bit)
   //PLL Prescaler Selection bits:
     #pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
   //Stack Overflow/Underflow Reset Enable bit:
     #pragma config STVREN = ON            //Reset on stack overflow/underflow enabled
   //Extended Instruction Set Enable bit:
     #pragma config XINST = OFF          //Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
   //CPU System Clock Postscaler:
     #pragma config CPUDIV = OSC1        //No CPU system clock divide
   //Code Protection bit:
     #pragma config CP0 = OFF            //Program memory is not code-protected
   //Oscillator Selection bits:
     #pragma config OSC = ECPLL          //HS oscillator, PLL enabled, HSPLL used by USB
   //Secondary Clock Source T1OSCEN Enforcement:
     #pragma config T1DIG = ON           //Secondary Oscillator clock source may be selected
   //Low-Power Timer1 Oscillator Enable bit:
     #pragma config LPT1OSC = OFF        //Timer1 oscillator configured for higher power operation
   //Fail-Safe Clock Monitor Enable bit:
     #pragma config FCMEN = OFF           //Fail-Safe Clock Monitor disabled
   //Two-Speed Start-up (Internal/External Oscillator Switchover) Control bit:
     #pragma config IESO = OFF           //Two-Speed Start-up disabled
   //Watchdog Timer Postscaler Select bits:
     #pragma config WDTPS = 32768        //1:32768
   //DSWDT Reference Clock Select bit:
     #pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as reference clock
   //RTCC Reference Clock Select bit:
     #pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as reference clock
   //Deep Sleep BOR Enable bit:
     #pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep (does not affect operation in non-Deep Sleep modes)
   //Deep Sleep Watchdog Timer Enable bit:
     #pragma config DSWDTEN = OFF        //Disabled
   //Deep Sleep Watchdog Timer Postscale Select bits:
     #pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
   //IOLOCK One-Way Set Enable bit:
     #pragma config IOL1WAY = OFF        //The IOLOCK bit (PPSCON<0>) can be set and cleared as needed
   //MSSP address mask:
     #pragma config MSSP7B_EN = MSK7     //7 Bit address masking
   //Write Protect Program Flash Pages:
     #pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0
   //Write Protection End Page (valid when WPDIS = 0):
     #pragma config WPEND = PAGE_0       //Write/Erase protect Flash Memory pages starting at page 0 and ending with page WPFP[5:0]
   //Write/Erase Protect Last Page In User Flash bit:
     #pragma config WPCFG = OFF          //Write/Erase Protection of last page Disabled
   //Write Protect Disable bit:
     #pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored
  
#else
    #error No hardware board defined, see "HardwareProfile.h" and __FILE__
#endif



//	========================	Global VARIABLES	========================
#pragma udata
//You can define Global Data Elements here

//	========================	PRIVATE PROTOTYPES	========================
static void InitializeSystem(void);
static void ProcessIO(void);
static void UserInit(void);
static void YourHighPriorityISRCode();
static void YourLowPriorityISRCode();

BOOL CheckButtonPressed(void);

//	========================	VECTOR REMAPPING	========================
#if defined(__18CXX)
  //On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
  //the reset, high priority interrupt, and low priority interrupt
  //vectors.  However, the current Microchip USB bootloader 
  //examples are intended to occupy addresses 0x00-0x7FF or
  //0x00-0xFFF depending on which bootloader is used.  Therefore,
  //the bootloader code remaps these vectors to new locations
  //as indicated below.  This remapping is only necessary if you
  //wish to program the hex file generated from this project with
  //the USB bootloader.  If no bootloader is used, edit the
  //usb_config.h file and comment out the following defines:
  //#define PROGRAMMABLE_WITH_SD_BOOTLOADER
  
  #if defined(PROGRAMMABLE_WITH_SD_BOOTLOADER)
    #define REMAPPED_RESET_VECTOR_ADDRESS     0xA000
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0xA008
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS 0xA018
  #else 
    #define REMAPPED_RESET_VECTOR_ADDRESS     0x00
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0x08
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS 0x18
  #endif
  
  #if defined(PROGRAMMABLE_WITH_SD_BOOTLOADER)
  extern void _startup (void);        // See c018i.c in your C18 compiler dir
  #pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
  void _reset (void)
  {
      _asm goto _startup _endasm
  }
  #endif
  #pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
  void Remapped_High_ISR (void)
  {
       _asm goto YourHighPriorityISRCode _endasm
  }
  #pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
  void Remapped_Low_ISR (void)
  {
       _asm goto YourLowPriorityISRCode _endasm
  }
  
  #if defined(PROGRAMMABLE_WITH_SD_BOOTLOADER)
  //Note: If this project is built while one of the bootloaders has
  //been defined, but then the output hex file is not programmed with
  //the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
  //As a result, if an actual interrupt was enabled and occured, the PC would jump
  //to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
  //executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
  //(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".  This
  //would effective reset the application.
  
  //To fix this situation, we should always deliberately place a 
  //"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
  //"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.  When the output
  //hex file of this project is programmed with the bootloader, these sections do not
  //get bootloaded (as they overlap the bootloader space).  If the output hex file is not
  //programmed using the bootloader, then the below goto instructions do get programmed,
  //and the hex file still works like normal.  The below section is only required to fix this
  //scenario.
  #pragma code HIGH_INTERRUPT_VECTOR = 0x08
  void High_ISR (void)
  {
       _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
  }
  #pragma code LOW_INTERRUPT_VECTOR = 0x18
  void Low_ISR (void)
  {
       _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
  }
  #endif  //end of "#if defined(||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER))"

  #pragma code
  
//	========================	Application Interrupt Service Routines	========================
  //These are your actual interrupt handling routines.
  #pragma interrupt YourHighPriorityISRCode
  void YourHighPriorityISRCode()
  {
    //Check which interrupt flag caused the interrupt.
    //Service the interrupt
    //Clear the interrupt flag
    //Etc.
  
  } //This return will be a "retfie fast", since this is in a #pragma interrupt section 
  #pragma interruptlow YourLowPriorityISRCode
  void YourLowPriorityISRCode()
  {
    //Check which interrupt flag caused the interrupt.
    //Service the interrupt
    //Clear the interrupt flag
    //Etc.
  
  } //This return will be a "retfie", since this is in a #pragma interruptlow section 
#endif


static int counterX = 0, counterY = 0;				//global int for accY & accX max value

//	========================	Board Initialization Code	========================
#pragma code
#define ROM_STRING rom unsigned char*

/******************************************************************************
 * Function:        void UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should take care of all of the application code
 *                  initialization that is required.
 *
 * Note:            
 *
 *****************************************************************************/
void UserInit(void)
{

  /* Initialize the mTouch library */
  mTouchInit();

  /* Call the mTouch callibration function */
  mTouchCalibrate();

  /* Initialize the accelerometer */
  InitBma150(); 

  /* Initialize the oLED Display */
   ResetDevice();  
   FillDisplay(0x00);
   //oledPutROMString((ROM_STRING)" PIC18F Starter Kit  ",0,0);
}//end UserInit


/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.                  
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void)
{
	// Soft Start the APP_VDD
	while(!AppPowerReady())
		;

    #if defined(PIC18F46J50_PIM)
  //Configure all I/O pins to use digital input buffers
    ANCON0 = 0xFF;                  // Default all pins to digital
    ANCON1 = 0xFF;                  // Default all pins to digital
    #endif
       
    UserInit();

}//end InitializeSystem


//	========================	Application Code	========================

BOOL CheckButtonPressed(void)
{
    static char buttonPressed = FALSE;					
    static unsigned long buttonPressCounter = 0;

    if(PORTBbits.RB0 == 0)														
    {						
		oledPutROMString((ROM_STRING)"Button: ",2,0);
		oledWriteCharRaw(0x7a);
        if(buttonPressCounter++ > 10)
        {
            buttonPressCounter = 0;
            buttonPressed = TRUE;	
        }
    }
    else
    {							
		oledPutROMString((ROM_STRING)"Button: ",2,0);
		oledWriteCharRaw(0x7b);
        if(buttonPressed == TRUE)
        {
            if(buttonPressCounter == 0)
            {	
                buttonPressed = FALSE;
                return TRUE;
            }
            else
            {
                buttonPressCounter--;
            }
        }
    }
    return FALSE;
}


void potentiometer()
{
	int i;
	char str[30];

	ADCON0bits.CHS = 4;		
	ADCON0bits.GO = 1;	
	while(ADCON0bits.GO);

	itoa(ADRES, str);					
	oledPutString(str, 0, 0);

	//clear garbage oled parameter
	if(ADRES < 1000){oledWriteChar1x(0x20, 0 + 0xB0, 18);}
	if(ADRES < 100){oledWriteChar1x(0x20, 0 + 0xB0, 12);}
	if(ADRES < 10){oledWriteChar1x(0x20, 0 + 0xB0, 6);}

	//graph bar
	oledWriteChar1x(0x5B, 0 + 0xB0,30);
	oledWriteChar1x(0x5D, 0 + 0xB0,80);

	for(i = 35; i <= 75;i++)
		oledWriteChar1x(0x2D, 0 + 0xB0,i);

	if(ADRES == 0)
	{
		oledWriteChar1x(0x4F, 0 + 0xBF0,35);
		return;
	}
					
	if(ADRES)
		oledWriteChar1x(0x4F, 0 + 0xBF0,35+(ADRES/50*2));	
}

void touchButtons()
{
	unsigned int left, right,scrollU, scrollD;;

	right  = mTouchReadButton(0);
	left   = mTouchReadButton(3);
	scrollU = mTouchReadButton(1);
	scrollD = mTouchReadButton(2);
	ADCON0 = 0b00010011;								//potentiometer declire again 								

	oledWriteChar1x(0x79, 2 + 0xB0, 11*10);

	//chack left touch
	if(left > 800)
		oledWriteChar1x(0x6C, 2 + 0xB0, 10*10);
	else
		oledWriteChar1x(0x4C, 2 + 0xB0, 10*10);
	
	//chack left touch
	if(right > 800)
		oledWriteChar1x(0x72, 2 + 0xB01, 12*10);
	else
		oledWriteChar1x(0x52,2 + 0xB0 , 12*10);

	//chack  scroll
	if(scrollU > 965)									
		oledWriteChar1x(0x7d, 1 + 0xB0, 11*10);
	if(scrollU < 960)					
		oledWriteChar1x(0x55, 1 + 0xB0, 11*10);	
	if(scrollD > 980)
		oledWriteChar1x(0x7c, 3 + 0xB0, 11*10);
	if(scrollD < 975)
		oledWriteChar1x(0x44, 3 + 0xB0, 11*10);

}

void accelerometer()
{
	BMA150_XYZ xyz;
	char xyArr[20] = {0};	
	BYTE lsb, msb;
	int i, val, repeat = 0;


	//accX
	for(i=13;i <= 40;i++)
		oledWriteChar1x(0x20, 4 + 0xB0, i);						//clear garbage oled parameter

	lsb = BMA150_ReadByte(BMA150_ACC_X_LSB); 	
	msb = BMA150_ReadByte(BMA150_ACC_X_MSB); 	
	xyz.x = 0;
	xyz.x = (short)msb << 8;
	xyz.x |= lsb;
	xyz.x >>= 6;
	if(xyz.x & 0x200)
		xyz.x |= 0xFC00;

	xyz.x = xyz.x << 2;		
	
	itoa(xyz.x, xyArr);
	oledPutROMString((ROM_STRING)"X: ",4,0);
	oledPutString(xyArr, 4, 15);

	//get max x
	for(i=55;i <= 110;i++)
		oledWriteChar1x(0x20, 4 + 0xB0, i);

	val = atoi(xyArr);
	if(val > counterX)
		counterX = val;
	
	//graph bar X
	oledWriteChar1x(0x5B, 4 + 0xB0,50);
	oledWriteChar1x(0x5D, 4 + 0xB0,100);
	repeat = counterX / 50;
	oledRepeatByte(0x18,4,55, repeat);

	//accY
	for(i=13;i <= 40;i++)
		oledWriteChar1x(0x20, 5 + 0xB0, i);

	lsb = BMA150_ReadByte(BMA150_ACC_Y_LSB); 	
	msb = BMA150_ReadByte(BMA150_ACC_Y_MSB); 	
	xyz.y = 0;
	xyz.y = (short)msb << 8;
	xyz.y |= lsb;
	xyz.y >>= 6;
	if(xyz.y & 0x200)
		xyz.y |= 0xFC00;

	xyz.x = xyz.x << 2;

	itoa(xyz.y, xyArr);
	oledPutROMString((ROM_STRING)"Y: ",5,0);
	oledPutString(xyArr, 5, 15);

	//get max y
	for(i=55;i <= 120;i++)
		oledWriteChar1x(0x20, 5 + 0xB0, i);

	val = atoi(xyArr);
	if(val > counterY)
		counterY = val;

	//graph bar Y
	oledWriteChar1x(0x5B, 5 + 0xB0,50);
	oledWriteChar1x(0x5D, 5 + 0xB0,100);
	repeat = counterY / 11.2;
	oledRepeatByte(0x18,5,55, repeat);

	//z
	lsb = BMA150_ReadByte(BMA150_ACC_Z_LSB); //LSB	
	msb = BMA150_ReadByte(BMA150_ACC_Z_MSB); //MSB	
	xyz.z = 0;
	xyz.z = (short)msb << 8;
	xyz.z |= lsb;
	xyz.z >>= 6;
	if(xyz.z & 0x200)
		xyz.z |= 0xFC00;

	itoa(xyz.z, xyArr);
	val = atoi(xyArr);

	//chack if microchip is upside down
	if(val < -58)
	{
		counterY = 0;
		counterX = 0;
		repeat = 1;
		oledRepeatByte(0x18,5,55, repeat);
		oledRepeatByte(0x18,4,55, repeat);
	}
}

void temperature()
{
	int temperature;
	char str[20];	

	temperature = BMA150_ReadByte(BMA150_TEMP);
	temperature = (temperature - 32) / 1.8;

	itoa(temperature, str);

	oledPutROMString((ROM_STRING)"Temp: ",7,0);
 	oledPutString(str, 7, 35);
	oledWriteCharRaw(0x7e);
	oledWriteCharRaw(0x43);

}

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
void main(void)
{



	InitBma150();
    InitializeSystem();
	
	
    while(1)												//Main is Usualy an Endless Loop
    {	
		/**************************************potentiometer************************************/
		potentiometer();			
			
		/**************************************button press************************************/
		CheckButtonPressed();								

		/**************************************L\R & scroll************************************/
		touchButtons();

		/******************************************temperature*********************************/
		temperature();

		/*******************************************accelerometer******************************/
		accelerometer();
	
  		

  }
}//end main


/** EOF main.c *************************************************/
//#endif
