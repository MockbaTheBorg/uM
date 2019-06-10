/**********  M(umps) Commands  **********/
#ifndef COMMANDS_H
#define COMMANDS_H

/**********  Finds a line labeled label and returns its position in the partition  **********/
ErrCode FindLabel(char** p, char** newpExec)
{
	char* pExec=*p;
	ErrCode error=eNoLine;

	char lmax=NAMESIZE-1;

	char label[NAMESIZE];
	char* lsize=label;
	char* l;

	char label2[NAMESIZE];
	char* l2size=label2;
	char* l2;

	char* part=Partition;

	// Grab the label reference from the current line
	l=label+1;
	*lsize=0;
	// label must be [Alpha][Alpha]*[Digit]*
	if (!IsAlpha(*pExec)) {	// First char must be alpha
		goto bail;
	}

	while (IsAlpha(*pExec)&&*lsize<lmax) {	// Scan the Alpha chars
		*l=*pExec;
		pExec++;
		l++;
		(*lsize)++;
	}
	while (IsDigit(*pExec)&&*lsize<lmax) {	// Scan the Digit chars (if any)
		*l=*pExec;
		pExec++;
		l++;
		(*lsize)++;
	}
	if (*pExec!=EOP &&
		*pExec!=EOL &&
		*pExec!=LSS &&
		*pExec!=' ') {
		goto bail;
	}

	// Find that label in the partition
	while (*part!=EOP) {
		*newpExec=part;	// Store current line's position
		// Check current line's label
		l2=label2+1;
		*l2size=0;
		while (*part!=LBS) {	// Grab current label on label2
			*l2=*part;
			part++;
			l2++;
			(*l2size)++;
		}
		if (!MStrCmp(label, label2)) {// Found it ... bail out;
			error=eNoErr;
			break;
		}
		while (*part!=EOL&&*part!=EOP)	// Else skip this line
			part++;
		if (*part==EOL)
			part++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Skips arguments (Post Conditional failed)  **********/
ErrCode SkipArgs(char** p)
{
	char* pExec=*p;
	ErrCode error=eNoErr;

	if (*pExec==' ')
		pExec++;

	if (*pExec==EOL||*pExec==EOP) {
		error=eIllExpr;
	} else {
		while (*pExec!=' ' && *pExec!=EOL && *pExec!=EOP) {
			if (*pExec=='"') {
				pExec++;
				while (*pExec!='"')
					pExec++;
			}
			pExec++;
		}
	}

	*p=pExec;
	return(error);
}

/**********  Skips to the End Of Program  **********/
ErrCode SkipEOP(char** p)
{
	char* pExec=*p;
	ErrCode error=eNoErr;

	while (*pExec!=EOP)
		pExec++;

	*p=pExec;
	return(error);
}

/**********  Checks for Post Conditional  **********/
ErrCode PostCond(char** p, FLAG* execute)
{
	ErrCode error=eNoErr;
	char* pExec=*p;

	char result[LINESIZE];

	*execute=1;
	if (*pExec==':') {
		pExec++;
		error=Expr(&pExec, result);
		if (!error) {
			*execute=MStrToNum(result);
			if (!*execute)
				error=SkipArgs(&pExec);
		}
	}

	*p=pExec;
	return(error);
}

/**********  Entry point of DO command  **********/
ErrCode ecDo(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char* labelpos;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		error=eIllTerm;
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllLabel;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		error=eIllTerm;
		goto bail;
	}

	while (!error) {
		// (todo) treat label names
		error=(FindLabel(&pExec, &labelpos));
		if (!error)
			error=Execute(&labelpos, PARTITION);
		if (*pExec==EOL||*pExec==EOP||*pExec==' ')	// At the end of the arguments
			break;
		if (*pExec==LSS)	// Found a list separator, skip to the next argument
			pExec++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of GOTO command  **********/
ErrCode ecGoto(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char* labelpos;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		error=eIllTerm;
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllLabel;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		error=eIllTerm;
		goto bail;
	}

	error=(FindLabel(&pExec, &labelpos));
	while (*labelpos!=LBS)	// Skip label
		labelpos++;
	labelpos++;
	if (!error)
		pExec=labelpos;
bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of HALT command  **********/
ErrCode ecHalt(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;
	//(todo) check for hang
	halt_requested=1;

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of KILL command  **********/
ErrCode ecKill(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char varname[NAMESIZE];	// Buffer for the variable name
	char subs[LINESIZE];	// Buffer for the subscripts

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		*SymTbl=0;
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllVar;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		*SymTbl=0;
		goto bail;
	}

	while (!error) {
		error=GetVarName(&pExec, varname);
		if (error)
			break;
		subs[0]=0;	// Clear the subscripts list
		if (*pExec=='(') {
			error=GetSubs(&pExec, subs);
			if (error)
				break;
		}
		VarKill(varname, subs);
		if (*pExec==EOL||*pExec==EOP||*pExec==' ')	// At the end of the arguments
			break;
		if (*pExec==LSS)	// Found a list separator, skip to the next argument
			pExec++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of PRINT command  **********/
ErrCode ecPrint(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		Write(Partition);
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllExpr;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		Write(Partition);
		goto bail;
	}

	while (!error) {
		// (todo) treat label names
		if (*pExec==EOL||*pExec==EOP||*pExec==' ')	// At the end of the arguments
			break;
		if (*pExec==LSS)	// Found a list separator, skip to the next argument
			pExec++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of QUIT command  **********/
ErrCode ecQuit(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		SkipEOP(&pExec);
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllLabel;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		SkipEOP(&pExec);
		goto bail;
	}
	error=eIllTerm;

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of SET command  **********/
ErrCode ecSet(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char varname[NAMESIZE];	// Buffer for the variable name
	char subs[LINESIZE];	// Buffer for the subscripts
	char value[LINESIZE];	// Buffer for the value

	int val;
	uchar byte;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		error=eIllTerm;
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllVar;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		error=eIllTerm;
		goto bail;
	}

	while (!error) {
		switch (*pExec) {
			case '%':
				pExec++;
				error=Expr(&pExec, subs);
				if (!error) {
					val=MStrToNum(subs);
				} else {
					break;
				}
				pExec++;
				if (*pExec!='=') {
					error=eMissEqual;
					break;
				}
				pExec++;	// Point to the expression
				error=Expr(&pExec, value);
				if (!error) {
					byte=MStrToNum(value);
					*(char*)val=byte;
				}
				break;
			default:
				error=GetVarName(&pExec, varname);
				if (error)
					break;
				subs[0]=0;	// Clear the subscripts list
				if (*pExec=='(') {
					error=GetSubs(&pExec, subs);
					if (error)
						break;
				}
				if (*pExec!='=') {
					error=eMissEqual;
					break;
				}
				pExec++;	// Point to the expression
				error=Expr(&pExec, value);
				if (!error) {
					error=VarSet(varname, subs, value);
				}
		}
		if (error)
			break;
		if (*pExec==EOL||*pExec==EOP||*pExec==' ')	// At the end of the arguments
			break;
		if (*pExec==LSS)	// Found a list separator, skip to the next argument
			pExec++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of WRITE command  **********/
ErrCode ecWrite(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char result[LINESIZE];

	int val;
	uchar byte;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		VarList();
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllExpr;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		VarList();
		goto bail;
	}

	while (!error) {
		switch (*pExec) {
			case '!':	// Line feed
				WriteChar(LF);
				pExec++;
				break;
			case '#':	// Clear screen
				ClearScreen();
				pExec++;
				break;
			case '*':	// Char
				pExec++;
				error=Expr(&pExec, result);
				if (!error) {
					val=MStrToNum(result);
					if (val<0||val>255) {
						error=eIllValue;
					} else {
						WriteChar(val);
					}
				}
				break;
			case '?':	// Horizontal tab
				pExec++;
				error=Expr(&pExec, result);
				if (!error) {
					val=MStrToNum(result);
					ht(val);
				}
				break;
			case '%':	// Memory address
				pExec++;
				error=Expr(&pExec, result);
				if (!error) {
					val=MStrToNum(result);
					byte=*(char*)val;
					NumToMStr(byte, result);
					WriteMS(result);
				}
				break;
			default:
				error=Expr(&pExec, result);
				if (!error)
					WriteMS(result);
		}
		if (error)
			break;
		if (*pExec==EOL||*pExec==EOP||*pExec==' ')	// At the end of the arguments
			break;
		if (*pExec==LSS)	// Found a list separator, skip to the next argument
			pExec++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of XECUTE command  **********/
ErrCode ecXecute(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char result[LINESIZE];
	char line[LINESIZE];

	char* pr;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		error=eIllTerm;
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllExpr;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		error=eIllTerm;
		goto bail;
	}

	while (!error) {
		error=Expr(&pExec, result);
		if (error)
			break;
		MStoS(result, line);	// Convert MString to \0 terminated for execution
		pr=line;
		error=Execute(&pr, IMMEDIATE);
		if (error)
			break;
		if (*pExec==EOL||*pExec==EOP||*pExec==' ')	// At the end of the arguments
			break;
		if (*pExec==LSS)	// Found a list separator, skip to the next argument
			pExec++;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of ZDO command  **********/
ErrCode ecZDo(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	pPartition=Partition;
	error=Execute(&pPartition, PARTITION);

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of ZINFO command  **********/
ErrCode ecZInfo(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		MemInfo();
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllExpr;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		MemInfo();
	} else {
		error=eIllExpr;
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of ZMEMDUMP command  **********/
ErrCode ecZMemdump(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	char result[NUMSIZE];

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (no args)
		MemDump();
		goto bail;
	}

	if (*pExec!=' ') {
		error=eIllTerm;
		goto bail;
	}

	pExec++;	// Skip to the beginning of the argument

	if (*pExec==EOL||*pExec==EOP) {	// Found EOL or EOP (error)
		error=eIllExpr;
		goto bail;
	}

	if (*pExec==' ') {	// Found a command separator (no args)
		MemDump();
		goto bail;
	}

	error=Expr(&pExec, result);
	if (!error) {
		DumpAddr=(char*)MStrToNum(result);
		MemDump();
	}

bail:
	*p=pExec;
	return(error);
}

/**********  Entry point of ZRemove command  **********/
ErrCode ecZRemove(char** p)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	FLAG postcond;

	error=PostCond(&pExec, &postcond);
	if (error||!postcond)
		goto bail;

	pPartition=Partition;	// Pointer to the partition's current position
	pPartitionMax=Partition;	// Pointer to the partition's next available position
	*pPartition=0;

bail:
	*p=pExec;
	return(error);
}

#endif