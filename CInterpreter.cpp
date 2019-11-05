#include "CInterpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define MAX_ARG_COUNT 10	//maximum number of args shell command can have
#define MAX_FUNCNAME_LENGTH 100		//max length of function name
#define MAX_SHELL_LINE_LENGTH 500	//used by getFuncAddressByName
#define MAX_STRINGARG_LENGTH 1000	//max length of all string arguments combined


CInterpreter* CInterpreter::getInstance(){
	//MUTEX->LOCK
	static CInterpreter* theInstance = new CInterpreter();
	return theInstance;
	//MUTEX->UNLOCK	//this function should be guarded in case of calls from multiple threads  
}

CInterpreter::CInterpreter(){
	bSilentMode = false;
	bool bIsStreamOpen = false;
	FILE* filepShell = 0;
	getProgramName(cpProgramName);

	RegisterFunction("printf", (void*)printf); //added these 3 funcs
	RegisterFunction("malloc", (void*)malloc); //they are located on shared libs
	RegisterFunction("free", (void*)free);	   //therefore won't appear within the executable	
}

void* CInterpreter::getFuncAddressByName(char* par_cpFuncName, int* par_npStatus)
{
	void* vpRegisteredFunc = SearchRegisteredFunction(par_cpFuncName);
	if(vpRegisteredFunc != NULL){
		if(par_npStatus)
			*par_npStatus = 0;
		return vpRegisteredFunc;
	}

	void* vpInternalFunc = SearchExecutableForFunction(par_cpFuncName, par_npStatus);
	return vpInternalFunc;

}

int CInterpreter::ShellIO_Line(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength)
{

	if(par_nMaxOutputLength < 1)	//close signal
	{
		if(bIsStreamOpen)
		{
			pclose(filepShell);
			bIsStreamOpen = false;
		}
		return 0;
	}
	if(!bIsStreamOpen)
	{

		filepShell = popen(par_cpInput, "r");
		if(filepShell == NULL)
		{
			printf("CInterpreter::ShellIO:Error running shell cmd\n");
			return -2;
		}
		bIsStreamOpen = true;
	}

	int nCharCount = 0;
	while(true)
	{
		char cRead = getc(filepShell);
		if(cRead == EOF)
		{
			par_cpOutput[nCharCount] = 0;
			pclose(filepShell);
			bIsStreamOpen = false;
			if(nCharCount == 0)
				return -1;
			return 0;	//return 0 if no more lines
		}
		if(cRead == '\n')	//if end of line
		{
			par_cpOutput[nCharCount] = 0;
			cRead = getc(filepShell);	//check if next char is EOF
			if(cRead == EOF)
			{
				pclose(filepShell);
				bIsStreamOpen = false;
				return 0;	//return 0 if no more lines
			}
			fseek(filepShell, -1, SEEK_CUR);	//if next char is not EOF, rewind the stream by one char
			return 1;	//return 1 if there are more lines
		}
		par_cpOutput[nCharCount++] = cRead;
	}
}

bool CInterpreter::getProgramName(char* par_cpProgramName)
{
	#if __linux__
		char cpPath[PATH_MAX];
		if(readlink("/proc/self/exe", cpPath, PATH_MAX) < 0)
		{
			printf("CInterpreter::getProgramName:Can not get the program name!\n");
			par_cpProgramName = NULL;
			return false;
		}
		char* cpName = strrchr(cpPath, '/') + 1;	//last '/' + 1 points to program name
		if(strcpy(par_cpProgramName, cpName) == NULL)
		{
			printf("CInterpreter::getProgramName:Can not get the program name!\n");
			return false;
		}
		return true;
	#else
		return false;	//Parsing the symbols within the executable is not implemented for 
						//other platforms yet
	#endif
}

