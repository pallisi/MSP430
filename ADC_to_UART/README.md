# MSP430_ADC_UART
Code for MSP430, receive command from UART RX and transmit ADC result via UART TX

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
 * The code has been created with CCS Code Composer Studio
 * Version: 6.1.3.00034
*/
