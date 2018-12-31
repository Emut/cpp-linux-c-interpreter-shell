# cpp-linux-c-interpreter-shell
Umut Ekici - 2018

UNDER CONSTRUCTION - First version gonna be here before 2019!

Inspired by the vxworks c interpreter shell. 

Call functions and existing variables or create new variables from anywhere, as simple c strings, including the terminal screen. 

#include? Who needs it anyway...

TODO:

-Sanitize output of ShellIO

-A way to handle multiple function name matches

-A way to output the case where call was unsuccessful

KNOWN ISSUES:

-Function name matching only checks starting from right. (e.g.: if a function named 'MyBestFunc' exist, a search for 'BestFunc' will match to it)