int CInterpreter::ParseShellCommand(char* par_cpShellCommand, char* par_cpFuncName, char* par_cpStringLiteralArgs, long long int* par_llnpNumericArgs, teArgumentType* par_epArgTypes)
{
	char* cpParseIndex = par_cpShellCommand;
	int nStringArgBufferOffset = 0;
	par_cpStringLiteralArgs[0] = 0;
	if(!ParseFuncName(&cpParseIndex, par_cpFuncName))
		return -1;
	bool bIsFuncParenWrapped = false;
	for(int nArgIndex = 0; nArgIndex < MAX_ARG_COUNT;)
	{
		if(*cpParseIndex == 0)
		{
			//printf("End of the ShellCommand\n");
			return nArgIndex;
		}
		else if(*cpParseIndex == ')' && bIsFuncParenWrapped)
		{
			//printf("End of the ShellCommand with paren\n");
			return nArgIndex;
		}
		else if(*cpParseIndex == ' ' || *cpParseIndex == ',')
		{
			//printf("WS or comma\n");
			cpParseIndex++;	//Ignore white space and commas
			continue;
		}
		else if(*cpParseIndex == '"')	//start of a string literal
		{
			//printf("String beginning\n");
			ParseStringArgument(&cpParseIndex, par_cpStringLiteralArgs, &nStringArgBufferOffset);
			par_epArgTypes[nArgIndex] = ARG_STRING;
			nArgIndex++;
			continue;
		}
		else if(*cpParseIndex >= '0' && *cpParseIndex <= '9')	//start of a number
		{
			//printf("Number beginning\n");
			ParseNumericArgument(&cpParseIndex, par_llnpNumericArgs + nArgIndex);
			par_epArgTypes[nArgIndex] = ARG_NUMBER;
			nArgIndex++;
			continue;
		}
		else
		{
			if (*cpParseIndex == '(' && nArgIndex == 0)	//If still not seen first arg and open paren
			{
				bIsFuncParenWrapped = true;
				cpParseIndex++;
				continue;
			}
			printf("CInterpreter::ParseShellCommand:Invalid syntax at char %c\n", *cpParseIndex);
			return -1;
		}
	}
}

bool CInterpreter::ParseFuncName(char** par_cppShellCommand, char* par_cpFuncName)
{
	char* cpParseIndex = *par_cppShellCommand;
	char* cpCopyIndex = par_cpFuncName;
	bool bArgumentTypeActive = false;
	while(true)
	{
		if(*cpParseIndex == '(' || (*cpParseIndex == ' ' && !bArgumentTypeActive) || *cpParseIndex == 0)
		{	//' ' is ok if an argument type is being read
			*cpCopyIndex = 0; 	//terminate the string
			*par_cppShellCommand = cpParseIndex;
			return true;
		}
		else if((*cpParseIndex >= '0' && *cpParseIndex <= '9')
				|| (*cpParseIndex >= 'A' && *cpParseIndex <= 'Z')
				|| (*cpParseIndex >= 'a' && *cpParseIndex <= 'z')
				|| (*cpParseIndex == '_') || (*cpParseIndex == ':'))	//if alphanumeric or underscore or scope 
		{
			*cpCopyIndex++ = *cpParseIndex++;	//copy and move indices
		}
		else if((*cpParseIndex == '*' || *cpParseIndex == ' ' || *cpParseIndex == ',') && bArgumentTypeActive)
			*cpCopyIndex++ = *cpParseIndex++;	//copy and move indices. For argument type ' ', ',' and '*' are ok
		else if(*cpParseIndex == '<' && !bArgumentTypeActive){
			bArgumentTypeActive = true;
			*cpCopyIndex++ = '(';	//replace '<' with '(' and move indices. This is used to pass input types with function name
			*cpParseIndex++;		//in cases with multiple alternative
		}
		else if(*cpParseIndex == '>' && bArgumentTypeActive){
			bArgumentTypeActive = false;
			*cpCopyIndex++ = ')';	//same with the above case
			*cpParseIndex++;		
		}
		else
		{
			printf("CInterpreter::ParseFuncName:Unexpecdted char value\n %d", *cpParseIndex);
			return false;
		}
	}
}

bool CInterpreter::ParseStringArgument(char** par_cppShellCommand, char* par_cpStringLiteralArgs, int* par_npStringBufferOffset)
{
	char* cpParseIndex = *par_cppShellCommand;
	cpParseIndex++;
	char* cpCopyIndex = par_cpStringLiteralArgs + *par_npStringBufferOffset;
	while(true)
	{
		if(*cpParseIndex == 0)	//if reached end of the input string
		{
			printf("CInterpreter::ParseStringArgument:Invalid syntax\n");
			return false;
		}
		if(*cpParseIndex == '"')	//if reached end of the arg string
		{
			*par_cppShellCommand = ++cpParseIndex;	//move passed ParseIndex to one past end '"'
			*cpCopyIndex = 0;
			(*par_npStringBufferOffset)++;	//add terminator and move forward
			return true;
		}
		*cpCopyIndex++ = *cpParseIndex++;	//copy and move both
		(*par_npStringBufferOffset)++;
	}
}

