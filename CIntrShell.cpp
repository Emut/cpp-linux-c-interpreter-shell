#include "CIntrShell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>


CIntrShell::CIntrShell(char* par_cpExecutableName)
{
	sprintf(cpExecutableName, "%s", par_cpExecutableName);
}


long long int CIntrShell::CallFunction(char* par_cpFuncName)
{
	void* vpFuncAddress = getFuncAddressByName(par_cpFuncName);
	void(*f)();
	f = (void(*)())vpFuncAddress;
	f();
}



void* CIntrShell::getFuncAddressByName(char* par_cpFuncName)
{
	char cpCommand[200];
	sprintf(cpCommand, "nm %s -C| grep %s", cpExecutableName, par_cpFuncName);
	
	char cpOutputBuffer[2000];
	ShellIO(cpCommand, cpOutputBuffer, 2000);

	long long int llnFuncAddress = 0;
	sscanf(cpOutputBuffer, "%x", &llnFuncAddress);
	printf("Function %s is at 0x%llx\n", par_cpFuncName, llnFuncAddress);
	return (void*)llnFuncAddress;
}

int CIntrShell::ShellIO(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength)
{
	FILE* filepShell = 0;

	filepShell = popen(par_cpInput, "r");
	if(filepShell == NULL)
	{
		printf("CIntrShell::ShellIO:Error running shell cmd\n");
		return -1;
	}
	int nRetVal = fread(par_cpOutput, 1, par_nMaxOutputLength -1 , filepShell);
	par_cpOutput[nRetVal] = 0;	//add null terminator
	printf("Shell Output:%s\n", par_cpOutput);

	pclose(filepShell);
	return nRetVal;
}

bool CIntrShell::getProgramName(char* par_cpProgramName)
{
	char cpPath[PATH_MAX];
	if(readlink("/proc/self/exe", cpPath, PATH_MAX) < 0)
	{
		printf("CIntrShell::getProgramName:Can not get the program name!\n");
		par_cpProgramName = NULL;
		return false;
	}
	char* cpName = strrchr(cpPath, '/') + 1;	//last '/' + 1 points to program name
	if(strcpy(par_cpProgramName, cpName) == NULL)
	{
		printf("CIntrShell::getProgramName:Can not get the program name!\n");
		return false;
	}
	printf("ProgramName:%s\n", par_cpProgramName);
	return true;
}