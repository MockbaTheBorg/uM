/**********  Symbol Table management  **********/
#ifndef SYMTBL_H
#define SYMTBL_H

/**********  Gets a variable name  **********/
ErrCode GetVarName(char** p, char* result)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	char* v=result+1;
	uchar vsize=0;
	uchar vmax=NAMESIZE-1;

	// variable must be [Alpha][Alpha]*[Digit]*
	if (!IsAlpha(*pExec)) {	// If there's a variable first char must be alpha
		error=eIllVar;
		goto bail;
	}

	while (IsAlpha(*pExec)&&vsize<vmax)	// Scan the Alpha chars
	{
		*v=*pExec;
		v++;
		pExec++;
		vsize++;
	}
	while (IsDigit(*pExec)&&vsize<vmax)	// Scan the Digit chars (if any)
	{
		*v=*pExec;
		v++;
		pExec++;
		vsize++;
	}
	*result=vsize;

bail:
	*p=pExec;
	return(error);
}

/**********  Gets variable subscripts  **********/
ErrCode GetSubs(char** p, char* subs)
{
	ErrCode error=eNoErr;
	char* pExec=*p;
	char* s=subs;
	char nsubs=0;

	char result[LINESIZE];

	pExec++;	// skip to the first subscript expression
	s++;
	while (!error) {
		error=exprEval(&pExec, result);
		if (error)
			break;

		MStrMove(result, s);
		s+=*(result+1);
		nsubs++;
		if (*pExec==')') {
			*subs=nsubs;
			pExec++;
			break;
		}
		if (*pExec==LSS) {	// Found a list separator, skip to the next argument
			pExec++;
			continue;
		}
		error=eUnmParen;
		break;
	}

	*p=pExec;
	return(error);
}

/**********  Prints out a list of variables  **********/
void VarList(void)
{
	if (*SymTbl!=0) {	// Symbol table is not empty
		char bkey[LINESIZE];
		char* key=bkey;
		char bval[LINESIZE];
		char* value=bval;

		uchar offkey;
		uchar offval;
		uchar nsubs;

		char* ps=SymTbl;

		while (*ps!=0) {	// Repeat until reaching the end of the symbol table
			key=bkey;
			MStrMove(ps, key);	// Get the key (name+subs)
			offkey=*key+1;
			ps+=offkey;

			value=bval;
			MStrMove(ps, value);	// Get the value
			offval=*value+1;
			ps+=offval;

			key++;
			WriteMS(key);
			key+=*key+1;
			if (*key>0) {
				nsubs=*key;
				key++;
				WriteChar('(');
				while (nsubs) {
					if (MSisNum(key)) {
						WriteMS(key);
					} else {
						WriteChar('"');
						WriteMS(key);
						WriteChar('"');
					}
					key+=*key+1;
					nsubs--;
					if (nsubs)
						WriteChar(',');
				}
				WriteChar(')');
			}
			WriteChar(' ');
			ht(15);
			WriteChar('"');
			WriteMS(value);
			WriteChar('"');
			WriteChar(LF);
		}
	}
}

/**********  Compares two variable keys, returns -1 0 1  **********/
int KeyCmp(char* key1, char* key2)
{
	int res;
	int subs1, subs2;

	// Compare the variable names
	key1++;
	key2++;
	res=MStrCmp(key1, key2);
	if (!res) {		// Variable names match, check the subscripts
		key1+=*key1+1;
		key2+=*key2+1;
		if (*key1==0&&*key2>0) {	// Checks the number of subscripts
			res=-1;
			goto bail;
		}
		if (*key2==0&&*key1>0) {
			res=1;
			goto bail;
		}
		// Both have subscripts, compare them
		subs1=*key1;
		subs2=*key2;
		key1++;
		key2++;
		while (subs1>0&&subs2>0) {
			res=MStrCmp(key1, key2);
			if (!res) {		// These match, skip to the next
				key1+=*key1+1;
				key2+=*key2+1;
				subs1--;
				subs2--;
			} else {
				break;
			}
		}
		if (subs1==0&&subs2>0)
			res=-1;
		if (subs2==0&&subs1>0)
			res=1;
	}

bail:
	return(res);
}

