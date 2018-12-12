
class CIntrShell
{
public:
	CIntrShell();
	~CIntrShell();

	static long long int CallFunction(char* par_cpFuncName);

private:
	static void* getFuncAddressByName(char* par_cpFuncName); 
	
};