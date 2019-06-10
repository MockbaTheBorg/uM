/**********  u8051 board specific definitions  **********/

#ifndef U8051_H
#define U8051_H

/**********  Specific memory structure sizes  **********/
#define PARTSIZE 8192
#define SYMSIZE 4096

/**********  Initialize board parameters  **********/
void init_board(void)
{
}

/**********  Initialize console parameters  **********/
void init_console(void)
{
	PCON&=0x7F;					// Clear SMOD of PCON, No Double Baud Rate
	TMOD&=0xAF; TMOD|=0x20;		// Set Timer1 to Mode 2 (8-bit auto reload) for Baud Rate Generation
	TH1=0xFD;					// Set Baud Rate to 9600 bps for 11.0592M Hz
	SM0=0; SM1=1;				// Set UART to Mode 1 (8-bit UART)
	REN=1;						// Set REN of SCON to Enable UART Receive
	TR1=1;						// Set TR1 of TCON to Start Timer1
	TI=0; RI=0;					// Set TI/RI of SCON to Get Ready to Send/Receive
}

/**********  Read a character from the console (blocking)  **********/
char _getch(void)
{
	while (RI == 0);
	RI=0;
	return(SBUF);
}

/**********  Write a character to the console  **********/
void _putch(char ch)
{
	SBUF=ch;
	while (TI == 0);
	TI=0;
}

#endif