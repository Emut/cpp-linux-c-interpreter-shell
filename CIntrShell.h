
class CIntrShell
{
public:
	CIntrShell(char* par_cpExecutableName);
	//~CIntrShell();

	long long int CallFunction(char* par_cpFuncName);

public:		//To be changed to private
	void* getFuncAddressByName(char* par_cpFuncName); 
	static int ShellIO(char* par_cpInput, char* par_cpOutput, int par_nMaxOutputLength);
	static bool getProgramName(char* par_cpProgramName);

	char cpExecutableName[50];
	
};