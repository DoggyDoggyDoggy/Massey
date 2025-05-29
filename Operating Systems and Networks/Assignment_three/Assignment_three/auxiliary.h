//////////////////////////////////////////////////////////////////////////////////
//
//   Program Name:  Auxiliary
//                  
//   Description:  Auxiliary functions, functions and constants
//                  
//   * you are not allowed to modify this file (auxiliary.h)  
//
//   Author: Napoleon Reyes
//
//   References:  
//
//     Martin Johnson's codes
//     Andre Barczak's codes
//     https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
//     https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
//
//     VIRTUAL ADDRESS SPACE AND PHYSICAL STORAGE: https://docs.microsoft.com/en-us/windows/win32/memory/virtual-address-space-and-physical-storage
//     MEMORY MANAGEMENT:  https://docs.microsoft.com/en-us/windows/win32/memory/about-memory-management
//     MEMORY PROTECTION CONSTANTS:
//          https://docs.microsoft.com/en-nz/windows/win32/memory/memory-protection-constants

//
//////////////////////////////////////////////////////////////////////////////////
#ifndef __AUXILIARY_H__
#define __AUXILIARY_H__

#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <time.h>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
   #include <windows.h>
   #include <psapi.h> // Include psapi.h for PROCESS_MEMORY_COUNTERS_EX
   #include <cmath>
   #include <cstdint>  //https://stackoverflow.com/questions/1845482/what-is-uintptr-t-data-type/1846648#1846648
                      //https://stackoverflow.com/questions/1845482/what-is-uintptr-t-data-type 
   #include <tchar.h>
    
#elif defined(__APPLE__)
    #include <mach/mach.h>
    #include <sys/resource.h> //declaration of struct rusage 
    #include <sys/time.h>
    #include <unistd.h>  //where _SC_PAGE_SIZE is defined and sysconf() is declared.
    #include <sys/mman.h>
#elif defined(__linux__)
    #include <fstream>
    #include <string>
    #include <sys/resource.h> //declaration of struct rusage 
    #include <sys/time.h>
    #include <unistd.h>  //where _SC_PAGE_SIZE is defined and sysconf() is declared.
    #include <sys/mman.h>
#endif



using namespace std;


////////////////////////////////////////////////////////////////
//----------------------------------------
// RUN WHICH TEST? RUN_SIMPLE_TEST or RUN_COMPLETE_TEST ()
//----------------------------------------
// (1) Complete test
  #define RUN_COMPLETE_TEST //default

   //------------------------
   //pick one
   //------------------------
   //(A) Simulation #1 
    // #define USE_SIMULATION_1
   
   //(B) Simulation #2 //bigger memory block requests
     #define USE_SIMULATION_2 //default
//---------------------------------------
// (2) Simple Test
    // #define RUN_SIMPLE_TEST
//---------------------------------------

/////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////




// the following is fixed by the OS
// you are not allowed to change it
#define PAGESIZE 4096
// you may want to change the following lines if your
// machine is very fast or very slow to get sensible times
// but when you submit please put them back to these values.
#define NO_OF_POINTERS 2000
#define NO_OF_ITERATIONS 200000

extern unsigned seed;


#define WIDTH 7
#define DIV 1024

////////////////////////////////////////////////////////////////////////////////////
// you are not allowed to change the following functions

void show_page_size();
void printMemoryUsage(size_t memory);
size_t getMemoryUsage();

#if defined __unix__ || defined __APPLE__
  
  void* Virtual_Alloc(size_t size);

#endif


void  *allocpages(int n);
int freepages(void *p);

void *mymalloc(int n);
int myfree(void *p);

int myrand();
int randomsize();
//---


#endif