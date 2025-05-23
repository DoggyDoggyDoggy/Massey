
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Program Name:  Buddy System
//                  
// Author of start-up code: Napoleon Reyes
// Description:   Memory Management using the Buddy System Algorithm.
//                This work was based largely on the previous assignments made by Martin Johnson 
//                and Andre Barczak.
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Submitted by: 
//
//      Student name(s), ID number:
//
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// References:
//
//  Martin Johnson's codes and assignment design
//  Andre Barczak's codes and assignment design
//  https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
//  https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
//
//  VIRTUAL ADDRESS SPACE AND PHYSICAL STORAGE: https://docs.microsoft.com/en-us/windows/win32/memory/virtual-address-space-and-physical-storage
//  MEMORY MANAGEMENT:  https://docs.microsoft.com/en-us/windows/win32/memory/about-memory-management
//  MEMORY PROTECTION CONSTANTS:
//     https://docs.microsoft.com/en-nz/windows/win32/memory/memory-protection-constants
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include "auxiliary.h"
#include "buddysys.h"

using namespace std;

unsigned seed;

Node *wholememory;

long long int MEMORYSIZE;

#define NUMBEROFPAGES  8192//7200  
// #define DEBUG_MODE //enable to see more details


////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------
// WHICH MEMORY MANAGEMENT STRATEGY?
//---------------------------------------

//---------------------------------------
//(1) use built-in C functions
// change the following lines to test the real malloc and free
const string strategy = "malloc"; //enable this to test the system MALLOC & FREE
#define MALLOC malloc //enable this to test the system MALLOC & FREE
#define FREE free //enable this to test the system MALLOC & FREE

//---------------------------------------
//(2) use user-defined functions
// const string strategy = "mymalloc";
// #define MALLOC mymalloc
// #define FREE myfree

//---------------------------------------
//Enable the following compiler directives to test your implementation of buddy system strategy
//(3) use Buddy System
// const string strategy = "Buddy System"; //enable this to test the Buddy System
// #define USE_BUDDY_SYSTEM  //enable this to test the Buddy System
// #define MALLOC buddyMalloc //enable this to test the Buddy System
// #define FREE buddyFree //enable this to test the Buddy System
//---------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN FUNCTION
////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
   int i,k;
   unsigned char *n[NO_OF_POINTERS]; // used to store pointers to allocated memory
   int size;
   unsigned int s[NO_OF_POINTERS]; // size of memory allocated - for testing



///////////////////////////////////////////////////////////
   //Initialise simulation variables

   seed=7652; //DO NOT CHANGE THIS SEED FOR RANDOM NUMBER GENERATION

   for(i=0;i<NO_OF_POINTERS;i++) {
      n[i]=0;     // initially nothing is allocated
   }
   
///////////////////////////////////////////////////////////
//---------------------------------------
// WHICH TEST ROUTINE?
//---------------------------------------
#ifdef RUN_SIMPLE_TEST
   cout << "=========================================" << endl;
   cout << "          << RUN SIMPLE TEST >>" << endl;
   cout << "=========================================" << endl;
#else
   #ifdef RUN_COMPLETE_TEST 
      cout << "=========================================" << endl;
      cout << "          << RUN COMPLETE TEST >>" << endl;

   #ifdef USE_SIMULATION_2
      cout << "          << SIMULATION 2 >>" << endl;
   #else
      cout << "          << SIMULATION 1 >>" << endl;
   #endif 

   cout << "=========================================" << endl;
   #endif

#endif



//---------------------------------------
//Record start time
//---------------------------------------
auto start = std::chrono::steady_clock::now();


//---------------------------------------
//Record initial memory
//---------------------------------------
#ifndef USE_BUDDY_SYSTEM
   size_t initialMemory = getMemoryUsage();

   printf("\nInitial ");
   printMemoryUsage(initialMemory);
