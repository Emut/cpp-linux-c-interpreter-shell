#include "CIntrShell.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	char cpBuffer[200];

	while(true)
	{
		printf("->");	//shell prompt
		gets(cpBuffer);	//yea, i know the gets is evil but can we roll with it for once?
		if(*cpBuffer != 0)	//if user input is not empty
		{
			long long int llnRetVal = CIntrShell::CallFunctionWithArgs(cpBuffer);
			printf("Call Returned:%lld\n", llnRetVal);
		}
	}


	
}