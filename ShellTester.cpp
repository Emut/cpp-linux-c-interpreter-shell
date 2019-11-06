#include "CInterpreter.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
	char cpBuffer[2000];
	CInterpreter::teCallReturn eStatus = CInterpreter::CALL_OK;
	CInterpreter::getInstance()->bSilentMode = false;

	while(true)
	{
		printf("->");	//shell prompt
		fgets(cpBuffer, 2000, stdin);	
		*strchr(cpBuffer, '\n') = 0;	//remove the newline at the end.
		if(*cpBuffer != 0)	//if user input is not empty
		{
			long long int llnRetVal = CInterpreter::getInstance()->CallFunctionWithArgs(cpBuffer, &eStatus);
		}
	}
}