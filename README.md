
Primes!
=======




To compile:
-----------

### Fetch the [cppformat](https://github.com/cppformat/cppformat) library.
```
git submodule update --init
```

### In debug mode

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

### In release mode

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

To run tests:
-------------

Unzip the test file:
```
gunzip 2000000.txt.gz
```

Mount a tmpfs/ramfs for extra speed:
```
mkdir tmp
sudo mount -t tmpfs tmpfs tmp
```

Run the tests with the `run` command. The parameters are as follows:

 1. Number of repetitions
 2. Range of sieving threads, formated as "1 7" meaning that we try from 1 to 7 threads.
 3. Range of threads converting uints to strings in the same format as above.
 4. Range of chunks for the threads specified as above in 100000:s.
 5. The binary to try

For example: 
```
./run 10 "1 7" "1 7" "1 10" src/primes
```

Then the test output will be in the newly created `tests` folder.


