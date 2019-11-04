all:
	g++ -o Test -g ShellTester.cpp CInterpreter.cpp CTestClass.cpp
	./Test
clear:
	rm Test