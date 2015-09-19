The C++ boost libraries are required.

To compile on Linux: g++ -g -std=c++0x search.cpp -L/usr/local/lib -lboost_random -lboost_system


This is an attempted variation of the Wu Manber search algorithm.

This focuses on the problem where many of the target strings have the same or similar prefix values, such as web addresses.

When given a text file and a target pattern, this program will search and return all occurrences of such a pattern.

TODO: fixed matches with the format: A*A*A*B