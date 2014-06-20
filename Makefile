
CPPFLAGS=-std=c++11
LDFLAGS=

CPP=clang++
LD=$(CPP)

CPP_CMD=$(CPP) $(CPPFLAGS) -o $@ -c $<

LD_CMD=$(LD) $(LDFLAGS)

OBJ=primes.o

primes: $(OBJ)
	$(LD_CMD) $(OBJ) -o primes

%.o: %.cpp
	$(CPP_CMD)


