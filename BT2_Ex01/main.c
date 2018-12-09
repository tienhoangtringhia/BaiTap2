#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include <stdint.h>
#include <stdbool.h>

volatile uint8_t count = 0;
volatile uint32_t led_status = GPIO_PIN_1|GPIO_PIN_2;

void Timer0IntHandle(void){
	//Clear Int Flag
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	led_status ^= GPIO_PIN_2;
	if (count == 1){
		count = 0;
		led_status ^= GPIO_PIN_1;
	} else {
		count += 1;
	}

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2, led_status);
}

int main(void) {
	static uint32_t ulPeriod;

	// Set system clock
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	// Enable Peripheral GPIO port F and Timer0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	// GPIO F1 output
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2);

	//Set up Timer 0
	ulPeriod = SysCtlClockGet()/1000;
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntEnable(INT_TIMER0A);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);
	while(1){

	}
}
