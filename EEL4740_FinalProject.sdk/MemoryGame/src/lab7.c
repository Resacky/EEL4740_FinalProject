#include "xparameters.h"
#include "xgpio.h"
#include "led_ip.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include <stdlib.h>

/**************************** Variable Definitions *****************************/
XGpio push, dip;           // GPIO instance for push buttons
XScuGic IntcInstance; // Interrupt Controller Instance
XScuGic_Config *IntcConfig;

volatile unsigned int random_numbers[10]; // Array to store random numbers
volatile int current_level = 0;
volatile int current_index = 0;
volatile unsigned int switch_value = 0; // Global variable to store the switch value

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

/************************** Function Definitions ****************************/

void delay(int base_delay)
{
    XTime tEnd, tCur;
    XTime_GetTime(&tCur);
    unsigned int delay_factor = 16 - switch_value; // Inverse relationship: higher switch value, shorter delay
    tEnd = tCur + ((XTime)base_delay * COUNTS_PER_SECOND / delay_factor);
    while (tCur < tEnd)
    {
        XTime_GetTime(&tCur);
    }
}

void seed_random()
{
    XTime t;
    XTime_GetTime(&t);
    srand((unsigned int)t);
}

static int Init_Peripherals()
{
    int xResult = XGpio_Initialize(&dip, XPAR_SWITCHES_DEVICE_ID);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Dip switch init failed\r\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&dip, 1, 0xffffffff);

    xResult = XGpio_Initialize(&push, XPAR_BUTTONS_DEVICE_ID);
    if (xResult != XST_SUCCESS)
    {
        xil_printf("Push button init failed\r\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&push, 1, 0xffffffff);
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

static void Push_Intr_Handler(void *CallBackRef)
{
    XGpio *push_ptr = (XGpio *)CallBackRef;
    XGpio_InterruptDisable(push_ptr, 0xF);

    static unsigned int last_button_state = 0;
    unsigned int button_state = XGpio_DiscreteRead(push_ptr, 1);

    // Edge detection: Trigger only on button press (rising edge)
    if (button_state != last_button_state)
    {
        last_button_state = button_state;
        LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, button_state);
        if (button_state != 0)
        {
            // Button press detected
            xil_printf("Button press detected: %x\r\n", button_state);
            if (button_state == random_numbers[current_index])
            {
                current_index++;
                if (current_index > current_level)
                {
                    current_level++;
                    current_index = 0;
                    xil_printf("Next level!\r\n");
                    ledPattern();
                    if (current_level >= 10)
                    {
                        xil_printf("YOU WIN!!!\r\n");
                    }
                }
            }
            else
            {
                xil_printf("Incorrect. Expected: %x, Received: %x\r\n", random_numbers[current_index], button_state);
                current_level = 0;
                current_index = 0;
                ledPattern();
            }
        }
    }
    XGpio_InterruptClear(push_ptr, 0xF);
    XGpio_InterruptEnable(push_ptr, 0xF);
}

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

void ledPattern()
{
    xil_printf("Level: %x\r\n", current_level + 1);
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

int main(void)
{
    xil_printf("-- Start of the Program --\r\n");

    Init_Peripherals();
    Init_GIC();
    Configure_GIC();
    EnableInts();
    ExceptionInit();

    xil_printf("Interrupt configurations finished\r\n");

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

    while (1)
    {
        // while buffer, waiting for interrupt.
    }

    DisableIntr();
    return 0;
}