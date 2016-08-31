/*
 * Ioannis Pallis, August 2016
 *
 * Version 1.0
 * MCU: MSP430G2553
 *
 * Description: A command is received via UART RX. Depending of the command the MCU
 * reads one of the three thermocouples. An average value of twenty (20) readings is
 * send via UART TX to the base station. The UART baud rate is set to 9600. In order
 * to properly acknoledge a command a START_BYTE and a END_BYTE, in ASCII represantation,
 * have to be send at the begining and at the end of the transmission, respectively.
 * Three commands can be acknowledged "?T1", "?T2" and "?T3". If a false message is send,
 * a NAK byte, in ASCII represetation, is send back to the base station. If the command
 * is properly recognized an ACK byte, in ASCII represation is send back. The ADC module
 * is configured on the fly, depending on the command. The ADC samples 20 repeatative
 * samples and transfers the result in temp[20]. The ADC conversion has a refernece voltage
 * of 1.5V (internal). With the end of the 20 conversions the average value is send, via UART
 * TX, to the base station in ASCII format represantation. No power saving configuration is
 * used in this version.
 *
 *								MSP430G2553
 *							-----------------
 *							|		|
 *					      RX    >---|P1.1 		|
 *							|		|
 *					      TX    <---|P1.2	    P1.7|---< T1
 *							|	        |
 *					      T3    >---|P1.5	    P1.6|---< T2
 *
 *
 * The code has been created with CCS Code Composer Studio
 * Version: 6.1.3.00034
*/

#include "msp430g2553.h"
#include <string.h>

#define START_BYTE 0x01 	    // From ASCII Table SOH (Start of heading)
#define END_BYTE 0x04		    // From ASCII Table EOT (End of transmission)
#define ACK 0x06			    // From ASCII Table ACK (Acknowledge)
#define NAK 0x15        	    // From ASCII Table NAK (Negative Acknowledge)
#define MAX_TX_BUF_LEN 5        // Maximum tx buffer lenghth
#define MAX_RX_BUF_LEN 4		// Maximum rx buffer lenghth

char unsigned txData[MAX_TX_BUF_LEN];
char rxData[MAX_RX_BUF_LEN];

const char thermocouple_1[MAX_RX_BUF_LEN] = {"?T1"};
const char thermocouple_2[MAX_RX_BUF_LEN] = {"?T2"};
const char thermocouple_3[MAX_RX_BUF_LEN] = {"?T3"};

unsigned int frame_received_flag = 0;
unsigned int buffer_counter = 0;
unsigned int packet_start_flg = 0;

unsigned int temp[20]={0};

void adc_setup(unsigned int channel);
void adc_sample();
void adc_avg_tx();
void binaryToASCII(unsigned int n, unsigned char * digits);


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
    IE2 |= UCA0RXIE + UCA0TXIE; // Enable USART0 RX/TX interrupt

    __bis_SR_register(GIE);     // Enter LPM0, interrupts enabled

    while(1)
    {
    	if((frame_received_flag == 1))
    	{

    		if(!(strcmp(rxData,thermocouple_1)))
    		{
    			while (!(IFG2 & UCA0TXIFG));
    			UCA0TXBUF = ACK;
    			frame_received_flag = 0;
    			adc_setup(INCH_7);
    			adc_sample();
    			adc_avg_tx();
    		}
    		else if(!(strcmp(rxData,thermocouple_2)))
    		{
    			while (!(IFG2 & UCA0TXIFG));
    			UCA0TXBUF = ACK;
    			frame_received_flag = 0;
    			adc_setup(INCH_6);
    			adc_sample();
    			adc_avg_tx();
    		}
    		else if(!(strcmp(rxData,thermocouple_3)))
    		{
    			while (!(IFG2 & UCA0TXIFG));
    			UCA0TXBUF = ACK;
    			frame_received_flag = 0;
    			adc_setup(INCH_5);
    			adc_sample();
    			adc_avg_tx();
    		}
    		else
    		{
    			frame_received_flag = 0;
    			while (!(IFG2 & UCA0TXIFG));
    			UCA0TXBUF = NAK;
    		}


    	}
    }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	IE2 |= UCA0TXIE;
	/* Protect against buffer overflow */
	/* In this case overwrite */
	if(buffer_counter == MAX_RX_BUF_LEN-1)
	{
		buffer_counter = 0;
	}

	/* Received Starting Byte */
	if((packet_start_flg == 0) & (UCA0RXBUF == START_BYTE))
	{
		packet_start_flg = 1;
		buffer_counter = 0;
	}

	/* Store Frame Bytes without SFB and EFB */
	else if(packet_start_flg & (UCA0RXBUF != END_BYTE))
	{
		/* Store in buffer */
		rxData[buffer_counter] = UCA0RXBUF;
		buffer_counter++;
	}

	/* Received Stop Byte so process Frame */
	else if((packet_start_flg == 1) & (UCA0RXBUF == END_BYTE))
	{
		frame_received_flag = 1;
		packet_start_flg = 0;
	}

 }

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{

}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
	IE2 &= ~UCA0TXIE;   		// Disable UART A0 TX
}

