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
	return par_nNumber;
}

int* getNewNumber(int par_nNumber)
{
	int* npRetVal = new int;
	*npRetVal = par_nNumber;
	return npRetVal;
}

void PrintValue(int* par_npNumber)
{
	printf("Number:%d\n", *par_npNumber);
}

void PrintValue(int par_nNumber)
{
	printf("Number:%d\n", par_nNumber);
}

void BoolAndInteger(bool par_bTest, int par_nTest)
{
	printf("Bool:%s Integer:%d\n", par_bTest?"true":"false", par_nTest);
}