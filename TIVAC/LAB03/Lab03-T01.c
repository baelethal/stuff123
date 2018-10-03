#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint8_t ui8PinData=2;

int main(void)
{
    //Set clock to 8 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable port for LEDs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Make LED pins outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    while(1)
    {
        // Turn on LEDs
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, ui8PinData);

        // Wait 0.425 sec
        SysCtlDelay(1700000);

        // Turn off LEDs
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

        // Wait 0.425 sec
        SysCtlDelay(1700000);

        // Turn on the next LED in sequence
        if(ui8PinData<8) {ui8PinData*=2;} else {ui8PinData=2;}
    }
}