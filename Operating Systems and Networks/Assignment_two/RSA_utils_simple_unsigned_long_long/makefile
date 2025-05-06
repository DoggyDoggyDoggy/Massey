RSA_utils_simple.exe		: 	RSA_utils_simple.o 
	g++ RSA_utils_simple.cpp -lws2_32 -o RSA_utils_simple.exe 
			
RSA_utils_simple.o		:	RSA_utils_simple.cpp 
	g++ -c -Wall -O2 -fconserve-space RSA_utils_simple.cpp

clean:
	del *.o
	del *.exe
