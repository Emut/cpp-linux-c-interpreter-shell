all:
	g++ -o Test -g ShellTester.cpp CIntrShell.cpp CTestClass.cpp
	./Test
clear:
	rm Test