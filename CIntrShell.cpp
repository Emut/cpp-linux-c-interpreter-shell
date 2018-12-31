#include "CIntrShell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define MAX_ARG_COUNT 10	//maximum number of args shell command can have
#define MAX_PROGNAME_LENGTH 100 	//max length of the executable's name
#define MAX_FUNCNAME_LENGTH 100		//max length of function name
#define MAX_SHELL_LINE_LENGTH 500	//used by getFuncAddressByName
#define MAX_STRINGARG_LENGTH 1000	//max length of all string arguments combined
/*	//Constructor is not needed at the moment
CIntrShell::CIntrShell(char* par_cpExecutableName)
{
	sprintf(cpExecutableName, "%s", par_cpExecutableName);
}
*/
bool CIntrShell::bSilentMode = false;

void* CIntrShell::getFuncAddressByName(char* par_cpFuncName, int* par_npStatus)
{
	static char cpProgramName[MAX_PROGNAME_LENGTH];
	static bool bFirstRun = true;
	if(bFirstRun)
	{
		getProgramName(cpProgramName);
		bFirstRun = false;
	}

	char cpCommand[MAX_FUNCNAME_LENGTH + 50];
	sprintf(cpCommand, "nm %s -C| grep %s", cpProgramName, par_cpFuncName);
	
	char cpOutputBuffer[MAX_SHELL_LINE_LENGTH];
	bool bSingleAlternative = false;
	long long int llnFuncAddress = 0;
	while(true)
	{
		int nRetVal = ShellIO_Line(cpCommand, cpOutputBuffer, MAX_SHELL_LINE_LENGTH);
		if(nRetVal < 0)
			break;	//nRetVal < 0 means error in shellIO
		char* cpOpenParen = strchr(cpOutputBuffer, '(');
		if(cpOpenParen == NULL)
			if(nRetVal == 0)
				break;
			else
				continue;	//line does not include a function
		int nFuncNameLength = strlen(par_cpFuncName);
		if(cpOpenParen - cpOutputBuffer >= nFuncNameLength)
		{
			cpOpenParen -= nFuncNameLength;
			if(strstr(cpOpenParen, par_cpFuncName) == cpOpenParen)
			{
				if(!bSingleAlternative)
				{
					sscanf(cpOutputBuffer, "%llx", &llnFuncAddress);
					bSingleAlternative = true;
				}
				else
				{
					if(par_npStatus != NULL)
						*par_npStatus = 2;
					break;	//there are more than one alternatives
				}
			}
		}
		if(nRetVal == 0)
			break;
	}
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

int CIntrShell::ShellIO(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength)
{
	FILE* filepShell = 0;

	filepShell = popen(par_cpInput, "r");
	if(filepShell == NULL)
	{
		printf("CIntrShell::ShellIO:Error running shell cmd\n");
		return -1;
	}
	int nRetVal = fread(par_cpOutput, 1, par_nMaxOutputLength -1 , filepShell);
	par_cpOutput[nRetVal] = 0;	//add null terminator
	printf("Shell Output:%s\n", par_cpOutput);

	pclose(filepShell);
	return nRetVal;
}

int CIntrShell::ShellIO_Line(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength)
{
	static bool bIsStreamOpen = false;
	static FILE* filepShell = 0;
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
			printf("CIntrShell::ShellIO:Error running shell cmd\n");
			return -1;
		}
		bIsStreamOpen = true;
	}

	int nCharCount = 0;
	while(true)
	{
		char cRead = getc(filepShell);
		if(cRead == EOF)
		{
			//printf("EOF\n");
			par_cpOutput[nCharCount] = 0;
			pclose(filepShell);
			bIsStreamOpen = false;
			return 0;	//return 0 if no more lines
		}
		if(cRead == '\n')	//if end of line
		{
			par_cpOutput[nCharCount] = 0;
			cRead = getc(filepShell);	//check if next char is EOF
			if(cRead == EOF)
			{
				//printf("EOFFF\n");
				pclose(filepShell);
				bIsStreamOpen = false;
				return 0;	//return 0 if no more lines
			}
			//printf("EOL\n");
			fseek(filepShell, -1, SEEK_CUR);	//if next char is not EOF, rewind the stream by one char
			return 1;	//return 1 if there are more lines
		}
		par_cpOutput[nCharCount++] = cRead;
	}
}

