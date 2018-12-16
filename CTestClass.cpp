#include "CTestClass.h"
#include <stdio.h>

void CTestClass::HiddenFunction()
{
	printf("You Have Found Me!\n");
}

int StringInputTester(char* par_cpString)
{
	printf("String input:%s\n", par_cpString);
}