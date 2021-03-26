//-----------------------------------------------------------------------------
// RadioMaster TX16S 6-Position switch
//-----------------------------------------------------------------------------
// by Risto ( GitHub https://github.com/rotorman/TX16S-6Pos.git )
//
// Target:         C8051F330
// Tool chain:     Simplicity Studio v4.1.14.0 / Keil C51 9.60
// Command Line:   None
//
// Release 1.0
//    -Initial Revision
//    -26 MARCH 2021
//
// Brief overview of the code:
// * main() initializes the hardware and starts Timer3
// * all further processing is done in Timer3 ISR
// * first, the Knight-Rider lights are played, after which
//   the code starts outputting position via DAC according
//   to buttons pressed. The selected value is displayed
//   also on the LED.


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <SI_C8051F330_Register_Enums.h>

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup (void)
{
    PCA0MD &= ~0x40;                    // Disable Watchdog timer
}

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void main (void);
void Init_Device (void);
void Oscillator_Init (void);
void Port_IO_Init (void);
void DAC_Init (void);
void Timer3_Init (int16_t counts);
void StartupAnimation (void);
unsigned short ButtonPos (void);
SI_INTERRUPT_PROTO_USING(Timer3_ISR, TIMER3_IRQn, 3);

//-----------------------------------------------------------------------------
// Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// SFR and Bit Definitions
//-----------------------------------------------------------------------------

SI_SBIT(LED1, SFR_P1, 7);               // LED='1' means ON
SI_SBIT(LED2, SFR_P1, 6);
SI_SBIT(LED3, SFR_P1, 5);
SI_SBIT(LED4, SFR_P1, 4);
SI_SBIT(LED5, SFR_P1, 3);
SI_SBIT(LED6, SFR_P1, 2);

SI_SBIT(BUTTON1, SFR_P1, 1);            // BUTTON='0' means switched
SI_SBIT(BUTTON2, SFR_P1, 0);
SI_SBIT(BUTTON3, SFR_P0, 7);
SI_SBIT(BUTTON4, SFR_P0, 6);
SI_SBIT(BUTTON5, SFR_P0, 5);
SI_SBIT(BUTTON6, SFR_P0, 4);

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define SYSCLK          3062500        // SYSCLK frequency in Hz
#define TICK_RATE       500            // Execution tick rate in Hz

#define DEBOUNCECOUNT   10             // Tick count for the switch change to count
#define STARTUPANIMSTEP 50             // Startup animation step count
#define ANIMATIONCOUNT  4              // Count runs to do the Knight-Rider lights

#define DACMAX          0x3FFF         // Highest DAC output
#define DACSTEP         (DACMAX / 5)   // DAC output step size between positions

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned appState = 0;

//-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------

void main (void) 
{
    Init_Device();

    while(1){}
} // end main


//-----------------------------------------------------------------------------
// Init Routines
//-----------------------------------------------------------------------------

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    Oscillator_Init();
    Port_IO_Init();
    DAC_Init();
    Timer3_Init(SYSCLK/TICK_RATE);         // Using Timer3 as update scheduler
    IE_EA = 1;                             // Enable global interrupts
}

//-----------------------------------------------------------------------------
// OSCILLATOR_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This routine initializes the system clock to use the internal 3.0625 MHz
// oscillator as its clock source.
//
//-----------------------------------------------------------------------------

void Oscillator_Init(void)
{
    OSCLCN    |= 0x03;
}

