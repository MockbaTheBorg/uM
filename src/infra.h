/**********  Infrastructure  **********/

#ifndef INFRA_H
#define INFRA_H

/**********  Global Variables  **********/
FLAG halt_requested=0;	// Ends execution if true
int Xpos, Ypos;			// Line position on the console
char* DumpAddr=&start;	// Address for Memory Dumps;

/**********  Global data structures  **********/
char Partition[PARTSIZE]="";
char* pPartitionEnd=Partition+PARTSIZE-1;	// Pointer to the partition's end
char* pPartition=Partition;					// Pointer to the partition's current position
char* pPartitionMax=Partition;				// Pointer to the partition's next available position

char SymTbl[SYMSIZE]="";
char* pSymTblEnd=SymTbl+SYMSIZE-1;
char* pSymTbl=SymTbl;
char* pSymTblMax=SymTbl;

/**********  Text error messages  **********/
#ifdef ERRTXT
typedef struct {
	ErrCode errcode;
	char* msg;
} errmsg_t;

static const errmsg_t errmsg_table[]={
	{eUnknown, "Unknown error"},
	{eNoErr, "No error"},
	{eNotImp, "Not Implemented"},
	{eBufOvr, "Buffer Overflow"},
	{eStkOvr, "Stack Overflow"},
	{eStkUnd, "Stack Underflow"},
	{eNoPart, "No partition space"},
	{eNoSym, "No symbol table space"},
	{eNoMemory, "No memory left"},
	{eIllCmd, "Illegal command name"},
	{eIllTerm, "Illegal command terminator"},
	{eIllExpr, "Illegal expression"},
	{eIllOper, "Illegal operator"},
	{eIllValue, "Illegal value"},
	{eIllNot, "Illegal use of NOT operator"},
	{eIllLine, "Illegal command line"},
	{eIllLabel, "Illegal label name"},
	{eIllVar, "Illegal variable name"},
	{eIllChar, "Illegal character"},
	{eIllRou, "Illegal routine or label name"},
	{eNotSaved, "Routine not saved"},
	{eUnmParen, "Unmatched parentheses"},
	{eUnmString, "Unmatched String"},
	{eDivZero, "Division by zero"},
	{eString, "String too long"},
	{eMissEqual, "Missing equal sign"},
	{eNoVar, "Not defined"},
	{eNoLine, "Invalid line reference"}
};

#define ERRMSG_TABLE_LEN sizeof(errmsg_table)/sizeof(errmsg_table[0])

#endif

/**********  Compares two \0 terminated strings  **********/
int StrCmp(char* str1, char* str2)
{
	int result=1;
	while (!(result=*str1-*str2)&&*str2) ++str1, ++str2;
	return(result==0);
}

/********** Returns the length of a \0 terminated string  **********/
int StrLen(char* string)
{
	int	len=0;
	while (*string!=0) {
		string++; len++;
	}
	return(len);
}

/**********  Converts a letter to lowercase  **********/
char ToLower(char ch)
{
	if (ch>64&&ch<91)
		ch+=32;
	return(ch);
}

