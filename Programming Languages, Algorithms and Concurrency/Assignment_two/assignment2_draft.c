// --------------------------------------------------
// ---   159.341 Assignment 2 - Lift Simulator    ---
// --------------------------------------------------
// Denys Pedan #23011350
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "lift.h"

// --------------------------------------------------
// Define Problem Size
// --------------------------------------------------
#define NLIFTS 4          // The number of lifts in the building
#define NFLOORS 20        // The number of floors in the building
#define NPEOPLE 20        // The number of people in the building
#define MAXNOINLIFT 10    // Maximum number of people in a lift

// --------------------------------------------------
// Define delay times (in milliseconds)
// --------------------------------------------------
#define SLOW
// #define FAST

#if defined(SLOW)
    #define LIFTSPEED 50      // The time it takes for the lift to move one floor
    #define GETINSPEED 50     // The time it takes to get into the lift
    #define GETOUTSPEED 50    // The time it takes to get out of the lift
    #define PEOPLESPEED 100   // The maximum time a person spends on a floor
#elif defined(FAST)
    #define LIFTSPEED 0       // The time it takes for the lift to move one floor
    #define GETINSPEED 0      // The time it takes to get into the lift
    #define GETOUTSPEED 0     // The time it takes to get out of the lift
    #define PEOPLESPEED 1     // The maximum time a person spends on a floor
#endif

// --------------------------------------------------
// Define lift directions (UP/DOWN)
// --------------------------------------------------
#define UP 1
#define DOWN -1

// --------------------------------------------------
// Mutual exclusion for printing
// --------------------------------------------------
semaphore print_mutex;

// --------------------------------------------------
// Information about a floor in the building
// --------------------------------------------------
typedef struct {
    int waitingtogoup;      // The number of people waiting to go up
    int waitingtogodown;    // The number of people waiting to go down
    semaphore up_arrow;     // People going up wait on this
    semaphore down_arrow;   // People going down wait on this
    semaphore mutex;        // Protects waiting counts
} floor_info;

// --------------------------------------------------
// Information about a lift
// --------------------------------------------------
typedef struct {
    int no;                       // The lift number (id)
    int position;                 // The floor it is on
    int direction;                // Which direction it is going (UP/DOWN)
    int peopleinlift;             // The number of people in the lift
    int stops[NFLOORS];           // How many people are going to each floor
    semaphore stopsem[NFLOORS];   // People in the lift wait on one of these
    semaphore mutex;              // Protects peopleinlift and stops[]
} lift_info;

// --------------------------------------------------
// Some global variables
// --------------------------------------------------
floor_info floors[NFLOORS];

// --------------------------------------------------
// External declarations (global lifts)
// --------------------------------------------------
lift_info lifts[NLIFTS];

// --------------------------------------------------
// Print a string on the screen at position (x,y)
// --------------------------------------------------
void print_at_xy(int x, int y, const char *s) {
    // Mutual exclusion for screen output
    semaphore_wait(&print_mutex);

    // Move cursor to (x,y)
    gotoxy(x,y);

    // Slow things down
	Sleep(1);
    
    // Print the string
	printf("%s", s);

    // Move cursor out of the way
    gotoxy(42, NFLOORS+2);

    semaphore_signal(&print_mutex);
}

// --------------------------------------------------
// Function for a lift to pick people waiting on a
// floor going in a certain direction
// --------------------------------------------------
void get_into_lift(lift_info *lift, int direction) {
    
    // Local variables
    int *waiting;
    semaphore *arrow;
    int count;

    // Check lift direction
    if(direction == UP) {
        // Use up_arrow semaphore
        arrow = &floors[lift->position].up_arrow;
        // Number of people waiting to go up
        waiting = &floors[lift->position].waitingtogoup;
    } else {
        // Use down_arrow semaphore
        arrow = &floors[lift->position].down_arrow;
        // Number of people waiting to go down
        waiting = &floors[lift->position].waitingtogodown;
    }

    while(1) {
        semaphore_wait(&floors[lift->position].mutex);
        count = *waiting;
        semaphore_signal(&floors[lift->position].mutex);
        if(count == 0) break;

        semaphore_wait(&lift->mutex);
        if(lift->peopleinlift >= MAXNOINLIFT) {
            semaphore_signal(&lift->mutex);
            break;
        }
        // First person sets lift direction
        if(lift->peopleinlift == 0)
            lift->direction = direction;
        lift->peopleinlift++;
        semaphore_signal(&lift->mutex);

        // Remove one waiting
        semaphore_wait(&floors[lift->position].mutex);
        (*waiting)--;
        semaphore_signal(&floors[lift->position].mutex);

        // Erase waiting person
        print_at_xy(NLIFTS*4 +
            floors[lift->position].waitingtogodown +
            floors[lift->position].waitingtogoup,
            NFLOORS - lift->position, " ");

        // Signal one waiting passenger to board
        semaphore_signal(arrow);

        Sleep(GETINSPEED);
    }
}

