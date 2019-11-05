/*
* CInterpreter
* Umut Ekici 2019
*
* Inspired by the vxworks c interpreter shell. 
* Call functions and create variables via the interpreter.
* Feed the interpreter from terminal or a telnet client for rapid prototyping and testing especially in embedded environments.
*
* Functions in the executable is parsed (only on linux for now) via nm and grep. Therefore it depends on nm and grep.
* Additionally symbol names should be present in the executable (i.e. must not be strip'ed)
*
* Dynamicly loadded functions (e.g. printf, malloc...) are not present in the executable. They can be added by
* RegisterFunction(..) to be referenced later on.  
*
* Interpreter has a single entry point: CallFunctionWithArgs.
* CAUTION: This undoes 20 years of development effort on compilers. There is no type checking or even argument number checking.
* User is one letter away from invoking Undefined Behavior
*
* Class provides 2 additional functions SearchRegisteredFunction(..) and SearchExecutableForFunction(..)
* These functions can be used for obtaining function address, which then can be used with a proper function pointer cast,
* as in the case of dynamic loading.  
*/

#define MAX_PROGNAME_LENGTH 100 	//max length of the executable's name
#include "CSparseTrie.h"

class CInterpreter
{

public:
	static CInterpreter* getInstance();

	long long int CallFunctionWithArgs(char* par_cpInput, int* par_npStatus = 0);
	void RegisterFunction(const char* par_cpFuncName, void* par_vpFuncAddress);
	void* SearchRegisteredFunction(const char* par_cpFuncName);
	void* SearchExecutableForFunction(const char* par_cpFuncName, int* par_npStatus);

	bool bSilentMode;	//If false, return value of the function call is outputted at the terminal.

private:		//To be changed to private after class is completed!

	enum teArgumentType
	{
		ARG_NUMBER = 0,
		ARG_STRING = 1
	};
	void* getFuncAddressByName(char* par_cpFuncName, int* par_npStatus = 0); 
	int ShellIO(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength);	//not used atm
	int ShellIO_Line(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength);
	bool getProgramName(char* par_cpProgramName);
	int ParseShellCommand(char* par_cpShellCommand, char* par_cpFuncName, char* par_cpStringLiteralArgs, long long int* par_llnpNumaricArgs, teArgumentType* par_epArgTypes);
	bool ParseFuncName(char** par_cppShellCommand, char* par_cpFuncName);
	bool ParseStringArgument(char** par_cppShellCommand, char* par_cpStringLiteralArgs, int* par_npStringBufferOffset);
	bool ParseNumericArgument(char** par_cppShellCommand, long long int* par_llnpResult);
	


	char cpProgramName[MAX_PROGNAME_LENGTH];
	CSparseTrie<void*> trieRegisteredFunctions;
	bool bIsStreamOpen;
	FILE* filepShell;

	static int lkup(); 

	CInterpreter();	//go singleton
};