bool CIntrShell::getProgramName(char* par_cpProgramName)
{
	char cpPath[PATH_MAX];
	if(readlink("/proc/self/exe", cpPath, PATH_MAX) < 0)
	{
		printf("CIntrShell::getProgramName:Can not get the program name!\n");
		par_cpProgramName = NULL;
		return false;
	}
	char* cpName = strrchr(cpPath, '/') + 1;	//last '/' + 1 points to program name
	if(strcpy(par_cpProgramName, cpName) == NULL)
	{
		printf("CIntrShell::getProgramName:Can not get the program name!\n");
		return false;
	}
	return true;
}

int CIntrShell::ParseShellCommand(char* par_cpShellCommand, char* par_cpFuncName, char* par_cpStringLiteralArgs, long long int* par_llnpNumericArgs, teArgumentType* par_epArgTypes)
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
			printf("CIntrShell::ParseShellCommand:Invalid syntax at char %c\n", *cpParseIndex);
			return -1;
		}
	}
}

bool CIntrShell::ParseFuncName(char** par_cppShellCommand, char* par_cpFuncName)
{
	char* cpParseIndex = *par_cppShellCommand;
	char* cpCopyIndex = par_cpFuncName;
	while(true)
	{
		if(*cpParseIndex == '(' || *cpParseIndex == ' ' || *cpParseIndex == 0)
		{
			*cpCopyIndex = 0; 	//terminate the string
			*par_cppShellCommand = cpParseIndex;
			return true;
		}
		else if((*cpParseIndex >= '0' && *cpParseIndex <= '9')
				|| (*cpParseIndex >= 'A' && *cpParseIndex <= 'Z')
				|| (*cpParseIndex >= 'a' && *cpParseIndex <= 'z')
				|| (*cpParseIndex == '_'))	//if alphanumeric or underscore
		{
			*cpCopyIndex++ = *cpParseIndex++;	//copy and move indices
		}
		else
		{
			printf("CIntrShell::ParseFuncName:Unexpecdted char value");
			return false;
		}
	}
}

bool CIntrShell::ParseStringArgument(char** par_cppShellCommand, char* par_cpStringLiteralArgs, int* par_npStringBufferOffset)
{
	//*par_cppShellCommand points to '"'
	//printf("ParseStringArgument\n");
	char* cpParseIndex = *par_cppShellCommand;
	cpParseIndex++;
	char* cpCopyIndex = par_cpStringLiteralArgs + *par_npStringBufferOffset;
	while(true)
	{
		if(*cpParseIndex == 0)	//if reached end of the input string
		{
			printf("CIntrShell::ParseStringArgument:Invalid syntax\n");
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

bool CIntrShell::ParseNumericArgument(char** par_cppShellCommand, long long int* par_llnpResult)
{
	//printf("ParseNumericArgument\n");
	char* cpParseIndex = *par_cppShellCommand;
	*par_llnpResult = 0;
	while(true)
	{
		if(*cpParseIndex >= '0' && *cpParseIndex <= '9')	//if a number
		{
			//printf("%s ", cpParseIndex);
			*par_llnpResult *= 10;
			*par_llnpResult += *cpParseIndex++ - 48; //asciinumber-48=number
		}
		else	//anything not a number concludes parsing
		{
			*par_cppShellCommand = cpParseIndex; 
			return true;
		}
		/*
		if(*cpParseIndex == 0 || *cpParseIndex == ' ' || *cpParseIndex == ',')	//if string ended or space or comma, means end of number 
		{
			*par_cppShellCommand = cpParseIndex; 
			return true;
		}
		
		else	//non-number in number input
		{
			printf("CIntrShell::ParseNumericArgument:Invalid syntax\n");
			return false;
		}
		*/
	}
}

long long int CIntrShell::CallFunctionWithArgs(char* par_cpInput, int* par_npStatus)
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
	if(vpFunctionAddress == NULL)
		return 0;

	long long int(*funcpFunctionToCall)(...);	//declare a func pointer taking a variable num of inputs and returning a long long int
	funcpFunctionToCall = (long long int(*)(...))vpFunctionAddress;	//cast the function address into its type
	long long int llnRetVal = 0;
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
	if(!CIntrShell::bSilentMode)
		printf("Returned:%lld\n", llnRetVal);
	return llnRetVal;
}