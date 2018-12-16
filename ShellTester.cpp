#include "CIntrShell.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	char cpBuffer[200];
	char cpBuffer2[200];
	//CIntrShell::getProgramName(cpBuffer);
	//printf("From outside:%s\n", cpBuffer);
	//CIntrShell TestInstance("Test");
	//CIntrShell::CallFunction("HiddenFunc");
	//CIntrShell::CallFunction("HiddenFunc");
	//CIntrShell::CallFunction("HiddenFunc");

	CIntrShell::CallFunctionWithArgs("HiMomma(\"asd\", 1, 123, \"kediler\", 58, \"\")");
	CIntrShell::CallFunctionWithArgs("Ceran  8 ,9");
	CIntrShell::CallFunctionWithArgs("Ayidolf");
	
}