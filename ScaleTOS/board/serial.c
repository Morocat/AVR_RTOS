/*
* serial.c
*
* Created: 12/22/2017 2:27:21 PM
*  Author: alelop
*/

#include "serial.h"
#include "usart_driver.h"
#include <string.h>

#define usart0 USARTC0

USART_data_t 	usart0_data; // RD1/TD1

void serial_init(void) {
	// PC3 (TXD0) as output.
	PORTC.DIRSET   = PIN3_bm;
	// PC2 (RXD0) as input.
	PORTC.DIRCLR   = PIN2_bm;

	USART_InterruptDriver_Initialize( &usart0_data, &usart0, USART_DREINTLVL_LO_gc);

	// 8 Data bits, No Parity, 2 Stop bits.
	USART_Format_Set( usart0_data.usart, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);

	// Enable RXC interrupt.
	USART_RxdInterruptLevel_Set( usart0_data.usart, USART_RXCINTLVL_LO_gc);
	//USART_TxdInterruptLevel_Set( usart0_data.usart, USART_TXCINTLVL_LO_gc);

	USART_Baudrate_Set( &usart0, SR_BAUD_9600_V , 0 );

	// Enable both RX and TX.
	USART_Rx_Enable( usart0_data.usart );
	USART_Tx_Enable( usart0_data.usart );

	/* Enable PMIC interrupt level low. */
	//PMIC.CTRL |= PMIC_LOLVLEX_bm;
}

void serial_send_string(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++)
		USART_TXBuffer_PutByte( &usart0_data, (uint8_t)str[i]);
}

void USARTC0_RXC_vect (void) __attribute__ ((signal,__INTR_ATTRS));
void USARTC0_RXC_vect (void)
{
	USART_RXComplete( &usart0_data );
}

void USARTC0_DRE_vect (void) __attribute__ ((signal,__INTR_ATTRS));
void USARTC0_DRE_vect (void)
{
	USART_DataRegEmpty( &usart0_data );
}