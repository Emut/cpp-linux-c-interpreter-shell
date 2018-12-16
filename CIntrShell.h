
class CIntrShell
{
public:
	//CIntrShell(char* par_cpExecutableName);
	//~CIntrShell();
	static long long int CallFunctionWithArgs(char* par_cpInput);

public:		//To be changed to private after class is completed!

	enum teArgumentType
	{
		ARG_NUMBER = 0,
		ARG_STRING = 1
	};
	static void* getFuncAddressByName(char* par_cpFuncName); 
	static int ShellIO(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength);
	static bool getProgramName(char* par_cpProgramName);
	static int ParseShellCommand(char* par_cpShellCommand, char* par_cpFuncName, char* par_cpStringLiteralArgs, long long int* par_llnpNumaricArgs, teArgumentType* par_epArgTypes);
	static bool ParseFuncName(char** par_cppShellCommand, char* par_cpFuncName);
	static bool ParseStringArgument(char** par_cppShellCommand, char* par_cpStringLiteralArgs, int* par_npStringBufferOffset);
	static bool ParseNumericArgument(char** par_cppShellCommand, long long int* par_llnpResult);
};