bool CInterpreter::ParseNumericArgument(char** par_cppShellCommand, long long int* par_llnpResult)
{
	char* cpParseIndex = *par_cppShellCommand;
	*par_llnpResult = 0;
	while(true)
	{
		if(*cpParseIndex >= '0' && *cpParseIndex <= '9')	//if a number
		{
			*par_llnpResult *= 10;
			*par_llnpResult += *cpParseIndex++ - 48; //asciinumber-48=number
		}
		else	//anything not a number concludes parsing
		{
			*par_cppShellCommand = cpParseIndex; 
			return true;
		}
	}
}

long long int CInterpreter::CallFunctionWithArgs(char* par_cpInput, int* par_npStatus)
{
	if(par_cpInput == NULL || *par_cpInput == 0)
		return 0;
	char cpFunctionName[MAX_FUNCNAME_LENGTH];
	char cpStringLiterals[MAX_STRINGARG_LENGTH];
	long long int llnpNumericArgs[MAX_ARG_COUNT];
	teArgumentType epArgumentTypes[MAX_ARG_COUNT];

	int nNumberOfArgs = ParseShellCommand(par_cpInput, cpFunctionName, cpStringLiterals, llnpNumericArgs, epArgumentTypes);

	char* cpPrintIndex = cpStringLiterals;

	#if ENABLE_DEBUG_PRINTS
	printf("Input:%s\n", par_cpInput);
	printf("FuncName:%s\n", cpFunctionName);
	printf("Number of Args:%d\n", nNumberOfArgs);

	for(int nInd = 0; nInd < nNumberOfArgs; nInd++)
	{
		printf("Arg%d::", nInd);
		if(epArgumentTypes[nInd] == ARG_NUMBER)
		{
			printf("Type:Number Value:%lld\n", llnpNumericArgs[nInd]);
		}
		if(epArgumentTypes[nInd] == ARG_STRING)
		{
			printf("Type:StringLiteral Value:%s\n", cpPrintIndex);
			cpPrintIndex += strlen(cpPrintIndex) + 1;
		}
	}
	#endif
	//place all arguments in llnpNumericArgs array
	cpPrintIndex = cpStringLiterals;
	for (int nInd = 0; nInd < MAX_ARG_COUNT; nInd++)
	{
		if(nInd < nNumberOfArgs)
		{
			if(epArgumentTypes[nInd] == ARG_NUMBER)	//numbers are already in the array
				continue;
			else if(epArgumentTypes[nInd] == ARG_STRING)
			{
				llnpNumericArgs[nInd] = (long long int)cpPrintIndex;
				cpPrintIndex += strlen(cpPrintIndex) + 1;
			}
		}
		else
		{
			llnpNumericArgs[nInd] = 0;
		}
	}

	void* vpFunctionAddress = getFuncAddressByName(cpFunctionName, par_npStatus);
	ShellIO_Line(NULL, NULL, -1);	//close and reset ShellIO
	if(vpFunctionAddress == NULL)
		return 0;

	long long int(*funcpFunctionToCall)(...);	//declare a func pointer taking a variable num of inputs and returning a long long int
	funcpFunctionToCall = (long long int(*)(...))vpFunctionAddress;	//cast the function address into its type
	long long int llnRetVal = 0;

	switch (nNumberOfArgs)
	{
		case 0:
			llnRetVal = funcpFunctionToCall();
			break;
		case 1:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0]);
			break;
		case 2:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1]);
			break;
		case 3:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2]);
			break;
		case 4:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3]);
			break;
		case 5:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3],
						llnpNumericArgs[4]);
			break;
		case 6:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3],
						llnpNumericArgs[4],
						llnpNumericArgs[5]);
			break;
		case 7:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3],
						llnpNumericArgs[4],
						llnpNumericArgs[5],
						llnpNumericArgs[6]);
			break;
		case 8:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3],
						llnpNumericArgs[4],
						llnpNumericArgs[5],
						llnpNumericArgs[6],
						llnpNumericArgs[7]);
			break;
		case 9:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3],
						llnpNumericArgs[4],
						llnpNumericArgs[5],
						llnpNumericArgs[6],
						llnpNumericArgs[7],
						llnpNumericArgs[8]);
			break;
		case 10:
			llnRetVal = funcpFunctionToCall(
						llnpNumericArgs[0],		//this is not nice. Need to find a way to call a variadic func with variable number of args in runtime
						llnpNumericArgs[1],
						llnpNumericArgs[2],
						llnpNumericArgs[3],
						llnpNumericArgs[4],
						llnpNumericArgs[5],
						llnpNumericArgs[6],
						llnpNumericArgs[7],
						llnpNumericArgs[8],
						llnpNumericArgs[9]);
			break;
	
		default:
			break;
	}
 
	if(!CInterpreter::bSilentMode)
		printf("Call Returned:%lld\n", llnRetVal);
	return llnRetVal;
}

