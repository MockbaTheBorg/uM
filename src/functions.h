/**********  M(umps) functions and special variables  **********/
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/**********  Gets the result of the function pointed by pExec  **********/
ErrCode GetFunction(char** p, char* result)
{
	ErrCode error=eNoErr;
	char* pExec=*p;

	*p=pExec;
	return(error);
}

#endif