/**********  Checks if a character is Alpha  **********/
char IsAlpha(char ch)
{
	if ((ch > 64&&ch<91)||(ch>96&&ch<123)) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/**********  Checks if a character is a Digit  **********/
char IsDigit(char ch)
{
	if (ch > 47&&ch<58) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/**********  Reads a character from the console (blocking)  **********/
char ReadCharB(void)
{
	return(_getch());
}

/**********  Writes a character to the console  **********/
void WriteChar(char ch)
{
	if (ch==CR)
		Xpos=0;
	if (ch==LF) {
#ifdef CRLF
		WriteChar(CR);
#endif
		Ypos++;
	} else
		Xpos++;
	_putch(ch);
}

/**********  Writes \0 terminated string to the console  **********/
void Write(char* line)
{
	while (*line!=0) {
		WriteChar(*line);
		line++;
	}
}

/**********  Writes EOL or EOP terminated string to the console  **********/
void WriteLine(char* line)
{
	while (*line!=EOL && *line!=EOP) {
		WriteChar(*line);
		line++;
	}
}

/**********  Writes MString to the console  **********/
void WriteMS(char* line)
{
	uchar sz=*line;
	char* pl=line+1;

	while (sz>0) {
		WriteChar(*pl);
		pl++;
		sz--;
	}
}

/**********  Reads line from the console  **********/
int Read(char* line, int size)
{
#if BOARD==Boston || BOARD==Ek128
	LED.OUTCLR=LED1;
#endif
	// Returns -1 is ^C (BREAK) was pressed
	char ch;	// Char read (and stored)
	char che;	// Char to actually echo
	char* linepos=line;
	int counter=0;

	while (TRUE)	// Very simplistic character line input
	{
		che=ch=ReadCharB();
		if (ch==0x0D||ch==0x0A)	// If CR of LF typed
		{
			*linepos=EOS;
			break;
		}
		if (ch==0x03)	// If ^C typed
		{
			linepos=line;
			*linepos=EOS;
			counter=-1;
			break;
		}
		if ((ch==0x08||ch==0x7F))	// If BS or DEL typed
		{
			if (counter>0) {
				Write(BS);
				counter--;
				linepos--;
			}
			continue;
		}
		if (ch==0x09)	// If TAB typed
		{
			che=' ';
		}
		if (che < 0x20||che > 0x7E)	// If invalid char typed
			continue;
		WriteChar(che);
		*linepos=ch;
		linepos++;
		counter++;
		if (counter==size) {
			*linepos=EOS;
			break;
		}
	}
#if BOARD==Boston || BOARD==Ek128
	LED.OUTSET=LED1;
#endif
		return(counter);
}

/**********  Clears Screen  **********/
void ClearScreen(void)
{
#if BOARD==CSIM
	system("cls");
#else
	Write(CLR);
#endif
	Xpos=0;
	Ypos=0;
}

/**********  Horizontal tabulation  **********/
void ht(int val)
{
	while (Xpos<val)
		WriteChar(' ');
}

/**********  Converts MString to (integer) number  **********/
int MStrToNum(char* str)
{
	uchar sz=*str;
	char* p=str+1;

	int val=0;
	int neg=1;

	if (sz) {
		if (*p=='-') {
			neg=-1;
			p++;
		}
		while (IsDigit(*p)&&sz>0) {
			val=val*10;
			val+=(*p)-48;
			p++;
			sz--;
		}
	} else {
		val=0;
	}
	return(val*neg);
}

/**********  Converts number to MString  **********/
void NumToMStr(int num, char* str)
{
	char* ptr=str+LINESIZE;
	uchar sz=0;
	char* ps=str+1;

	char isNegative=0;

	ptr--;
	*ptr=0;	// Prepare an empty \0 string

	if (num==0) {	// Convert to \0 string using the end of the buffer
		ptr--;
		*ptr='0';
	} else {
		if (num < 0) {
			isNegative=1;
			num=num * -1;
		}
		while (num > 0) {
			ptr--;
			*ptr=(num%10)+48;
			num=num/10;
		}
		if (isNegative) {
			ptr--;
			*ptr='-';
		}
	}

	while (*ptr!=0) {	// Move the string to the start of the buffer
		*ps=*ptr;
		ptr++;
		ps++;
		sz++;
	}
	*str=sz;
}

/**********  Checks if a MString is a number  **********/
int MSisNum(char* string)
{
	int val=0;
	int sz=*string;
	string++;
	while (sz>0) {
		if (IsDigit(*string)) {
			val=1;
		} else {
			val=0;
			break;
		}
		sz--;
	}
	return(val);
}

/**********  Slides range of memory left or right by amount  **********/
void MemSlide(char* begin, char* end, int amount)
{
	char* newpos;
	if (amount!=0) {	// No offset, no slide to do
		if (amount>0) {	// Slide right (positive offset)
			newpos=end+amount;
			while (end>=begin) {
				*newpos=*end;
				end--;
				newpos--;
			}
		} else {	// Slide left (negative offset)
			newpos=begin+amount;
			while (begin<=end) {
				*newpos=*begin;
				begin++;
				newpos++;
			}
		}
	}
}

/**********  Moves size bytes of memory from pos1 to pos2  **********/
void MemMove(char* pos1, char* pos2, int size)
{
	if (pos1!=pos2)	// Nothing to move
	{
		if (pos1<pos2) {	// Scan right to left
			pos1+=size;
			pos2+=size;
			while (size>0) {
				pos1--;
				pos2--;
				*pos2=*pos1;
				size--;
			}
		} else {			// Scan left to right
			while (size>0) {
				*pos2=*pos1;
				pos1++;
				pos2++;
				size--;
			}
		}
	}
}

/**********  Moves a MString to pos  **********/
void MStrMove(char* string, char* pos)
{
	int sz=*string+1;
	MemMove(string, pos, sz);
}

/**********  Compares two MStrings  **********/
int MStrCmp(char* str1, char* str2)
{
	int result=0;

	uchar sz1=*str1;
	uchar sz2=*str2;
	str1++;
	str2++;

	while ((*str1==*str2)&&(sz1>0&&sz2>0)) {
		str1++;
		str2++;
		sz1--;
		sz2--;
	}

	if (sz1==0&&sz2==0)	// Strings are the same
		goto bail;
	if (sz1<sz2) {
		result=-1;
		goto bail;
	}
	if (sz2<sz1) {
		result=1;
		goto bail;
	}
	if (*str1<*str2) {
		result=-1;
	} else {
		result=1;
	}

bail:
	return(result);
}

/**********  Converts MString to \0 terminated  **********/
void MStoS(char* in, char* out)
{
	int sz=*in;
	in++;
	while (sz) {
		*out=*in;
		out++;
		in++;
		sz--;
	}
	*out=0;
}

/**********  Converts char to (MString) hex  **********/
void CtoHex(char ch, char* hex)
{
	char hexa[]="0123456789ABCDEF";

	hex++;
	hex++;
	*hex=*(hexa+(ch&15));
	hex--;
	*hex=*(hexa+((ch&240)>>4));
	hex--;
	*hex=2;
}

/**********  Dumps memory  **********/
void MemDump(void)
{
	int l, c;
	char ch;
	char addr[NUMSIZE];
	char hex[3];

	for (l=0; l<16; l++) {
		NumToMStr((int)DumpAddr,addr);
		WriteMS(addr);
		ht(10);
		Write(": ");
		for (c=0; c<16; c++) {
			ch=*(DumpAddr+c);
			CtoHex(ch, hex);
			WriteMS(hex);
			WriteChar(' ');
		}
		for (c=0; c<16; c++) {
			ch=*DumpAddr;
			DumpAddr++;
			if (ch<32) {
				WriteChar('.');
			} else {
				WriteChar(ch);
			}
		}
		WriteChar(LF);
	}
}

/**********  Memory information  **********/
void MemInfo(void)
{
	int value;
	char result[LINESIZE];

	Write("\nData memory starts at: ");
	value=(int)&start;
	NumToMStr(value, result);
	WriteMS(result);
	Write("\nPartition starts at: ");
	value=(int)&Partition;
	NumToMStr(value, result);
	WriteMS(result);
	Write("\nSymbol table starts at: ");
	value=(int)&SymTbl;
	NumToMStr(value, result);
	WriteMS(result);
}

#ifdef ERRTXT
/*********  Text error message  **********/
char* get_err_msg(ErrCode code)
{
	char* msg=errmsg_table[eUnknown].msg;
	int i;
	for (i=0; i<ERRMSG_TABLE_LEN; i++) {
		if (errmsg_table[i].errcode==code) {
			msg=errmsg_table[i].msg;
		}
	}
	return msg;
}
#endif

/**********  Writes error message  **********/
void WriteError(ErrCode error, char* line, char* errpos)
{
	char num[LINESIZE];

	Write(Error);
	Write(" : ");
	NumToMStr(error, num);
	WriteMS(num);
#ifdef ERRTXT
	Write(" - ");
	Write(get_err_msg(error));
#endif
	Write(NL);
	WriteLine(line);
	Write(NL);
	while (errpos>line) {
		if (*line!=LBS) {
			WriteChar(' ');
		} else {
			WriteChar(LBS);
		}
		line++;
	}
	WriteChar('^');
}

#endif