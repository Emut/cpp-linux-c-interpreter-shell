#define MAX_PROGNAME_LENGTH 100 	//max length of the executable's name
#include "CSparseTrie.h"

class CInterpreter
{

public:
	static CInterpreter* getInstance();
	//CIntrShell(char* par_cpExecutableName);
	//~CIntrShell();
	long long int CallFunctionWithArgs(char* par_cpInput, int* par_npStatus = 0);
	void RegisterFunction(const char* par_cpFuncName, void* par_vpFuncAddress);

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
	void* SearchRegisteredFunction(const char* par_cpFuncName);
	void* SearchExecutableForFunction(const char* par_cpFuncName, int* par_npStatus);


	char cpProgramName[MAX_PROGNAME_LENGTH];
	CSparseTrie<void*> trieRegisteredFunctions;
	bool bIsStreamOpen;
	FILE* filepShell;

	static int lkup(); 

	CInterpreter();	//go singleton
};