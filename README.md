
 CInterpreter
 Umut Ekici 2019

 Inspired by the vxworks c interpreter shell. 
 Call functions and create variables via the interpreter.
 Feed the interpreter from terminal or a telnet client for rapid prototyping and testing especially in embedded environments.

 Functions in the executable is parsed (only on linux for now) via nm and grep. Therefore it depends on nm and grep.
 Additionally symbol names should be present in the executable (i.e. must not be strip'ed)

 Dynamicly loadded functions (e.g. printf, malloc...) are not present in the executable. They can be added by
 RegisterFunction(..) to be referenced later on. 

 Interpreter has a single entry point: CallFunctionWithArgs.
 CAUTION: This undoes 20 years of development effort on compilers. There is no type checking or even argument number checking.
 User is one letter away from invoking Undefined Behavior

 Class provides 2 additional functions SearchRegisteredFunction(..) and SearchExecutableForFunction(..)
 These functions can be used for obtaining function address, which then can be used with a proper function pointer cast,
 as in the case of dynamic loading. 

 Tested on Ubuntu 14.04, Ubuntu 16.04 g++ 5.4.0 and Visual Studio 2012 

An example session with functions in CTestClass.cpp is below. Comments with // are added for clarity

->
->lkup	//built-in function to print all user registered functions
CInterpreter::Registered Functions:
PrintVariables
free
lkup
malloc
printf
Call Returned:5
->
->printf("Hello World %d %c", 123, 10)	//call the printf with 3 arguments (3th arg is a newline)
Hello World 123 
Call Returned:17
->
->PrintMe //call the function AnotherTestClass::PrintMe() defined in CTestClass.cpp
So here you are
Call Returned:16
->
->CAnotherTestClass::PrintMe	//it is also possible to use with scope name
So here you are
Call Returned:16
->
->HiddenFunction	//this function is defined in 2 classes, listed below
Function has multiple alternatives:
CTestClass::HiddenFunction()
CAnotherTestClass::HiddenFunction()
Unknown Symbol, Call Failed
->
->CTestClass::HiddenFunction()	//include scope name to refer an indivdual function
You Have Found Me!
Call Returned:19
->CAnotherTestClass::HiddenFunction()	//note that paren's are optional 
You Have Found Me Out Of Scope!
Call Returned:32
->
->PrintValue()	//An overloaded function. use <> template specifiers to access
Function has multiple alternatives:
PrintValue(bool)
PrintValue(int)
PrintValue(int, char*)
PrintValue(int*)
PrintValue()
Unknown Symbol, Call Failed
->
->PrintValue<>()
No argument there..
Call Returned:20
->
->PrintValue<int>(123)
Number:123
Call Returned:11
->
->PrintValue<bool>(1)
Bool:true
Call Returned:10
->
->PrintValue<int, char*>(123, "A String Here")
Number:123, String:A String Here
Call Returned:123
->
->
->$myVar	//define variables, they have to start with $
myVar:0
->
->$anotherVar
anotherVar:0
->
->$intPtr = getNewNumber(123)	//if not defined, vars will be defined upon usage
Call Returned:8408304
->
->PrintVariables	//built-in function to print existing vars and values
CInterpreter::Registered Variables:
anotherVar:0
intPtr:8408304
myVar:0
Call Returned:3
->
->PrintValue<int*>($intPtr)	//variables can be used as arguments in function calls
Number:123
Call Returned:11
->








