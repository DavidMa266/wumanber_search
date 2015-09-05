The C++ boost libraries are required.

To compile on Linux: g++ -g -std=c++0x search.cpp -L/usr/local/lib -lboost_random -lboost_system


This is just an attempted/implementation of the Wu Manber search algorithm.

Given a text file, with words separated by spaces/new lines, and a pattern to search for, 
this algorithm will efficiently return all occurrences of the pattern.

It also allows the user to simply enter a * input to indicate any number of any pattern of letters/numbers.

For example searching for the pattern *, will return all words in the text. 

Another example: if the word "hello" appears in the text, "hello", "ell", "h*o", "*h*e*l*l*o*" will all return the
word "hello" as a match.

TODO: fixed matches with the format: A*A*A*B