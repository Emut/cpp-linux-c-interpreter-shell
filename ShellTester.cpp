#include "CIntrShell.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	char cpBuffer[2000];
	int nStatus = 0;
	CIntrShell::bSilentMode = true;

	while(true)
	{
		printf("->");	//shell prompt
		gets(cpBuffer);	//yea, i know the gets is evil but can we roll with it for once?
		if(*cpBuffer != 0)	//if user input is not empty
		{
			long long int llnRetVal = CIntrShell::CallFunctionWithArgs(cpBuffer, &nStatus);
			switch(nStatus)
			{
				case 0:
					printf("Call Returned:%lld\n", llnRetVal);
					break;
				case 1:
					printf("Unknown Symbol\n");
					break;
				case 2:	
					printf("Function has multiple alternatives\n");
					break;
			}
			
		}
	}
}