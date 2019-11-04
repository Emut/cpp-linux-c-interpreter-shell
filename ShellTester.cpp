#include "CInterpreter.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
	char cpBuffer[2000];
	int nStatus = 0;
	CInterpreter::bSilentMode = false;

	while(true)
	{
		printf("->");	//shell prompt
		fgets(cpBuffer, 2000, stdin);	
		*strchr(cpBuffer, '\n') = 0;	//remove the newline at the end.
		if(*cpBuffer != 0)	//if user input is not empty
		{
			long long int llnRetVal = CInterpreter::CallFunctionWithArgs(cpBuffer, &nStatus);
			switch(nStatus)
			{
				case 0:
					printf("Call Returned:%lld\n", llnRetVal);
					break;
				case 1:
					printf("Unknown Symbol, Call Failed\n");
					break;
				case 2:	
					//printf("Function has multiple alternatives\n");
					break;
			}
			
		}
	}
}