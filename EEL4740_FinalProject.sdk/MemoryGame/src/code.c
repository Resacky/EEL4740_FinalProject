#include "xparameters.h"
#include "xgpio.h"
#include "led_ip.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include <stdlib.h>

// SSD digit mappings
#define ZERO 0b00111111
#define ONE 0b00000110
#define TWO 0b01011011
#define THREE 0b01001111
#define FOUR 0b01100110
#define FIVE 0b01101101
#define SIX 0b01111101
#define SEVEN 0b00000111
#define EIGHT 0b01111111
#define NINE 0b01101111
#define A 0b01110111

/**************************** Variable Definitions *****************************/
XGpio push, dip, output; // GPIO instance for push buttons
XScuGic IntcInstance;    // Interrupt Controller Instance
XScuGic_Config *IntcConfig;

// Array to store random numbers
volatile unsigned int random_numbers[10];
// int to store current level
volatile int current_level = 0;
// int to store current index
volatile int current_index = 0;
// Global variable to store the switch value, which is the speed modifier
volatile unsigned int switch_value = 0;
// global variable to take into account if the game is won
volatile int gameWon = 0;
// global variable to take into account if the player exited the game
volatile int exitGame = 0;

/************************** Function Prototypes ******************************/
static int Init_Peripherals();
static int Init_GIC();
static int Configure_GIC();
static void EnableInts();
static void ExceptionInit();
static void Push_Intr_Handler(void *CallBackRef);
static void Switch_Intr_Handler(void *CallBackRef);
static void DisableIntr();
void delay(int seconds);
void seed_random();
void ledPattern();
void ledPatternIncorrect();
void ledPatternCorrect();
void ledPatternUserInput();
void initGame();
void displayLevel(int level);

/************************** Function Definitions ****************************/

// function to take into account the internal clocks delay
void delay(int base_delay)
{
    XTime tEnd, tCur;
    XTime_GetTime(&tCur);
    unsigned int delay_factor = switch_value + 1; // Inverse relationship: higher switch value, shorter delay
    tEnd = tCur + ((XTime)base_delay * COUNTS_PER_SECOND / delay_factor);
    while (tCur < tEnd)
    {
        XTime_GetTime(&tCur);
    }
}

// using the internal clock to generate a seed for random pattern generation
void seed_random()
{
    XTime t;
    XTime_GetTime(&t);
    srand((unsigned int)t);
}

// code found within Lab 7 to initialize the interrupts of the switches and buttons, and modded SSD.
static int Init_Peripherals()
{
    // Initializing the Switches
    int xResult = XGpio_Initialize(&dip, XPAR_SWITCHES_DEVICE_ID);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Dip switch init failed\r\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&dip, 1, 0xffffffff);

    // Initializing the Buttons
    xResult = XGpio_Initialize(&push, XPAR_BUTTONS_DEVICE_ID);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Push button init failed\r\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&push, 1, 0xffffffff);

    // Initializing the SSD
    xResult = XGpio_Initialize(&output, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Push button init failed\r\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&output, 1, 0);

    return XST_SUCCESS;
}

static int Init_GIC()
{
    IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
    if (!IntcConfig)
    {
        xil_printf("Looking for GIC failed! \r\n");
        return XST_FAILURE;
    }
    int xResult = XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Init GIC failed! \r\n");
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

static int Configure_GIC()
{
    // Configuration for button interrupts
    XScuGic_SetPriorityTriggerType(&IntcInstance, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR, 0x8, 0x3);
    int xResult = XScuGic_Connect(&IntcInstance,
                                  XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR,
                                  (Xil_ExceptionHandler)Push_Intr_Handler,
                                  (void *)&push);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Connect push button interrupt failed! \r\n");
        return XST_FAILURE;
    }

    // Configuration for switch interrupts
    XScuGic_SetPriorityTriggerType(&IntcInstance, XPAR_FABRIC_SWITCHES_IP2INTC_IRPT_INTR, 0xA, 0x3); // Adjust the priority as needed
    xResult = XScuGic_Connect(&IntcInstance,
                              XPAR_FABRIC_SWITCHES_IP2INTC_IRPT_INTR,
                              (Xil_ExceptionHandler)Switch_Intr_Handler,
                              (void *)&dip);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Connect switch interrupt failed! \r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

static void EnableInts()
{
    // Enable interrupts for buttons and switches
    XScuGic_Enable(&IntcInstance, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR);
    XScuGic_Enable(&IntcInstance, XPAR_FABRIC_SWITCHES_IP2INTC_IRPT_INTR);

    XScuGic_CPUWriteReg(&IntcInstance, 0x0, 0xf);

    XGpio_InterruptEnable(&push, 0xF);
    XGpio_InterruptGlobalEnable(&push);

    XGpio_InterruptEnable(&dip, 0xF);
    XGpio_InterruptGlobalEnable(&dip);
}

static void ExceptionInit()
{
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &IntcInstance);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 (void *)&IntcInstance);
    Xil_ExceptionEnableMask(XIL_EXCEPTION_ALL);
}