void Port_IO_Init(void)
{
    // Crossbar:
    // P0.0  -  Skipped,     Open-Drain, Analog
    // P0.1  -  Skipped,     Open-Drain, Analog
    // P0.2  -  Skipped,     Open-Drain, Digital
    // P0.3  -  Skipped,     Open-Drain, Digital
    // P0.4  -  Skipped,     Open-Drain, Digital
    // P0.5  -  Skipped,     Open-Drain, Digital
    // P0.6  -  Skipped,     Open-Drain, Digital
    // P0.7  -  Skipped,     Open-Drain, Digital

    // P1.0  -  Skipped,     Open-Drain, Digital
    // P1.1  -  Skipped,     Open-Drain, Digital
    // P1.2  -  Unassigned,  Push-Pull,  Digital
    // P1.3  -  Unassigned,  Push-Pull,  Digital
    // P1.4  -  Unassigned,  Push-Pull,  Digital
    // P1.5  -  Unassigned,  Push-Pull,  Digital
    // P1.6  -  Unassigned,  Push-Pull,  Digital
    // P1.7  -  Unassigned,  Push-Pull,  Digital

    P0MDIN    = 0xFC;
    P1MDOUT   = 0xFC;
    P0SKIP    = 0xFF;
    P1SKIP    = 0x03;
    XBR1      = 0x40;

    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    LED4 = 0;
    LED5 = 0;
    LED6 = 0;
}

//-----------------------------------------------------------------------------
// DAC_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Enable VREF and IDAC0
//-----------------------------------------------------------------------------


void DAC_Init(void)
{
    REF0CN    = 0x03;                   // Enable internal VREF generator
    IDA0CN    = 0xF0;                   // Enable IDAC0 for 0.5 mA full-scale
    IDA0 = 0;                           // 0V
}

//-----------------------------------------------------------------------------
// Timer3_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1)  int16_t counts - calculated Timer overflow rate
//       range is positive range of integer: 0 to 32767
//
// Configure Timer3 to auto-reload at interval specified by <counts>
// using SYSCLK as its time base.  Interrupts are enabled.
//-----------------------------------------------------------------------------

void Timer3_Init(int16_t counts)
{
    TMR3CN  = 0x00;                     // Resets Timer 3, sets to 16 bit mode
    CKCON  |= 0x40;                     // Use system clock
    TMR3RL  = -counts;                  // Initial reload value

    TMR3    = 0xffff;                   // Sets timer to reload automatically
    EIE1   |= 0x80;                     // Enable Timer 3 interrupts
    TMR3CN  = 0x04;                     // Start Timer 3
}

//-----------------------------------------------------------------------------
// Internal Function Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Knight-Rider start-up animation
//-----------------------------------------------------------------------------

void StartupAnimation(void)
{
    unsigned lightRunPos;

    appState++;
    if (appState - 1 < STARTUPANIMSTEP)  { LED1 = 1; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 0; return; }

    lightRunPos = (appState - 1 - STARTUPANIMSTEP)%(10*STARTUPANIMSTEP);

    if (lightRunPos < STARTUPANIMSTEP)    { LED1 = 0; LED2 = 1; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 0; return; }
    if (lightRunPos < 2*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 1; LED4 = 0; LED5 = 0; LED6 = 0; return; }
    if (lightRunPos < 3*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 1; LED5 = 0; LED6 = 0; return; }
    if (lightRunPos < 4*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 1; LED6 = 0; return; }
    if (lightRunPos < 5*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 1; return; }
    if (lightRunPos < 6*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 1; LED6 = 0; return; }
    if (lightRunPos < 7*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 1; LED5 = 0; LED6 = 0; return; }
    if (lightRunPos < 8*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 0; LED3 = 1; LED4 = 0; LED5 = 0; LED6 = 0; return; }
    if (lightRunPos < 9*STARTUPANIMSTEP)  { LED1 = 0; LED2 = 1; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 0; return; }
    if (lightRunPos < 10*STARTUPANIMSTEP) { LED1 = 1; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 0; return; }
}

//-----------------------------------------------------------------------------
// Button debounce routine, considering also multi-button-presses
//-----------------------------------------------------------------------------

