/*
 * Ioannis Pallis, August 2016
 *
 * Version 1.0
 * MCU: MSP430G2553
 *
 * Description: Simple program for testing purposes. Just stream the MESSAGE_BYTE
 * continiously through the TX pin (UART)
 *
 *								MSP430G2553
 *							-----------------
 *							|		|
 *					                |               |
 *							|		|
 *				       	         TX <---|P1.2		|
 *							|       	|
 *					                |		|
 *
 *
 * The code has been created with CCS Code Composer Studio
 * Version: 6.1.3.00034
*/

#include "msp430g2553.h"
#include <string.h>

#define MESSAGE_BYTE 0b10011001

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;   // Stop WDT

    if (CALBC1_1MHZ==0xFF)	 	// If calibration constant erased
    {
        while(1);    			// do not load, trap CPU!!
    }
    DCOCTL = 0;      			// Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;   	// Set DCO
    DCOCTL = CALDCO_1MHZ;


    /* Initialize USART */
    P1SEL = BIT1 + BIT2 ;     	// P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;

    UCA0CTL1 |= UCSSEL_2;       // Select SMCLK as clock source
    UCA0BR0 = 104;			    // Set the baud rate at 9600 in correlation to SMCLK
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;          // Modulation UCBRSx = 0
    UCA0CTL1 &= ~UCSWRST;       // Initialize USCI state machine**
    IE2 |= UCA0TXIE; 			// Enable USART0 TX interrupt

    __bis_SR_register(GIE);     // Enter LPM0, interrupts enabled

    while(1)
    {
    			while (!(IFG2 & UCA0TXIFG));
    			UCA0TXBUF = MESSAGE_BYTE;
    			IE2 |= UCA0TXIE;
    };

}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
	IE2 &= ~UCA0TXIE;   		// Disable UART A0 TX
}