// function to handle the button interrupt, which holds most of the logic for the game itself.
static void Push_Intr_Handler(void *CallBackRef)
{
    XGpio *push_ptr = (XGpio *)CallBackRef;
    XGpio_InterruptDisable(push_ptr, 0xF);

    // this int is for the edge detection of the button itself. It is set to 1 to indicate a falling edge.
    static unsigned int last_button_state = 1;
    unsigned int button_state = XGpio_DiscreteRead(push_ptr, 1) & 0x0F;

    // Edge detection: Trigger only on button release (falling edge), this is to fix finicky behaviours with the buttons
    if (button_state == 0 && last_button_state != 0)
    {
        // Button release detected
        xil_printf("Button release detected: %x\r\n", last_button_state);
        ledPatternUserInput(last_button_state);
        // Check if both buttons 1 and 8 are pressed, which if set off it will exit the game at the users request.
        // There is a bug where the buttons 9 binary is not being recognized.
        if (last_button_state == 9)
        {
            exitGame = 1; // Set the exit flag
            XGpio_InterruptClear(push_ptr, 0xF);
            XGpio_InterruptEnable(push_ptr, 0xF);
            return;
        }
        if (last_button_state == random_numbers[current_index])
        {
            current_index++;
            if (current_level < current_index)
            {
                if (current_level == 9) // Check if current level is 9 (the 10th level, since it starts at 0)
                {
                    xil_printf("YOU WIN!!!\r\n");
                    gameWon = 1;
                    ledPatternCorrect();
                    last_button_state = button_state;
                    XGpio_InterruptClear(push_ptr, 0xF);
                    XGpio_InterruptEnable(push_ptr, 0xF);
                    return; // Exit the function to stop further processing
                }
                current_level++;
                current_index = 0;
                xil_printf("Next level!\r\n");
                ledPatternCorrect();
                ledPattern();
            }
        }
        else
        {
            current_level = 0;
            current_index = 0;
            xil_printf("Incorrect. Expected: %x, Received: %x\r\n", random_numbers[current_index], last_button_state);
            ledPatternIncorrect();
            ledPattern();
        }
    }
    last_button_state = button_state;
    XGpio_InterruptClear(push_ptr, 0xF);
    XGpio_InterruptEnable(push_ptr, 0xF);
}

// function to handle the switch interrupt which is used to modify the speed of the game based on the switch value
static void Switch_Intr_Handler(void *CallBackRef)
{
    XGpio *switch_ptr = (XGpio *)CallBackRef;
    XGpio_InterruptDisable(switch_ptr, 0xF);

    // Update global switch_value
    switch_value = XGpio_DiscreteRead(switch_ptr, 1) & 0x0F;

    xil_printf("Switch changed registered, speed modifier: %x\r\n", switch_value);

    // Re-enable interrupts
    XGpio_InterruptClear(switch_ptr, 0xF);
    XGpio_InterruptEnable(switch_ptr, 0xF);
}

void DisableIntr()
{
    XGpio_InterruptDisable(&push, 0x3);
    XScuGic_Disable(&IntcInstance, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR);
    XScuGic_Disconnect(&IntcInstance, XPAR_FABRIC_BUTTONS_IP2INTC_IRPT_INTR);
}

// functions to determine led pattern animations based on what conditions are needed
void ledPattern()
{
    xil_printf("Level: %x\r\n", current_level + 1);
    displayLevel(current_level);
    // Display the LED sequence up to the current level
    for (int i = 0; i <= current_level; i++)
    {
        xil_printf("Random Number Sequence: %x\r\n", random_numbers[i]);
        LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, random_numbers[i]);
        delay(1);
        LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0);
        delay(1);
    }
    xil_printf("Your turn!\r\n");
}

void ledPatternCorrect()
{
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 15);
    delay(1);
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0);
    delay(1);
}

void ledPatternIncorrect()
{
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 15);
    delay(1);
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0);
    delay(1);
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 15);
    delay(1);
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0);
    delay(1);
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 15);
    delay(1);
}

void ledPatternUserInput(unsigned int last_button_state)
{
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, last_button_state);
    delay(1);
    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0);
    delay(1);
}

// function that displays the current level towards the Seven Segment Display
void displayLevel(int level) {
    unsigned int display_pattern;
    switch(level) {
        case 0: display_pattern = ONE; break;
        case 1: display_pattern = TWO; break;
        case 2: display_pattern = THREE; break;
        case 3: display_pattern = FOUR; break;
        case 4: display_pattern = FIVE; break;
        case 5: display_pattern = SIX; break;
        case 6: display_pattern = SEVEN; break;
        case 7: display_pattern = EIGHT; break;
        case 8: display_pattern = NINE; break;
        case 9: display_pattern = A; break;
        default: display_pattern = ZERO;
    }
    XGpio_DiscreteWrite(&output, 1, display_pattern);
}


// function to initialize a game
void initGame()
{
    current_level = 0;
    current_index = 0;
    gameWon = 0;

    seed_random();

    for (int i = 0; i < 10; i++)
    {
        unsigned int random_bits = rand() & 0x03;
        switch (random_bits)
        {
        case 0:
            random_numbers[i] = 1;
            break;
        case 1:
            random_numbers[i] = 2;
            break;
        case 2:
            random_numbers[i] = 4;
            break;
        case 3:
            random_numbers[i] = 8;
            break;
        }
    }

    ledPattern();
}

int main(void)
{
    xil_printf("-- Start of the Program --\r\n");

    Init_Peripherals();
    Init_GIC();
    Configure_GIC();
    EnableInts();
    ExceptionInit();

    switch_value = XGpio_DiscreteRead(&dip, 1) & 0x0F;
    xil_printf("Switch initialized, speed modifier: %x\r\n", switch_value);

    xil_printf("Interrupt configurations finished\r\n");

    // Main game loop
    while (!exitGame)
    {
        initGame();
        while (!gameWon)
        {
            // Game loop
            if (exitGame)
                break;
        }
    }

    DisableIntr();
    xil_printf("Exiting game\r\n");
    return 0;
}