#endif
 
   
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
#ifdef USE_BUDDY_SYSTEM
   ////acquire one wholememory block
   if (wholememory==NULL) {


        #ifndef RUN_SIMPLE_TEST
              //MEMORYSIZE = (long long int) ((long long int)NUMBEROFPAGES * (long long int)PAGESIZE);
              MEMORYSIZE = (long long int) ((long long int)NUMBEROFPAGES * (long long int)PAGESIZE);
        #else
              MEMORYSIZE = 512; //bytes  -  RUN_SIMPLE_TEST  
        #endif


#if defined __unix__ || defined __APPLE__

         
         wholememory=(Node*) Virtual_Alloc(MEMORYSIZE); 

#elif defined __WIN32__

         //---  
         //VirtualAlloc - Reserves, commits, or changes the state of a region of pages in the virtual address space of the calling process. Memory allocated by this function is automatically initialized to zero.
         //  the return value is the base address of the allocated region of pages.
         wholememory=(Node*) VirtualAlloc(NULL, MEMORYSIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); //works!

#endif
         //---
         wholememory->size=(long long int)(MEMORYSIZE-(long long int)sizeof(Node));//Data size only          
         wholememory->next=NULL;
         wholememory->previous=NULL;
   
   printf("\n---<< MEMORY SETTINGS >>-------------------------------------");
   
   //Find pagesize 
   show_page_size();

   cout << "\twhole memory address: " << wholememory << "\n";
   printf("\tNode structure size: %d\n",sizeof(Node));
   printf("\tInitial block: %lld bytes or %lld Megabytes.\n", wholememory->size, wholememory->size/(1024*1024));
   
   printf("----------------------------------------------------------------");     


   //Hint: Initialise the contiguous block to be used by the Buddy System algorithm here.


   }
   printf("\nInitialisation complete.\n");

#endif   




   
////////////////////////////////////////////////////////////////////////////////////////////////////////////
cout << "\n<< Simulation start >>\n";
   
//---------------------------------------
//Test routines - Begin
//---------------------------------------
long long int totalAllocatedBytes=0;
long long int totalFreeSpace=0;
long long int totalSizeOfNodes=0;

#ifdef RUN_SIMPLE_TEST


  int NUM_OF_REQUESTS= 5; //sequence of requests
  char actions[] =      {'m' ,  'm', 'm', 'f', 'f'};  //m = MALLOC, f = FREE
  int requests[] =      {13, 3, 110, 3, 13};  //if NUMBEROFPAGES_BUDDY is set to 64
  
  int pointer_index[] = { 0,  1, 2, 1, 0};
//======================================================================================= 
    
  char selectedAction;
  cout << "\tExecuting " << NUM_OF_REQUESTS << " rounds of combinations of memory allocation and deallocation..." << endl;

  for(int r=0; r < NUM_OF_REQUESTS; r++){
     
     #ifdef DEBUGGINGPRT
        cout << "---[Iteration : " << r << "]" << endl;
      #endif
      selectedAction = actions[r];

      size = (int)requests[r];
      k = pointer_index[r];

      
      switch(selectedAction){
        case 'm':
                   // #ifdef DEBUGGINGPRT
                       // cout << "\n======>REQUEST: MALLOC(" << size << ") =======\n\n";
                       cout << "\n======>REQUEST: n[" << k << "] = MALLOC(" << size << ") =======\n\n";  
                   // #endif
                    
                    
                    n[k]=(unsigned char *)MALLOC(size); // do the allocation
                    if(n[k] != NULL){
                       s[k]=size; // remember the size
                       totalAllocatedBytes = totalAllocatedBytes + s[k];
                       totalSizeOfNodes = totalSizeOfNodes + sizeof(Node);
                       #ifdef DEBUGGINGPRT
                          cout << "\t\t\t\tsuccessfully allocated memory of size: " << size << endl;   
                          //// printf("\t\tRETALLOC size %d at address %ld (block size %d at Nodeaddress %ld)\n",s[k], (Node*) ((uintptr_t)n[k]-(uintptr_t)wholememory),  s[k]+sizeof(Node), (Node*)( (uintptr_t)n[k]-(uintptr_t)sizeof(Node)-(uintptr_t)wholememory) );
                          
                          printf("\n\t\t\t\t << MALLOC() >>  relative address: %8ld size: %8d  (Node: %8ld Nodesize: %8d)\n", (Node*)((uintptr_t)n[k]-(uintptr_t)wholememory),  s[k], (Node*)((uintptr_t)n[k]-(uintptr_t)sizeof(Node)-(uintptr_t)wholememory) , s[k]+sizeof(Node) );
                       #endif   
                       

                       n[k][0]=(unsigned char) k;  // put some data in the first and 
               
                       if(s[k]>1) 
                          n[k][s[k]-1]=(unsigned char) k; // last byte

                    } else {
                       cout << "\tFailed to allocate memory of size: " << size << endl;   
                    }
                   break;
        case 'f':
                   // #ifdef DEBUGGINGPRT
                       cout << "\n======>REQUEST: FREE(n[" << pointer_index[r] << "]) =======\n\n";

                   // #endif
                   if(n[k]) { // if it was allocated then free it
                       // check that the stuff we wrote has not changed
                       
                       if ( (n[k][0]) != (unsigned char) k)//(n[k]+s[k]+k) )
                          printf("\t\t==>Error when checking first byte! in block %d \n",k);
                       if(s[k]>1 && (n[k][s[k]-1])!=(unsigned char) k )//(n[k]-s[k]-k))
                          printf("\t\t==>Error when checking last byte! in block %d \n",k);

                       #ifdef DEBUGGINGPRT
                         cout << "\n======>REQUEST: FREE(" << hex << n[k] << ") =======\n\n";
                         printf("\n\t\t\t\t << FREE() >>  relative address: %8ld size: %8d  (Node: %8ld Nodesize: %8d)\n", (Node*)((uintptr_t)n[k]-(uintptr_t)wholememory),  s[k], (Node*)((uintptr_t)n[k]-(uintptr_t)sizeof(Node)-(uintptr_t)wholememory) , s[k]+sizeof(Node) );
                       #endif
                       FREE(n[k]);
                       totalFreeSpace = totalFreeSpace + s[k];      

                    } 
                   break; 
      }



  }

  
