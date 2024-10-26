# Lab1 doc

Folder `src` contains the source code for the task and the folder `test` contains the test examples given to us.

Inside `src` you can run the following command to test one test case:

```
g++ *.cpp -std=c++17 -O2 -W -Wall -o generator && 
./generator < ../test/lab1_teza/01_nadji_x/test.lan && 
cd analizator && 
g++ *.cpp -std=c++17 -O2 -W -Wall -o analizator && 
./analizator < ../../test/lab1_teza/01_nadji_x/test.in
```

The folder `lab1-test-temp` contains a test script, copied contents from /src and more test cases

The zip file `lab1-submission.zip` is the submitted code.