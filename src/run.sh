#! /bin/bash
make clean
make

echo ""
../bin/./hw5 ../testcase/ibm01.modified.txt ../output/ibm01.result
../verifier/./verify ../testcase/ibm01.modified.txt ../output/ibm01.result

echo ""
../bin/./hw5 ../testcase/ibm02.modified.txt ../output/ibm02.result
../verifier/./verify ../testcase/ibm02.modified.txt ../output/ibm02.result

echo ""
../bin/./hw5 ../testcase/ibm04.modified.txt ../output/ibm04.result
../verifier/./verify ../testcase/ibm04.modified.txt ../output/ibm04.result
