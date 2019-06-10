/**********  Main definition file  **********/

#ifndef DEFINES_H
#define DEFINES_H

/**********  Boards  *********/
#define CSIM 0
#define Boston 1
#define Ek128 2
#define Evk1105 3
#define u8051 4

#ifndef BOARD
#define BOARD CSIM
#endif

#ifdef _MSC_VER
#define __attribute__(x)
#define __inline__
#define __builtin_va_list int
#define __asm__(x)
#endif

/**********  Global Defines  **********/
#define Greet "uM Monitor for Microcontrollers"
#define Version "\nVersion: 06.Oct.14"
#define Author "\nby Marcelo Dantas"
#define eMail "\nmarcelo.f.dantas@gmail.com\n"

#define Prompt "\n>"										// Command prompt
#define BS "\b \b"											// Backspace for line input
#define NL "\n"												// For printing new lines
#define Break "\n** BREAK **"								// Line break prefix
#define Error "\n** ERROR **"								// Line error prefix
#define Halted "System halted... Press RESET"
#define CLR "\x1B[H\x1B[J"
#define	CR	'\r'
#define LF	'\n'

#define LBS '\t'											// Label separator
#define LSS ','												// List separator
#define	REM	';'												// Comment start
#define EOS '\0'											// End of string
#define EOL '\n'											// End of program line
#define EOP '\0'											// End of Program

typedef unsigned char uchar;

/**********  Flags  **********/
typedef char FLAG;

#define MULTISPACE											// Allow multiple spaces between commands
#define LREXP												// Expressions evaluated left to right
#define CRLF												// Converts CR to LF when printing
#define FALSE 0
#define TRUE 1

#define	IMMEDIATE 0											// Executes in immediate mode
#define PARTITION 1											// Executes in partition mode

/**********  Generic memory structure sizes  **********/
#define LINESIZE 256										// Maximum size of a string (255+1)
#define NUMSIZE 14											// Maximum size of a number (for printing)
#define NAMESIZE 8											// Maximum size of a name (label, variable) (7+1)

/**********  Error management  **********/
typedef char ErrCode;

#define	eUnknown		-1									// Unknown error
#define	eNoErr			0									// No error
#define	eNotImp			1									// Not implemented
#define	eBufOvr			2									// Buffer overflow
#define	eStkOvr			3									// Stack overflow
#define	eStkUnd			4									// Stack underflow
#define	eNoPart			5									// No partition space
#define eNoSym			6									// No symbol table space
#define	eNoMemory		7									// No memory left
#define	eIllCmd			8									// Illegal command name
#define	eIllTerm		9									// Illegal command terminator
#define	eIllExpr		10									// Illegal expression
#define	eIllOper		11									// Illegal operator
#define	eIllValue		12									// Illegal value
#define	eIllNot			13									// Illegal use of NOT operator
#define eIllLine		14									// Illegal program line
#define eIllLabel		15									// Illegal label name
#define eIllVar			16									// Illegal variable name
#define eIllChar		17									// Illegal character
#define eIllRou			18									// Illegal routine or label name
#define	eNotSaved		19									// Routine not saved
#define	eUnmParen		20									// Unmatched parentheses
#define	eUnmString		21									// Unmatched string
#define	eDivZero		22									// Division by zero
#define	eString			23									// String too long
#define eMissEqual		24									// Missing equal sign
#define eNoVar			25									// Not defined
#define eNoLine			26									// Invalid line reference

/**********  Atom types for expression parsing  **********/
#define aDigit '0'		// Atom is a number
#define aAlpha 'A'		// Atom is a variable
#define aString '"'		// Atom is a string
#define aFunction '$'	// Atom is a function
#define aUnaryMinus '-'	// Atom is unary operator Minus
#define aUnaryPlus '+'	// Atom is unary operator Plus
#define aUnaryNot '\''	// Atom is unary operator Not

/**********  Character Types  **********/
#define	cNumber			'0'
#define	cString			'"'
#define	cVariable		'A'
#define	cGlobal			'^'
#define	cFunction		'$'

#endif