void CInterpreter::RegisterFunction(const char* par_cpFuncName, void* par_vpFuncAddress){
	trieRegisteredFunctions[par_cpFuncName] = par_vpFuncAddress;
}

void* CInterpreter::SearchRegisteredFunction(const char* par_cpFuncName){
	CSparseTrie<void*>* triepRetVal = trieRegisteredFunctions.Find(par_cpFuncName);	//returns null
	if(triepRetVal == NULL)
		return NULL;

	return *(triepRetVal->pTData);
}

int CInterpreter::lkup(){
	printf("CInterpreter::Registered Functions:\n");
	getInstance()->trieRegisteredFunctions.PrintKeys();
	return getInstance()->trieRegisteredFunctions.Size();
}

void* CInterpreter::SearchExecutableForFunction(const char* par_cpFuncName, int* par_npStatus){
	char cpCommand[MAX_FUNCNAME_LENGTH + 50];
	if(strchr(par_cpFuncName, '(') == NULL) //If there is no '(', function name does not need argument type matching
		sprintf(cpCommand, "nm %s -C| grep [:+[:space:]]%s\\(", cpProgramName, par_cpFuncName); //func name is preceeded by a space or ':'
	else
		sprintf(cpCommand, "nm %s -C| grep -F \'%s\'", cpProgramName, par_cpFuncName); //func name is preceeded by a space of ':'
		//sprintf(cpCommand, "nm %s -C| grep -F \'[:+[:space:]]%s\'", cpProgramName, par_cpFuncName); //func name is preceeded by a space of ':'

	char cpOutputBuffer[MAX_SHELL_LINE_LENGTH];
	bool bSingleAlternative = false;
	long long int llnFuncAddress = 0;
	
	//printf("%s\n", cpCommand);
	int nRetVal = ShellIO_Line(cpCommand, cpOutputBuffer, MAX_SHELL_LINE_LENGTH);
	//printf("ShellIO_Line output:%s, retval:%d\n", cpOutputBuffer, nRetVal); //temptemptmep, I am here for debuging!
	if(nRetVal > 0){
		if(par_npStatus != NULL)
			*par_npStatus = 2;
		if(!CInterpreter::bSilentMode){
			printf("Function has multiple alternatives:\n");
			do{	//print said alternatives
				printf("%s\n", strstr(cpOutputBuffer, par_cpFuncName));
				if(nRetVal == 0)
					break;
				nRetVal = ShellIO_Line(cpCommand, cpOutputBuffer, MAX_SHELL_LINE_LENGTH);
				
			}while(true);
		}
		return NULL;
	}
	if(nRetVal < 0){
		if(par_npStatus != NULL){
			*par_npStatus = 1;
		}
		return 0;
	}
	
	sscanf(cpOutputBuffer, "%llx", &llnFuncAddress);
		
		
	
	if(par_npStatus != NULL)
		if(*par_npStatus != 2)
			if(llnFuncAddress == 0)
				*par_npStatus = 1;
			else
				*par_npStatus = 0;
	ShellIO_Line(NULL, NULL, -1);	//close and reset ShellIO
	//printf("Function %s is at 0x%llx\n", par_cpFuncName, llnFuncAddress);
	return (void*)llnFuncAddress;	
}