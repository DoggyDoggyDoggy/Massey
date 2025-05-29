//////////////////////////////////////////////////////////////////////////////////
//
//   Program Name:  Auxiliary
//                  
//   Description:  Auxiliary functions, functions and constants
//                  
//   * you are not allowed to modify this file (auxiliary.cpp) 
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

#include "auxiliary.h"


////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////
// Find pagesize of system
//////////////////////////////////////
#if defined __unix__ || defined __APPLE__

/*
Virtual_Alloc() uses mmap() to allocate a block of memory of the specified size. 
The NULL argument indicates that the kernel should choose a suitable address for the memory. 
The PROT_READ | PROT_WRITE arguments specify that the memory should be readable and writable. 
The MAP_PRIVATE | MAP_ANONYMOUS arguments indicate that the memory should be private to the process and not backed by any file or physical memory. 
Finally, the -1 and 0 arguments specify that the kernel should choose a suitable file descriptor and offset.

If mmap() fails to allocate memory, it returns MAP_FAILED. In this case, we print an error message using perror() and exit the program with a failure status.
*/
    void* Virtual_Alloc(size_t size) {
        void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
        return ptr;
    }
#endif


//////////////////////////////////////
// Find pagesize of system
//////////////////////////////////////
#if defined __unix__ || defined __APPLE__

    void show_page_size() {
        long page_size = sysconf(_SC_PAGESIZE);  //call sysconf() with the _SC_PAGESIZE argument to retrieve the size of a page in bytes. 
        if (page_size == -1) { //If sysconf() returns -1, an error has occurred 
            printf("Failed to get page size.\n");
        } else {
            printf("\n\tPage size is %ld bytes.\n", page_size);
        }
    }

#elif defined __WIN32__

    void show_page_size() {
        SYSTEM_INFO system_info;
        GetSystemInfo(&system_info);
        printf("\n\tPage size is %u bytes.\n", system_info.dwPageSize);
    }

#endif

//////////////////////////////////////////////////////////////////////////////////////


// you are not allowed to change the following function
void  *allocpages(int n) { // allocate n pages and return start address
   // VirtualAlloc reserves a block of pages with NULL specified as the base address parameter, forcing the system to determine the location of the block
   // VirtualAlloc is called whenever it is necessary to commit a page from this reserved region, and the base address of the next page to be committed is specified
   // Extra: To allocate memory in the address space of another process, use the VirtualAllocEx function.
   // return VirtualAlloc(NULL,n * PAGESIZE,4096+8192,4); //original
   
#if defined __unix__ || defined __APPLE__
   return (void *) malloc(n*PAGESIZE+1); //unix
#elif defined __WIN32__
   return VirtualAlloc(NULL, n * PAGESIZE, PAGESIZE + MEM_RESERVE, PAGE_READWRITE);  //windows only
#endif
                     //Parameters: 
   
                     // NULL          = system selects address (where to allocate the region);
                     // n*dwPageSize = number of pages requested * PAGESIZE = Size of allocation
                     // MEM_RESERVE  = Allocate reserved pages
                     // PAGE_READWRITE = Enables read-only or read/write access to the committed region of pages. 
                     //                  If Data Execution Prevention is enabled, attempting to execute code in the committed region results in an access violation.
}
//---
// you are not allowed to change the following function
int freepages(void *p) { // free previously allocated pages.
  //return VirtualFree(p,0,32768); //original
#if defined __unix__ || defined __APPLE__
  free(p);
  return(1);//indicate true
#elif defined __WIN32__
  return VirtualFree(p,0, MEM_RELEASE); //// Release the block of pages
                       //p - A pointer to the base address of the region of pages to be freed.
                       //0 - Bytes of committed pages; If the dwFreeType parameter is MEM_RELEASE, this parameter must be 0 (zero).
                       //MEM_RELEASE - Decommit the pages and release the entire region of reserved and committed pages. constant is equivalent to 32768
#endif

}

//---
void *mymalloc(int n) { // very simple memory allocation
   void *p;
   p=allocpages((n/PAGESIZE)+1);
   if(!p) puts("Failed");
   return p;
}
//---
int myfree(void *p) { // very simple free
   int n;
   n=freepages(p);
   if(!n) puts("Failed");
   return n;
}


////////////////////////////////////////////////////////////////////////
//---
// SIMULATION 2
//2024 version
#ifdef USE_SIMULATION_2
  int myrand() { // pick a random number

     //seed=(seed*2416+374441) % 4095976;
     seed=(seed*2416+374441) % 1095976;
     return seed;
  }

  int randomsize() { // choose the size of memory to allocate
    int j,k;
    int n=0;
    j=0; //new
    k=0; //new
     
    k=myrand();
       
    j=(k&3)+(k>>2 &3)+(k>>4 &3)+(k>>6 &3)+(k>>4 &3)+(k>>4 &3);
    j=1<<j;
    //n = 2500 + (myrand() % (j<<8));
    n = 500 + (myrand() % (j<<5));
    
    return n;
  }

#endif
//---
// SIMULATION 1
//old version

////////////////////////////////////////////////////////////////////////
#ifdef USE_SIMULATION_1

  int myrand() { // pick a random number
     
     seed=(seed*2416+374441)%1771875;
     return seed;
  }

  int randomsize() { // choose the size of memory to allocate
     int j,k;
     j=0; //new
     k=0; //new

     k=myrand();
     j=(k&3)+(k>>2 &3)+(k>>4 &3)+(k>>6 &3)+(k>>8 &3)+(k>>10 &3);
     j=1<<j;
     return (myrand() % j) +1;
  }
#endif

void printMemoryUsage(size_t memory) {

    double mem_Bytes = (double)memory;
    double mem_Kbytes = (double)memory / 1024;
    
    double mem_Mbytes = (double)memory / (1024 * 1024);
    double mem_Gbytes = (double)memory / (1024 * 1024 * 1024);
    
    // cout << "Memory = " << mem_Kbytes << " KiloBytes, or " <<  mem_Mbytes << " MegaBytes" << endl;
    cout << "Memory = " << mem_Bytes << " bytes" << ", or " << mem_Kbytes << " KiloBytes, or " <<  mem_Mbytes << " MegaBytes" << endl;
    //cout << "Memory = " << mem_Bytes << " bytes" << ", or " << mem_Kbytes << " KiloBytes, or " <<  mem_Mbytes << " MegaBytes" << ", or " <<  mem_Gbytes << " GigaBytes" << endl;
}


// Function to retrieve private memory usage
size_t getMemoryUsage() {
    size_t memoryUsage = 0;

#if defined(_WIN32) || defined(_WIN64)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        memoryUsage = pmc.PrivateUsage;
    }
#elif defined(__APPLE__)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
        memoryUsage = info.resident_size;
    }
#elif defined(__linux__)
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        size_t residentPages;
        statm >> residentPages;
        // Resident pages * page size = memory usage in bytes
        memoryUsage = residentPages * sysconf(_SC_PAGESIZE);
    }
    // struct rusage usage;
    // getrusage(RUSAGE_SELF, &usage);
    // memoryUsage = usage.ru_maxrss;

#endif

    return memoryUsage;
}

////////////////////////////////////////////////////////////////////////
//---






