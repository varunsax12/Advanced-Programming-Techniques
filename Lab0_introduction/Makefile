CFLAG += -fPIC -O3 #-fsanitize=address
CFLAG += -lm
CFLAG += -std=c++11 -Wno-unused-result


all:
	g++ *.cpp -o sim $(CFLAG) $(IFLAG)

clean:
	rm -f *.o sim
