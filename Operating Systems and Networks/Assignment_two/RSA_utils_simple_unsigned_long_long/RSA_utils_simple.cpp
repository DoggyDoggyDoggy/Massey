//////////////////////////////////////////////////////////////////////////////////////////////
// RSA test
// compiles using GCC 
//
//
// References: http://www.di-mgt.com.au/rsa_alg.html
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//
//////////////////////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

//---
#include <cfenv> //for math error checking
#include <cmath>
 


using namespace std;

//////////////////////////////////////////////////////////////


int repeat_square(int x,int e,int n);
unsigned long long repeat_square(unsigned long long x,unsigned long long e,unsigned long long n) ;

//////////////////////////////////////////////////////////////
int repeat_square(int x,int e,int n) {
  int y=1;//initialize y to 1, very important
  while (e>0) {
    if ((e%2)==0) {
      x=(x*x)%n;
      e=e/2;
    }
    else {
      y=(x*y)%n;
      e=e-1;
    }
  }
  return y; //the result is stored in y
}


/////////////////////////////////////////////



//////////////////////////////////////////////////////////////
unsigned long long repeat_square(unsigned long long x,unsigned long long e,unsigned long long n) {
  unsigned long long y=1;//initialize y to 1, very important
  while (e>0) {
    if ((e%2)==0) {
      x=(x*x)%n;
      e=e/2;
    }
    else {
      y=(x*y)%n;
      e=e-1;
    }
  }
  return y; //the result is stored in y
}

/////////////////////////////////////////////
void testRSA_3(){
    cout << "<< RSA TEST >>" << endl;
    cout << "integer numbers." << endl;
    int e,n, z;
    int d;
    // int nonce;
    // int calculatedRandomNum;
    //p=173 and q=149
    // int p=173;
    // int q=149;

    //note: p and q must be prime numbers!
    // int p=5;
    // int q=7;

    // n=p*q;
    // z = (p-1)*(q-1);
    // e = 5;
    // d = 29;
    // int input = 12;

//------------------------
    //lecture
    int p=173;
    int q=149;

    n=25777;
    z = (p-1)*(q-1);
    e = 3;
    d = 16971;

//------------------------



    int input = 66;//1234;

    int cipher;
    cout << "p = " << p << endl;
    cout << "q = " << q << endl; 
    cout << "n = " << n << endl;
    cout << "z = " << z << endl;
    cout << "--------------" << endl;
    cout << "(input) m = " << input << endl;
    cout << "\npublic key(e,n) = (" << e << ", " << n << ")" << endl;
    cout << "encrypting: c = m^e mod n" << endl;
    cout << "encrypting: c = " << input << "^" << e << " mod " << n << endl;
    cipher = repeat_square(input, e, n);

    cout << "cipher c = " << cipher << endl;

    
    cout << "\nprivate key(d,n) = (" << d << ", " << n << ")" <<  endl;
    cout << "decrypting: m = c^d mod n" << endl;
    cout << "decrypting: c = " << cipher << "^" << d << " mod " << n << endl;
    int number = repeat_square(cipher, d, n);
    cout << "decrypted value = " << number << endl;
    
    cout << "\n--- Analysing results ---" << endl;
    if(input == number){
       cout << "We have a match, therefore correct decryption!, " << "input = " << input << ", decrypted value = " << number << endl;    
    } else {
       cout << "Error in decryption!!" << "input = " << input << ", decrypted value = " << number << endl;    
    }

}

/////////////////////////////////////////////

void testRSA_4(){
    cout << "<< RSA TEST >>" << endl;
    cout << "integer numbers." << endl;
    unsigned long long e,n, z;
    unsigned long long d;
    // int nonce;
    // int calculatedRandomNum;
    //p=173 and q=149
    // int p=173;
    // int q=149;

    //note: p and q must be prime numbers!
    // int p=5;
    // int q=7;

    // n=p*q;
    // z = (p-1)*(q-1);
    // e = 5;
    // d = 29;
    // int input = 12;

//------------------------
    //lecture
    // int p=173;
    // int q=149;

    // n=25777;
    // z = (p-1)*(q-1);
    // e = 3;
    // d = 16971;

//------------------------

    //lecture
    unsigned long long p=431;
    unsigned long long q=443;

    n=190933;
    z = (p-1)*(q-1);
    e = 2113;
    d = 74297;

//------------------------

    unsigned long long input = 66;//1234;

    unsigned long long cipher;
    cout << "p = " << p << endl;
    cout << "q = " << q << endl; 
    cout << "n = " << n << endl;
    cout << "z = " << z << endl;
    cout << "--------------" << endl;
    cout << "(input) m = " << input << endl;
    cout << "\npublic key(e,n) = (" << e << ", " << n << ")" << endl;
    cout << "encrypting: c = m^e mod n" << endl;
    cout << "encrypting: c = " << input << "^" << e << " mod " << n << endl;
    cipher = repeat_square(input, e, n);

    cout << "cipher c = " << cipher << endl;

    
    cout << "\nprivate key(d,n) = (" << d << ", " << n << ")" <<  endl;
    cout << "decrypting: m = c^d mod n" << endl;
    cout << "decrypting: c = " << cipher << "^" << d << " mod " << n << endl;
    unsigned long long number = repeat_square(cipher, d, n);
    cout << "decrypted value = " << number << endl;
    
    cout << "\n--- Analysing results ---" << endl;
    if(input == number){
       cout << "We have a match, therefore correct decryption!, " << "input = " << input << ", decrypted value = " << number << endl;    
    } else {
       cout << "Error in decryption!!" << "input = " << input << ", decrypted value = " << number << endl;    
    }

}

/////////////////////////////////////////////

/////////////////////////////////////////////

void testRSA_5(){ //test only - not correct!
    cout << "<< RSA TEST >>" << endl;
    cout << "integer numbers." << endl;
    unsigned long long e,n, z;
    unsigned long long d;
    
//------------------------

    //lecture
    unsigned long long p=431;
    unsigned long long q=443;

    n=190933;
    z = (p-1)*(q-1);
    e = 2113;
    d = 74297;

//------------------------

    unsigned long long input = 66;//1234;

    unsigned long long cipher;
    cout << "p = " << p << endl;
    cout << "q = " << q << endl; 
    cout << "n = " << n << endl;
    cout << "z = " << z << endl;
    cout << "--------------" << endl;
    cout << "(input) m = " << input << endl;
    cout << "\npublic key(e,n) = (" << e << ", " << n << ")" << endl;
    cout << "encrypting: c = m^e mod n" << endl;
    cout << "encrypting: c = " << input << "^" << e << " mod " << n << endl;
    cipher = repeat_square(input, e, n);

    cout << "cipher c = " << cipher << endl;

    
    cout << "\nprivate key(d,n) = (" << d << ", " << n << ")" <<  endl;
    cout << "decrypting: m = c^d mod n" << endl;
    cout << "decrypting: c = " << cipher << "^" << d << " mod " << n << endl;
    unsigned long long number = repeat_square(cipher, d, n);
    cout << "decrypted value = " << number << endl;
    
    cout << "\n--- Analysing results ---" << endl;
    if(input == number){
       cout << "We have a match, therefore correct decryption!, " << "input = " << input << ", decrypted value = " << number << endl;    
    } else {
       cout << "Error in decryption!!" << "input = " << input << ", decrypted value = " << number << endl;    
    }

}

/////////////////////////////////////////////

int main(){

    // testRSA_3(); //int - works!
    testRSA_4(); //unsigned long long

  return 0;
}