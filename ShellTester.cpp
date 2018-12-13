#include "CIntrShell.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	char cpBuffer[20];
	CIntrShell::getProgramName(cpBuffer);
	printf("From outside:%s\n", cpBuffer);
	CIntrShell TestInstance("Test");
	TestInstance.CallFunction("HiddenFunc");
	
}