/* Setup of the ADC according to the received command*/
void adc_setup(unsigned int channel){
	ADC10CTL0 &= ~ENC; 			// Disable ADC
	ADC10CTL1 &=~INCH_15;		// Reset all channel of the ADC

	/* Set the ADC */
	/* Repeat channel controlled by the RX message */
	ADC10CTL1 = channel + CONSEQ_2;

	/* Set the ref at 1.5V, Sample and hold, multisequence conversion, enable interrupt, enable ADC */
	ADC10CTL0 = SREF_1 + ADC10SHT_2 + MSC + REFON + ADC10ON + ADC10IE + ENC;

	ADC10DTC1 = 0x14;   		// Set the DTC for 20 conversions
}

/* ADC conversion after the initialization*/
void adc_sample(){
	ADC10CTL0 &= ~ENC; 			// Disable ADC

	while(ADC10CTL1 & ADC10BUSY); // Ensure that the ADC is idle

	ADC10SA = (int)temp;        // Set the DTC to point at the variable temp
	ADC10CTL0 |= ENC + ADC10SC;	// Enable and start conversion
}

/*Calculation of the average and transmit*/
void adc_avg_tx(){
	unsigned int i;
	unsigned int avg_temp=0;

	/* Find the average value of the ADC reading */
	for (i=0; i<20; i++){
		avg_temp = avg_temp + temp[i];
	}

	avg_temp = avg_temp/20;
	/* Convert the avg_temp to ASCII for UART transmit */
	binaryToASCII(avg_temp, txData);

	int c = 0;
		while(c < 6) {
			while (!(IFG2 & UCA0TXIFG));
				UCA0TXBUF = txData[c];
			c++;
		}
}

/* Conversion in ASCII format*/
void binaryToASCII(unsigned int n, unsigned char * digits) {
    __asm(" clr		R14");
    __asm(" rla		R12");
    __asm(" rla		R12");
    __asm(" rla		R12");
    __asm(" rla		R12");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" rla		R12");
    __asm(" dadd	R14, R14");
    __asm(" mov.b	R14, 3(R13)");
    __asm(" swpb	R14");
    __asm(" mov.b	R14, 1(R13)");
    __asm(" rra		R14");
    __asm(" rra		R14");
    __asm(" rra		R14");
    __asm(" rra		R14");
    __asm(" mov.b	R14, 0(R13)");
    __asm(" swpb	R14");
    __asm(" mov.b	R14, 2(R13)");
    __asm(" and		#0x0F0F, 0(R13)");
    __asm(" and		#0x0F0F, 2(R13)");
    __asm(" add.b	#0x30, 3(R13)");
    __asm(" tst.b	0(R13)");
    __asm(" jnz l1");
    __asm(" mov.b	#0x20, 0(R13)");
    __asm(" tst.b	1(R13)");
    __asm(" jnz l2");
    __asm(" mov.b	#0x20, 1(R13)");
    __asm(" tst.b	2(R13)");
    __asm(" jnz l3");
    __asm(" mov.b	#0x20, 2(R13)");
    __asm(" jmp l4");
    __asm("l1:");
    __asm(" add.b	#0x30, 0(R13)");
    __asm("l2:");
    __asm(" add.b	#0x30, 1(R13)");
    __asm("l3:");
    __asm(" add.b	#0x30, 2(R13)");
    __asm("l4:");
    return;
}
