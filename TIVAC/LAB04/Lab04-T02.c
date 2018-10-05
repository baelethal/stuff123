#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

// tracks if the switch is pressed
unsigned char volatile pressed;

int main(void)
{
    uint32_t ui32Period;

    // set up the clock
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // enable port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // unlock the GPIOCR register for port F and free pin 0
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;

    // set LEDs as output and switches as input
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);

    // configure switches
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);   // enable F0's pullup, the drive strength won't affect the input

    // enable switches
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // configure timer0
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // calculate period and set timer0
    ui32Period = (SysCtlClockGet() / 5) / 2;
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

    // enable interrupts
    IntEnable(INT_TIMER0A);
    IntEnable(INT_GPIOF);
    GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_RISING_EDGE);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
    IntMasterEnable();

    // enable timer
    TimerEnable(TIMER0_BASE, TIMER_A);

    while(1)
    {
    }
}

void Timer0IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Read the current state of the GPIO pin and
    // write back the opposite state
    if(pressed == 0) {
        if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
        }
        else
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
        }
    }
}

void Timer1IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntDisable(INT_TIMER1A);
    TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // disable timer1
    TimerDisable(TIMER1_BASE, TIMER_A);

    //reset to blinking
    pressed = 0;
}

void SW2IntHandler(void)
{

    // check if button is pressed
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))
    {
        pressed = 1;

        // turn LED on
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);

        // calculate 1.5 seconds
        uint32_t ui32Period;
        ui32Period = (SysCtlClockGet() * 1.5);
        TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period -1);

        //enable interupt for timer1
        IntEnable(INT_TIMER1A);
        TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER1_BASE, TIMER_A);
    }
    // clear interrupt
    GPIOIntClear (GPIO_PORTF_BASE, GPIO_INT_PIN_0);
}