// --------------------------------------------------
// Function for the Lift Threads
// --------------------------------------------------
void* lift_thread(void *arg) {
    // Local variables
    lift_info *lift = (lift_info*)arg;
    int i;

    // Initialize lift state
    lift->position = 0;
    lift->direction = UP;
    lift->peopleinlift = 0;
    semaphore_create(&lift->mutex, 1);

    for(i = 0; i < NFLOORS; i++) {
        lift->stops[i] = 0;
        semaphore_create(&lift->stopsem[i], 0);
    }

    randomise();
    Sleep(rnd(1000));

    while(1) {
        // Draw lift
        print_at_xy(lift->no*4 + 1, NFLOORS - lift->position, lf);
        Sleep(LIFTSPEED);

        // Drop off passengers
        while (1) {
            semaphore_wait(&lift->mutex);
            if(lift->stops[lift->position] == 0) {
                semaphore_signal(&lift->mutex);
                break;
            }
            lift->peopleinlift--;
            lift->stops[lift->position]--;
            semaphore_signal(&lift->mutex);

            Sleep(GETOUTSPEED);
            // Signal passenger to leave
            semaphore_signal(&lift->stopsem[lift->position]);

            if(lift->stops[lift->position] == 0) {
                print_at_xy(lift->no*4+3, NFLOORS - lift->position, " ");
            }
        }

        // Pick up
        if(lift->direction == UP || lift->peopleinlift == 0)
            get_into_lift(lift, UP);
        if(lift->direction == DOWN || lift->peopleinlift == 0)
            get_into_lift(lift, DOWN);

        // Erase lift and move
        print_at_xy(lift->no*4 + 1, NFLOORS - lift->position, " ");
        lift->position += lift->direction;
        if(lift->position == 0 || lift->position == NFLOORS-1)
            lift->direction = -lift->direction;
    }
    return NULL;
}

// --------------------------------------------------
// Function for the Person Threads
// --------------------------------------------------
void* person_thread(void *arg) {
    int from = 0, to;
    lift_info *lift;
    int i;

    randomise();
    while(1) {
        Sleep(rnd(PEOPLESPEED));
        do { to = rnd(NFLOORS); } while(to == from);

        // Press button and wait
        if(to > from) {
            semaphore_wait(&floors[from].mutex);
            floors[from].waitingtogoup++;
            semaphore_signal(&floors[from].mutex);
            print_at_xy(NLIFTS*4 + floors[from].waitingtogodown + floors[from].waitingtogoup,
                        NFLOORS - from, pr);
            semaphore_wait(&floors[from].up_arrow);
        } else {
            semaphore_wait(&floors[from].mutex);
            floors[from].waitingtogodown++;
            semaphore_signal(&floors[from].mutex);
            print_at_xy(NLIFTS*4 + floors[from].waitingtogodown + floors[from].waitingtogoup,
                        NFLOORS - from, pr);
            semaphore_wait(&floors[from].down_arrow);
        }

        // Find the lift we just boarded
        for(i = 0; i < NLIFTS; i++) {
            semaphore_wait(&lifts[i].mutex);
            if(lifts[i].position == from &&
               ((to>from && lifts[i].direction==UP) ||
                (to<from && lifts[i].direction==DOWN)) &&
               lifts[i].peopleinlift <= MAXNOINLIFT) {
                lift = &lifts[i];
                semaphore_signal(&lifts[i].mutex);
                break;
            }
            semaphore_signal(&lifts[i].mutex);
        }

        // Request destination
        semaphore_wait(&lift->mutex);
        lift->stops[to]++;
        if(lift->stops[to] == 1) {
            print_at_xy(lift->no*4 + 3, NFLOORS - to, "-");
        }
        semaphore_signal(&lift->mutex);

        // Wait until arrival
        semaphore_wait(&lift->stopsem[to]);

        // Exit lift
        from = to;
    }
    return NULL;
}

// --------------------------------------------------
// Print the building on the screen
// --------------------------------------------------
void printbuilding(void) {
    // Local variables
    int l,f;

    // Clear Screen
    system(clear_screen);
    // Print Roof
    printf("%s", tl);
    for(l = 0; l < NLIFTS-1; l++) printf("%s%s%s%s", hl, td, hl, td);
    printf("%s%s%s%s\n", hl, td, hl, tr);

    // Print Floors and Lifts
    for(f = NFLOORS-1; f >= 0; f--) {
        for(l = 0; l < NLIFTS; l++) {
            printf("%s%s%s ", vl, lc, vl);
            if(l == NLIFTS-1) printf("%s\n", vl);
        }
    }
    
    // Print Ground
    printf("%s", bl);
    for(l = 0; l < NLIFTS-1; l++) printf("%s%s%s%s", hl, tu, hl, tu);
    printf("%s%s%s%s\n", hl, tu, hl, br);

    printf("Lift Simulation - Press CTRL-Break to exit\n");
}

// --------------------------------------------------
// Main starts the threads and then waits.
// --------------------------------------------------
int main() {
    // Local variables
    unsigned long long i;

    // Initialize print mutex
    semaphore_create(&print_mutex, 1);

    // Initialize floors
    for(i = 0; i < NFLOORS; i++) {
        floors[i].waitingtogoup = 0;
        floors[i].waitingtogodown = 0;
        semaphore_create(&floors[i].up_arrow, 0);
        semaphore_create(&floors[i].down_arrow, 0);
        semaphore_create(&floors[i].mutex, 1);
    }

    // Print Building
    printbuilding();

    // Create lifts
    for(i = 0; i < NLIFTS; i++) {
        lifts[i].no = i;
        create_thread(lift_thread, &lifts[i]);
    }

    // Create people
    for(i = 0; i < NPEOPLE; i++) {
        // Create Person Thread
        create_thread(person_thread, (void*)i);
    }

    Sleep(86400000ULL);
    return 0;
}
