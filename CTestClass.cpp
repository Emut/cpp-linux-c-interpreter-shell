#include "CTestClass.h"
#include <stdio.h>
#include <string.h>

void CTestClass::HiddenFunction()
{
	printf("You Have Found Me!\n");
}

int StringInputTester(char* par_cpString)
{
	printf("String input:%s\n", par_cpString);
	return strlen(par_cpString);
}

int DoubleInput(char* par_cpString, int par_nNumber)
{
	printf("String:%s Number:%d\n", par_cpString, par_nNumber);
}