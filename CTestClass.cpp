#include "CTestClass.h"
#include <stdio.h>
#include <string.h>

void CTestClass::HiddenFunction()
{
	printf("You Have Found Me!\n");
}

int CTestClass::getNumber(int par_nNumber){
	printf("Number:%d\n", par_nNumber);
	return par_nNumber;
}

void CAnotherTestClass::HiddenFunction(){
	printf("You Have Found Me Out Of Scope!\n");
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

void PrintValue(bool par_bNumber)
{
	printf("Bool:%s\n", par_bNumber?"true":"false");
}

void PrintValue()
{
	printf("No argument there..\n");
}

int PrintValue(int par_nNumber, char* par_cpString)
{
	printf("Number:%d, String:%s\n", par_nNumber,  par_cpString);
	return par_nNumber;
}

void BoolAndInteger(bool par_bTest, int par_nTest)
{
	printf("Bool:%s Integer:%d\n", par_bTest?"true":"false", par_nTest);
}