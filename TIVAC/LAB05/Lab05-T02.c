#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"

volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;

int main(void)
{
    uint32_t ui32Period;

    // set clock
ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    // enable ADC and Port F
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 32);

    // enable timer1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    // configure ADC
    ROM_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE,2,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
    ROM_ADCSequenceEnable(ADC0_BASE, 2);

    //calculate period
    ui32Period = (SysCtlClockGet() / 2);
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period -1);

    // enable timer interrupt
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    // start timer1
    TimerEnable(TIMER1_BASE, TIMER_A);

    // turn off LED
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

    while(1)
    {
    }
}

void Timer1IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    uint32_t ui32ADC0Value[4];

    // clear ADC and then start it
    ROM_ADCIntClear(ADC0_BASE, 2);
    ROM_ADCProcessorTrigger(ADC0_BASE, 2);

    // wait until ADC is finished
    while(!ROM_ADCIntStatus(ADC0_BASE, 2, false))
    {
    }

    // grab ADC data
    ROM_ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);

    // calculate Temp
    ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
    ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
    ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

    // check temp and update LED
    if(ui32TempValueF>=72) {
        ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
    } else {
        ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    }
}
