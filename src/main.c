/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
uM Monitor for Microcontrollers
by Marcelo Ferreira Dantas
marcelo.f.dantas@gmail.com

This is a M(umps) language interpreter targeted onto microcontrollers,
loosely based on the 1984 M(umps) Standard and on the Z80 Mumps for CP/M.
Information about the M(umps) language can be found at http://en.wikipedia.org/wiki/MUMPS

Deviations from the M(umps) standard:

. label/variable names can only be up to 7 chars long only
. label/variable names can only have 1 or more letters followed by 0 or more numbers
. no '%' allowed on label/variable names
. no other device than console is implemented
. no global variables (no filesystem implemented yet)
. operator [ is not implemented yet
. operator / does integer division for now
. all numbers are integers for now
. (number of subscripts)+(length of all subscripts) must be less than LINESIZE
. kill (exclude_var) not implemented

Mar.5.2014 - First line of code written
Mar.8.2014 - Implemented expression parser
Mar.12.2014 - Implemented symbol table

* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char start;
/**********  Includes  **********/
#include "defines.h"

#if BOARD == CSIM
#include <conio.h>
#include "csim.h"
#define ERRTXT	// Error messages in text format
#define CMDLONG	// Allow also long forms of the commands/functions
#endif
#if BOARD == Boston || BOARD == Ek128
#include <avr/io.h>
#include "xmega.h"
//#define ERRTXT	// Disabled to save memory
//#define CMDLONG	// Disabled to save memory
#endif
#if BOARD == Evk1105
#include "ac3.h"
//#define ERRTXT	// Disabled to save memory
//#define CMDLONG	// Disabled to save memory
#endif
#if BOARD == u8051
#include <mcs51/compiler.h>
#include <mcs51/mcs51reg.h>
#include "u8051.h"
//#define ERRTXT	// Disabled to save memory
//#define CMDLONG	// Disabled to save memory
#endif 

#include <stdlib.h>

/**********  Function Prototypes  **********/
ErrCode Execute(char** p, FLAG ExecMode);
ErrCode exprEval(char** p, char* atom1);

#include "infra.h"		// Insfrastructure functions and variables
#include "symtbl.h"		// Symbol table
#include "functions.h"	// M(umps) functions and special variables
#include "expr.h"		// Expression parser
#include "commands.h"	// M(umps) commands

/**********  Checks if line contains a label separator  **********/
char HasLabel(char* line)
{
	char result=FALSE;
	while (*line!=EOL && *line!=EOP) {
		if (*line==LBS) {
			result=TRUE;
			break;
		}
		line++;
	}
	return(result);
}

/**********  Adds line to the end of the partition  **********/
ErrCode AddLine(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	char* pInsert;

	int lsize=0;
	uchar lmax=NAMESIZE-1;

	// label must be [Alpha][Alpha]*[Digit]* or no label
	if (!IsAlpha(*pExec)&&*pExec!=LBS) {	// If there's a label first char must be alpha
		error=eIllLabel;
		goto bail;
	}

	if (*pExec!=LBS)	// Label is not empty
	{
		while (IsAlpha(*pExec)&&lsize<lmax)	// Scan the Alpha chars
		{
			pExec++;
			lsize++;
		}
		while (IsDigit(*pExec)&&lsize<lmax)	// Scan the Digit chars (if any)
		{
			pExec++;
			lsize++;
		}
	}

	if (*pExec!=LBS) {	// At the end must be pointing to a label separator
		error=eIllLabel;
		goto bail;
	}
	pExec++;	// Check the first character of the line
	if (!IsAlpha(*pExec)&&*pExec!=REM) {	// First char of the line must be a command or a comment
		error=eIllLine;
		goto bail;
	}
	// Append this line into the partition
	pInsert=*p;
	lsize=StrLen(pInsert);
	if (pPartitionMax+lsize < pPartitionEnd) {
		if (pPartitionMax>Partition) {
			*pPartitionMax=EOL;
			pPartitionMax++;
		}
		MemMove(pInsert, pPartitionMax, lsize);
		pPartitionMax+=lsize;
		*pPartitionMax=0;
	} else {
		error=eNoPart;
	}
	if (error)
		WriteError(error, pExec, pExec);

bail:
	*p=pExec;
	return(error);
}

/**********  Runs a command pointed by pExec  **********/
ErrCode RunCommand(char** p)
{
	typedef struct {
		char* name;
		ErrCode(*addr)(char **);
	} cmdtable;

	static const cmdtable commands[]={
		{"d", &ecDo},
		{"g", &ecGoto},
		{"h", &ecHalt},
		{"k", &ecKill},
		{"p", &ecPrint},
		{"q", &ecQuit},
		{"s", &ecSet},
		{"w", &ecWrite},
		{"x", &ecXecute},
		{"zd", &ecZDo},
		{"zi", &ecZInfo},
		{"zm", &ecZMemdump},
		{"zr", &ecZRemove}
#ifdef CMDLONG
		,
		{"do", &ecDo},
		{"goto", &ecGoto},
		{"halt", &ecHalt},
		{"kill", &ecKill},
		{"print", &ecPrint},
		{"quit", &ecQuit},
		{"set", &ecSet},
		{"write", &ecWrite},
		{"xecute", &ecXecute},
		{"zdo", &ecZDo},
		{"zinfo", &ecZInfo},
		{"zmemdump", &ecZMemdump},
		{"zremove", &ecZRemove}
#endif
	};

#define CMDTABLE_LEN sizeof(commands)/sizeof(commands[0])

	ErrCode error=eNoErr;
	char* pExec=*p;

	char cmd[NAMESIZE];
	char* pcmd=cmd;
	uchar csize=0;
	uchar cmax=NAMESIZE-1;
	int i;

	ErrCode(*Command)(char **);	// Defines a pointer to the command entry points

	if (!IsAlpha(*pExec)) {	// Command must start with a letter
		error=eIllCmd;
		goto bail;
	}
	while (IsAlpha(*pExec)) {
		*pcmd=ToLower(*pExec);	// commands are treated lowercase internally
		pcmd++;
		pExec++;
		csize++;
		if (csize==cmax) {	// Command is too long
			error=eIllCmd;
			break;
		}
	}
	if (error)	// It is a valid command (possibly)
		goto bail;

	if (*pExec==':'||*pExec==' '||*pExec==EOL||*pExec==EOP) {	// Command ends on one of these
		*pcmd=0;

		Command=0;
		for (i=0; i<CMDTABLE_LEN; i++) {
			if (StrCmp(cmd, commands[i].name)) {
				Command=commands[i].addr;
				break;
			}
		}

		if (!Command)
			error=eIllCmd;

		if (!error)	// It is a valid command (definitely)
		{
			error=Command(&pExec);	// Execute it
		}
	} else {
		error=eIllCmd;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Executes a line pointed by pExec  **********/
ErrCode Execute(char** p, FLAG ExecMode)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	char* pExecSave=pExec;	// Save the initial line position for error printing

	if (ExecMode==PARTITION) {
		//Skip Label
		while (*pExec!=LBS)
			pExec++;
		pExec++;
	}

	while (*pExec!=EOP) {
		// After the label there must be a command or a comment
		if (IsAlpha(*pExec)) {
			error=RunCommand(&pExec);
			if (error)
				break;
		}
		if (*pExec==REM)	// Found a comment
		{
			if (ExecMode==PARTITION) {
				while (*pExec!=EOL && *pExec!=EOP)
					pExec++;
			} else {
				break;
			}
		}
		if (*pExec==EOL) {	// At the end of a line, skip to the next one
			pExec++;
			pExecSave=pExec;
			while (*pExec!=LBS)	// skip next line's label
				pExec++;
			pExec++;
			continue;
		}
		if (*pExec==EOP) {	// At the end of the program, nothing else to do
			break;
		}
		if (*pExec==' ')	// At the command separator, skip to the next command
			pExec++;
		if (*pExec==EOL||*pExec==EOP)	// Next command is end of line or end of program
		{
			error=eIllCmd;
			break;
		} else {
			error=eIllChar;
			break;
//			continue;
		}
//		error=eIllLine;
//		break;
	}

	if (error)
		WriteError(error, pExecSave, pExec);

	*p=pExec;
	return(error);
}

/**********  Process a line and decide for immediate execution  **********/
void Process(char** p)
{
	char* pExec=*p;

	if (HasLabel(pExec)) {
		AddLine(&pExec);
	} else {
		Execute(&pExec, IMMEDIATE);
	}
}

/**********  Main Program  **********/
int main(void)
{
	int charcount;
	char line[LINESIZE];	// Line to execute
	char* pExec;			// Execution pointer	

	init_board();
	init_console();

	ClearScreen();
	Write(Greet);
	Write(Version);
	Write(Author);
	Write(eMail);

	while (1) {
		Write(Prompt);
		charcount=Read(line, LINESIZE-1);
		if (charcount==-1) {
			Write(Break);
		} else {
			if (charcount>0) {
				Write(NL);
				pExec=line;
				Process(&pExec);
			}
		}
		if (halt_requested)
			break;
	}
#if BOARD != CSIM
	Write(Halted);
	while (1)
		;
#else
	return(0);
#endif
}