unsigned short ButtonPos(void)
{
    static short btn1 = 0;
    static short btn2 = 0;
    static short btn3 = 0;
    static short btn4 = 0;
    static short btn5 = 0;
    static short btn6 = 0;

    static unsigned short btnpos = 1;

    // If button is pressed, increase it's weight and at the same time suppress all others; else suppress own weight
    if (!BUTTON1) {btn1++; btn2--; btn3--; btn4--; btn5--; btn6--;} else btn1--;
    if (!BUTTON2) {btn1--; btn2++; btn3--; btn4--; btn5--; btn6--;} else btn2--;
    if (!BUTTON3) {btn1--; btn2--; btn3++; btn4--; btn5--; btn6--;} else btn3--;
    if (!BUTTON4) {btn1--; btn2--; btn3--; btn4++; btn5--; btn6--;} else btn4--;
    if (!BUTTON5) {btn1--; btn2--; btn3--; btn4--; btn5++; btn6--;} else btn5--;
    if (!BUTTON6) {btn1--; btn2--; btn3--; btn4--; btn5--; btn6++;} else btn6--;

    // Limit values between 0 and DEBOUNCECOUNT
    if (btn1 < 0) btn1 = 0; if (btn1 > DEBOUNCECOUNT) btn1 = DEBOUNCECOUNT;
    if (btn2 < 0) btn2 = 0; if (btn2 > DEBOUNCECOUNT) btn2 = DEBOUNCECOUNT;
    if (btn3 < 0) btn3 = 0; if (btn3 > DEBOUNCECOUNT) btn3 = DEBOUNCECOUNT;
    if (btn4 < 0) btn4 = 0; if (btn4 > DEBOUNCECOUNT) btn4 = DEBOUNCECOUNT;
    if (btn5 < 0) btn5 = 0; if (btn5 > DEBOUNCECOUNT) btn5 = DEBOUNCECOUNT;
    if (btn6 < 0) btn6 = 0; if (btn6 > DEBOUNCECOUNT) btn6 = DEBOUNCECOUNT;

    if (btn1 == DEBOUNCECOUNT) btnpos = 1;
    if (btn2 == DEBOUNCECOUNT) btnpos = 2;
    if (btn3 == DEBOUNCECOUNT) btnpos = 3;
    if (btn4 == DEBOUNCECOUNT) btnpos = 4;
    if (btn5 == DEBOUNCECOUNT) btnpos = 5;
    if (btn6 == DEBOUNCECOUNT) btnpos = 6;

    return btnpos;
}

//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Timer3_ISR
//-----------------------------------------------------------------------------
//
// This ISR is called on Timer3 overflows.  Timer3 is set to auto-reload mode.
//
//-----------------------------------------------------------------------------
SI_INTERRUPT_USING(Timer3_ISR, TIMER3_IRQn, 3)
{
    TMR3CN &= ~0x80;                    // Clear Timer3 overflow flag

    if (appState < 10*STARTUPANIMSTEP*ANIMATIONCOUNT + STARTUPANIMSTEP)
        StartupAnimation();
    else
    {
        switch (ButtonPos())
        {
            case (6):
                LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 1;
                IDA0 = (uint16_t) (5 * DACSTEP);
                break;
            case (5):
                LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 1; LED6 = 0;
                IDA0 = (uint16_t) (4 * DACSTEP);
                break;
            case (4):
                LED1 = 0; LED2 = 0; LED3 = 0; LED4 = 1; LED5 = 0; LED6 = 0;
                IDA0 = (uint16_t) (3 * DACSTEP);
                break;
            case (3):
                LED1 = 0; LED2 = 0; LED3 = 1; LED4 = 0; LED5 = 0; LED6 = 0;
                IDA0 = (uint16_t) (2 * DACSTEP);
                break;
            case (2):
                LED1 = 0; LED2 = 1; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 0;
                IDA0 = (uint16_t) DACSTEP;
                break;
            case (1):
            default:
                LED1 = 1; LED2 = 0; LED3 = 0; LED4 = 0; LED5 = 0; LED6 = 0;
                IDA0 = 0;
                break;
        }
    }
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
