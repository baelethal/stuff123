#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint8_t ui8Adjust_RED;
    volatile uint8_t ui8Adjust_BLUE;
    volatile uint8_t ui8Adjust_GREEN;

	// set up clock and pwm clock
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	// enable pwm1 and port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Enable GPIO pins and PWMs
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);

	// enable switchs
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	// calculate pwm periods
    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust_RED * ui32Load / 100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust_RED * ui32Load / 100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust_RED * ui32Load / 100);
	
	// enable pwms
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    while(1)
    {
		// go through combinations of colors
        for(ui8Adjust_RED=90;ui8Adjust_RED>10;ui8Adjust_RED-=10) {
            for(ui8Adjust_BLUE=90;ui8Adjust_BLUE>10;ui8Adjust_BLUE-=5) {
                for(ui8Adjust_GREEN=90;ui8Adjust_GREEN>10;ui8Adjust_GREEN-=1) {
					
					//set pwm duties
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust_RED * ui32Load/100);
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust_BLUE * ui32Load/100);
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust_GREEN * ui32Load/100);
					
					// delay
                    SysCtlDelay(100000);
                }
            }
        }
        for(ui8Adjust_GREEN=90;ui8Adjust_GREEN>10;ui8Adjust_GREEN-=10) {
            for(ui8Adjust_BLUE=90;ui8Adjust_BLUE>10;ui8Adjust_BLUE-=5) {
                for(ui8Adjust_RED=90;ui8Adjust_RED>10;ui8Adjust_RED-=1) {
					// set pwm duties
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust_RED * ui32Load/100);
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust_BLUE * ui32Load/100);
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust_GREEN * ui32Load/100);
					
					// delay
                    SysCtlDelay(100000);
                }
            }
        }
     }
}

