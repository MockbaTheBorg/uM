/**********  Expression Parser  **********/
#ifndef EXPR_H
#define EXPR_H

/**********  Returns the type of an expression atom  **********/
char aType(char ch)
{
	if (ch>47&&ch < 58)	// Is a number
		ch=aDigit;
	if ((ch>64&&ch<91)||(ch>96&&ch<123))	// Is a letter
		ch=aAlpha;
	return(ch);
}

/**********  Operand evaluation  **********/
ErrCode operSum(char* atom1, char* atom2)	// Sum operator (+)
{
	int val=MStrToNum(atom1)+MStrToNum(atom2);
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operSub(char* atom1, char* atom2)	// Subtration operator (-)
{
	int val=MStrToNum(atom1)-MStrToNum(atom2);
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operMul(char* atom1, char* atom2)	// Multiplication operator (*)
{
	int val=MStrToNum(atom1)*MStrToNum(atom2);
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operIntdiv(char* atom1, char* atom2)	// Integer division operator (\)
{
	ErrCode error=eNoErr;
	if (MStrToNum(atom2)!=0) {
		int val=MStrToNum(atom1)/MStrToNum(atom2);
		NumToMStr(val, atom1);
	} else {
		error=eDivZero;
	}
	return(error);
}

ErrCode operMod(char* atom1, char* atom2)	// Modulus operator (#)
{
	int val=MStrToNum(atom1)%MStrToNum(atom2);
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operCat(char* atom1, char* atom2)	// Concatenation operator (_)
{
	ErrCode error=eNoErr;
	uchar s1=*atom1;
	uchar s2=*atom2;
	char* p1=atom1+1;
	char* p2=atom2+1;

	if ((s1+s2+1)>LINESIZE) {
		error=eString;
	} else {
		p1+=s1;	// Position at the end of atom1
		while (s2>0) {
			*p1=*p2;
			p1++;
			p2++;
			s2--;
			s1++;
		}
		*atom1=s1;
	}
	return(error);
}

ErrCode operEqual(char* atom1, char* atom2, char fNot)	// Equal operator (=)
{
	int val=MStrCmp(atom1, atom2)==0;
	if (fNot) {
		val=!val;
	}
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operLT(char* atom1, char* atom2, char fNot)	// Less Than operator (<)
{
	int val=MStrCmp(atom1, atom2)<0;
	if (fNot) {
		val=!val;
	}
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operGT(char* atom1, char* atom2, char fNot)	// Greater Than operator (>)
{
	int val=MStrCmp(atom1, atom2)>0;
	if (fNot) {
		val=!val;
	}
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operAnd(char* atom1, char* atom2, char fNot)	// And operator (&)
{
	int val=MStrToNum(atom1)&&MStrToNum(atom2);
	if (fNot) {
		val=!val;
	}
	NumToMStr(val, atom1);
	return(eNoErr);
}

ErrCode operOr(char* atom1, char* atom2, char fNot)	// Or operator (!)
{
	int val=MStrToNum(atom1)||MStrToNum(atom2);
	if (fNot) {
		val=!val;
	}
	NumToMStr(val*fNot, atom1);
	return(eNoErr);
}

ErrCode operFollow(char* atom1, char* atom2, char fNot)	// Follow operator (])
{
	int val=MStrCmp(atom1, atom2)<1;
	if (fNot) {
		val=!val;
	}
	NumToMStr(val*fNot, atom1);
	return(eNoErr);
}

ErrCode operUNot(char* atom)	// Unary Not operator (')
{
	int val=!MStrToNum(atom);
	NumToMStr(val, atom);
	return(eNoErr);
}

/**********  Evaluates an expression atom  **********/
ErrCode exprAtom(char** p, char* patom)
{
	char MStrOne[2]={1, '1'};
	char MStrMinusOne[3]={2, '-', '1'};

	char varname[NAMESIZE];	// Buffer for the variable name
	char subs[LINESIZE];	// Buffer for the subscripts

	int error=eNoErr;
	char* pExpr=*p;
	int sz=0;
	char* pa=patom+1;

	switch (aType(*pExpr)) {
		case '(':
			pExpr++;
			error=exprEval(&pExpr, patom);
			if (!error) {
				if (*pExpr==')') {
					pExpr++;
				} else {
					error=eUnmParen;
				}
			}
			break;
		case aUnaryPlus:
			pExpr++;
			error=exprAtom(&pExpr, patom);
			if (!error)
				error=operMul(patom, (char*)&MStrOne);
			break;
		case aUnaryMinus:
			pExpr++;
			error=exprAtom(&pExpr, patom);
			if (!error)
				error=operMul(patom, (char*)&MStrMinusOne);
			break;
		case aUnaryNot:
			pExpr++;
			error=exprAtom(&pExpr, patom);
			if (!error)
				error=operUNot(patom);
			break;
		case aDigit:
			while (IsDigit(*pExpr)&&sz<LINESIZE) {
				*pa=*pExpr;
				pa++;
				pExpr++;
				sz++;
			}
			*patom=sz;
			break;
		case aString:
			pExpr++;	// Skip the "
			while (*pExpr!=aString&&sz<LINESIZE) {
				if (*pExpr==EOL||*pExpr==EOP)
					break;
				*pa=*pExpr;
				pa++;
				pExpr++;
				sz++;
			}
			if (*pExpr!=aString) {
				error=eUnmString;
			}
			*patom=sz;
			if (*pExpr==EOL||*pExpr==EOP)
				error=eUnmString;
			if (*pExpr==aString)
				pExpr++;
			break;
		case aAlpha:
			error=GetVarName(&pExpr, varname);
			if (error)
				break;
			subs[0]=0;	// Clear the subscripts list
			if (*pExpr=='(') {
				error=GetSubs(&pExpr, subs);
				if (error)
					break;
			}
			error=VarGet(varname, subs, patom);
			break;
		case aFunction:
			error=GetFunction(&pExpr, patom);
			break;
		default:
			error=eIllExpr;
	}
	*p=pExpr;
	return(error);
}

/**********  Evaluates an expression pointed by pExec  **********/
ErrCode exprEval(char** p, char* atom1)
{
	char atom2[LINESIZE];

	ErrCode error=eNoErr;
	char* pExpr=*p;

	char fNot=0;	// Flag to reverse the operand (NOT operator)
	char oper;

	if (*pExpr==EOL||*pExpr==EOP) {
		error=eIllExpr;
		goto bail;
	}

	// First collect atom 1
	error=exprAtom(&pExpr, atom1);
	if (error)
		goto bail;
	if (*pExpr==EOS||
		*pExpr==EOL||
		*pExpr==LSS||
		*pExpr==')'||
		*pExpr==' ')
		goto bail;

	if (*pExpr=='\'') {	// Found a not operator
		fNot=1;		// Set the not
		pExpr++;	// Point to the new operator
	}

	while (!error) {
		oper=*pExpr;
		pExpr++;
		error=exprAtom(&pExpr, atom2);
		if (error)
			goto bail;
		switch (oper) {
			case '\'':
				error=eIllNot;
				break;
			case '+':
				if (fNot) {
					error=eIllNot;
				} else {
					error=operSum(atom1, atom2);
				}
				break;
			case '-':
				if (fNot) {
					error=eIllNot;
				} else {
					error=operSub(atom1, atom2);
				}
				break;
			case '*':
				if (fNot) {
					error=eIllNot;
				} else {
					error=operMul(atom1, atom2);
				}
				break;
			case '/':	// Decimal division not implemented (defaults to integer)
			case '\\':
				if (fNot) {
					error=eIllNot;
				} else {
					error=operIntdiv(atom1, atom2);
				}
				break;
			case '#':
				if (fNot) {
					error=eIllNot;
				} else {
					error=operMod(atom1, atom2);
				}
				break;
			case '_':
				if (fNot) {
					error=eIllNot;
				} else {
					error=operCat(atom1, atom2);
				}
				break;
			case'=':
				error=operEqual(atom1, atom2, fNot);
				break;
			case'<':
				error=operLT(atom1, atom2, fNot);
				break;
			case'>':
				error=operGT(atom1, atom2, fNot);
				break;
			case'&':
				error=operAnd(atom1, atom2, fNot);
				break;
			case'!':
				error=operOr(atom1, atom2, fNot);
				break;
			case']':
				error=operFollow(atom1, atom2, fNot);
				break;
			case EOS:
			case EOL:
			case LSS:
			case ')':
			case ' ':
				break;
			default:
				error=eIllChar;
		}
		if (error)
			break;
		if (*pExpr==EOS||
			*pExpr==EOL||
			*pExpr==LSS||
			*pExpr==')'||
			*pExpr==' ')
			break;
	}

bail:
	*p=pExpr;
	return(error);
}

/**********  Main entry point  **********/
ErrCode Expr(char** p, char* result)
{
	ErrCode error=eNoErr;
	char* pExpr=*p;

	error=exprEval(&pExpr, result);
	if (!error) {
		if (*pExpr==')')
			error=eUnmParen;
	}

	*p=pExpr;
	return(error);
}

#endif