/**********  XMega based board specific definitions  **********/
#ifndef XMEGA_H
#define XMEGA_H

#if BOARD == Boston
#define SERIAL PORTF
#define USART USARTF0
#define LED PORTF
#define LED0 PIN0_bm
#define LED1 PIN1_bm	// Busy led
#define MCUx256a3b
#define PARTSIZE 4096
#define SYMSIZE 2048
#endif
#if BOARD == Ek128
#define SERIAL PORTC
#define USART USARTC0
#define LED PORTF
#define LED0 PIN0_bm
#define LED1 PIN1_bm	// Busy led
#define MCUx128a1u
#define PARTSIZE 2048
#define SYMSIZE 2048
#endif

#define F_CPU 32000000UL		// CPU clock for 
#define USART_BAUDRATE 9600		// Baud rate to use on the Serial Console
#define USART_BSCALE 0
#define USART_BSEL (((F_CPU / (USART_BAUDRATE * 16UL)))-1)

/**********  Initialize board parameters  **********/
void init_board(void)
{
	// Sets the clock to 32Mhz
	CCP=CCP_IOREG_gc;							// disable register security for oscillator update
	OSC.CTRL=OSC_RC32MEN_bm;					// enable 32MHz oscillator
	while (!(OSC.STATUS & OSC_RC32MRDY_bm));	// wait for oscillator to be ready
	CCP=CCP_IOREG_gc;							// disable register security for clock update
	CLK.CTRL=CLK_SCLKSEL_RC32M_gc;				// switch to 32MHz clock

#if BOARD==Ek128
	LED.PIN0CTRL=64;
	LED.PIN1CTRL=64;
#endif

	// Sets all LED ports to outputs
	LED.DIRSET=LED0;
	LED.DIRSET=LED1;
	// Turn on the "busy" led
	LED.OUTSET=LED1;
}

/**********  Initialize console parameters  **********/
void init_console(void)
{
	// Set the TxD pin as an output and the RxD pin as an input
	SERIAL.DIRSET=PIN3_bm;
	SERIAL.DIRCLR=PIN2_bm;
	// Set baud rate & frame format
	USART.BAUDCTRLA=USART_BSEL;
	USART.BAUDCTRLB=0;
	// Set mode of operation
	// (async, no parity, 8 bit data, 1 stop bit)
	USART.CTRLA=0; // no interrupts enabled
	USART.CTRLC=USART_CMODE_ASYNCHRONOUS_gc |
		USART_CHSIZE_8BIT_gc |
		USART_PMODE_DISABLED_gc;
	// Enable transmitter and receiver
	USART.CTRLB=USART_TXEN_bm |
		USART_RXEN_bm;
}

/**********  Read a character from the console (blocking)  **********/
char _getch(void)
{
	while (!(USART.STATUS&USART_RXCIF_bm));
	return(USART.DATA);
}

/**********  Write a character to the console  **********/
void _putch(char ch)
{
	while (!(USART.STATUS&USART_DREIF_bm));
	USART.DATA=ch;
}

#endif