#endif


////////////////////////////////////////////////////////

#ifdef RUN_COMPLETE_TEST  
   cout << "\n\tExecuting " << NO_OF_ITERATIONS << " rounds of combinations of memory allocation and deallocation..." << endl;
    
   for(i=0;i<NO_OF_ITERATIONS;i++) {

    #ifdef DEBUG_MODE
      cout << "iteration: " << i << endl;
    #endif

      k=myrand() % NO_OF_POINTERS; // pick a pointer

      if(n[k]) { // if it was allocated then free it
         // check that the stuff we wrote has not changed       
         if ( (n[k][0]) != (unsigned char) k)//(n[k]+s[k]+k) )
            printf("Error when checking first byte! in block %d \n",k);
         if(s[k]>1 && (n[k][s[k]-1])!=(unsigned char) k )//(n[k]-s[k]-k))
            printf("Error when checking last byte! in block %d \n",k);

         FREE(n[k]);         
      }
      size=randomsize(); // pick a random size
      #ifdef DEBUG_MODE
        cout << "\tPick random size to allocate: " << size << endl;
      #endif
        
      n[k]=(unsigned char *)MALLOC(size); // do the allocation
      if(n[k] != NULL){
         #ifdef DEBUG_MODE
            cout << "\tallocated memory of size: " << size << endl;   
         #endif   
         s[k]=size; // remember the size

         n[k][0]=(unsigned char) k;  // put some data in the first and 
 
         if(s[k]>1) n[k][s[k]-1]=(unsigned char) k; // last byte

      } else {
         // cout << "\tFailed to allocate memory of size: " << size << endl;   
         cout << "\tFailed to allocate memory of size: " << size << " at iteration #" << i  << endl;
      }    
      
   }
#endif
//---------------------------------------
//Test routines - End
//---------------------------------------
   cout << "\n<< End of simulation >>\n\n"; 
   /////////////////////////////////////////////////////////////////////////////////////////////////


//---------------------------------------
// Performance Report
//---------------------------------------
   auto end = std::chrono::steady_clock::now();
   auto time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

   cout << "========================================================" << endl;
   cout << "          << " << strategy << " PERFORMANCE REPORT >>" << endl;
   cout << "========================================================" << endl;


#ifndef USE_BUDDY_SYSTEM
  // Get the memory usage after running the process
  size_t finalMemory = getMemoryUsage();

  // Compute the memory used by the process
  size_t memoryUsed = finalMemory - initialMemory;


  printf("\nFinal ");
  printMemoryUsage(finalMemory);
#endif   
  printf("\n---<< RESULTS >>---------------------------------------------");

#ifndef USE_BUDDY_SYSTEM    
  printf("\n\tUsed ");
  printMemoryUsage(memoryUsed);
#endif

  std::cout << "\n\tTime elapsed: " << time_elapsed.count() / 1e6 << " seconds" << std::endl;
  std::cout << "\tTime elapsed: " << time_elapsed.count() << " microseconds" << std::endl;
  printf("----------------------------------------------------------------");
  
 


   return 0;
}

