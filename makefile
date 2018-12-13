all:
	g++ -o Test ShellTester.cpp CIntrShell.cpp CTestClass.cpp
	./Test
clear:
	rm Test