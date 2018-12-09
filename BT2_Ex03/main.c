#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

unsigned long ulADC0Value[4];
char voltage_data[4];
volatile unsigned char output_data[16] = "The voltage is: ";
volatile unsigned long  ulADC0ValueAvg;
volatile unsigned long  ulVoltageValue;

void GetVoltage(void);
void SendVoltage(void);

void UARTIntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts
	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE)); //echo character
    	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
		SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
	}
}

void SendVoltage(void){
	uint8_t i = 0;
 	for(i=0;i<16;i++) {
		UARTCharPut(UART0_BASE, output_data[i]);
	}

 	GetVoltage();
	UARTCharPut(UART0_BASE, voltage_data[0]);
	UARTCharPut(UART0_BASE, '.');
	UARTCharPut(UART0_BASE, voltage_data[1]);
	UARTCharPut(UART0_BASE, voltage_data[2]);
	UARTCharPut(UART0_BASE, voltage_data[3]);
	UARTCharPut(UART0_BASE, ulVoltageValue%10 + 48);
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, 'V');
	UARTCharPut(UART0_BASE, '\n');
	UARTCharPut(UART0_BASE, '\r');
}
void GetVoltage(void){
	ADCIntClear(ADC0_BASE, 1);
	ADCProcessorTrigger(ADC0_BASE, 1);
	while(!ADCIntStatus(ADC0_BASE, 1, false))
	{
	}
	ADCSequenceDataGet(ADC0_BASE, 1, ulADC0Value);
	ulADC0ValueAvg = (ulADC0Value[0] + ulADC0Value[1] + ulADC0Value[2] + ulADC0Value[3] + 2)/4;
	ulVoltageValue = (3300 * ulADC0ValueAvg) / 4096;

	voltage_data[3] = ulVoltageValue%10 + 48;
	ulVoltageValue = ulVoltageValue/10 ;
	voltage_data[2] = ulVoltageValue%10 + 48;
	ulVoltageValue = ulVoltageValue/10 ;
	voltage_data[1] = ulVoltageValue%10 + 48;
	voltage_data[0] = ulVoltageValue/10 + 48;
}

int main(void) {
	// Set system clock
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

	// Enable Peripheral GPIO port F, ADC0, UART0 and Timer0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); //enable pin for LED PF2
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);

	// Set up ADC0
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);
	ADCSequenceDisable(ADC0_BASE, 1);
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH1|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
    IntMasterEnable(); //enable processor interrupts
    IntEnable(INT_UART0); //enable the UART interrupt
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts

    UARTCharPut(UART0_BASE, 'S');
    UARTCharPut(UART0_BASE, 'T');
    UARTCharPut(UART0_BASE, 'A');
    UARTCharPut(UART0_BASE, 'R');
    UARTCharPut(UART0_BASE, 'T');
    UARTCharPut(UART0_BASE, '\n');
    UARTCharPut(UART0_BASE, '\r');

	while(1){
		SendVoltage();
		SysCtlDelay(SysCtlClockGet()/3);
	}
}