/**********  Kills a variable from the symbol table  **********/
ErrCode VarKill(char* name, char* sub)
{
	char bkey[LINESIZE];
	char* key=bkey;
	char bkey2[LINESIZE];
	char* key2=bkey2;

	char* ps=SymTbl;

	int res;
	int sz;
	int szkey;
	int offkey;
	int offval;

	key++;	// Build key and compute key size
	MStrMove(name, key);
	szkey=*name+1;
	key+=*name+1;
	*key=*sub;	// Move the number of subscripts into the key
	key++;
	szkey++;
	if (*sub) {
		sz=*sub;
		sub++;
		while (sz) {
			MStrMove(sub, key);
			szkey+=*sub+1;
			sub+=*sub+1;
			key+=*key+1;
			sz--;
		}
	}
	offkey=szkey+1;
	// offval=0;
	key=bkey;	// Point back to the start of the key
	*key=szkey;

	if (*SymTbl!=0) {	// Symbol table is not empty
		while (*ps!=0) {	// Repeat until reaching the end of the symbol table
			MStrMove(ps, key2);
			res=KeyCmp(key, key2);
			if (res==-1)	// Key2 is bigger, var doesn't exist, bail out
				goto bail;
			if (res==0) {	// Found it! remove
				ps+=offkey;	// Skip to the value;
				offval=*ps;
				offval++;
				ps+=offval;
				sz=offkey+offval;	// Compute the size of the removal
				MemSlide(ps, pSymTblMax, -sz);	// Slide the memory left
				pSymTblMax-=sz;
				*pSymTblMax=0;
				goto bail;
			}
			// Key is smaller, skip to the next
			ps+=*key2+1;	// Skip the key
			ps+=*ps+1;		// Skip the value
		}
	}

bail:
	return(eNoErr);
}

/**********  Gets a variable from the symbol table  **********/
ErrCode VarGet(char* name, char* sub, char* value)
{
	ErrCode error=eNoErr;

	char bkey[LINESIZE];
	char* key=bkey;
	char bkey2[LINESIZE];
	char* key2=bkey2;

	char* ps=SymTbl;

	int res;
	int sz;
	int szkey;
	int offkey;

	key++;	// Build key and compute key size
	MStrMove(name, key);
	szkey=*name+1;
	key+=*name+1;
	*key=*sub;	// Move the number of subscripts into the key
	key++;
	szkey++;
	if (*sub) {
		sz=*sub;
		sub++;
		while (sz) {
			MStrMove(sub, key);
			szkey+=*sub+1;
			sub+=*sub+1;
			key+=*key+1;
			sz--;
		}
	}
	offkey=szkey+1;
	key=bkey;	// Point back to the start of the key
	*key=szkey;

	error=eNoVar;
	if (*SymTbl!=0) {	// Symbol table is not empty
		while (*ps!=0) {	// Repeat until reaching the end of the symbol table
			MStrMove(ps, key2);
			res=KeyCmp(key, key2);
			if (res==-1) {	// Key is bigger, var doesn't exist, bail out
//				error=eNoVar;
				goto bail;
			}
			if (res==0) {	// Found it! retrieve
				ps+=offkey;	// Skip to the value;
				MStrMove(ps, value);
				error=eNoErr;
				goto bail;
			}
			// New var key is smaller, skip to the next
			ps+=*key2+1;	// Skip the key
			ps+=*ps+1;		// Skip the value
		}
//	} else {
//		error=eNoVar;
	}

bail:
	return(error);
}

/**********  Sets a variable into the symbol table  **********/
ErrCode VarSet(char* name, char* sub, char* value)
{
	ErrCode error=eNoErr;

	char bkey[LINESIZE];
	char* key=bkey;
	char bkey2[LINESIZE];
	char* key2=bkey2;

	char* ps=SymTbl;

	int res;
	int sz;
	int szkey;
	int offkey;
	int offval;

	key++;	// Build key and compute key size
	MStrMove(name, key);
	szkey=*name+1;
	key+=*name+1;
	*key=*sub;	// Move the number of subscripts into the key
	key++;
	szkey++;
	if (*sub) {
		sz=*sub;
		sub++;
		while (sz) {
			MStrMove(sub, key);
			szkey+=*sub+1;
			sub+=*sub+1;
			key+=*key+1;
			sz--;
		}
	}
	offkey=szkey+1;
	offval=*value+1;
	key=bkey;	// Point back to the start of the key
	*key=szkey;

check:
	if (*ps==0) {	// Symbol table is empty
		MStrMove(key, ps);
		ps+=offkey;
		MStrMove(value, ps);
		pSymTblMax=ps+offval;	// Point to the next free position
		*pSymTblMax=0;	// "Close" the symbol table
	} else {
		while (*ps!=0) {	// Repeat until reaching the end of the symbol table
			MStrMove(ps, key2);
			res=KeyCmp(key, key2);
			if (res==-1) {	// New var key is smaller, insert
				sz=offkey+offval;	// compute the size of the insertion
				if ((pSymTblMax+sz)>pSymTblEnd) {
					error=eNoSym;
					goto bail;
				}
				MemSlide(ps, pSymTblMax, sz);
				pSymTblMax+=sz;
				*pSymTblMax=0;
				MemMove(key, ps, offkey);
				ps+=offkey;
				MemMove(value, ps, offval);
				goto bail;
			}
			if (res==0) {	// New var key is duplicated, delete then insert
				error=VarKill(name, sub);
				error=VarSet(name, sub, value);
				goto bail;
			}
			// New var key is bigger, skip to the next
			ps+=*key2+1;	// Skip the key
			ps+=*ps+1;		// Skip the value
		}
		goto check;
	}

bail:
	return(error);
}

#endif