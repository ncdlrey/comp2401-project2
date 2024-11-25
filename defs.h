#include <semaphore.h>

// Don't worry about these! These are special codes that allow us to do some formatting in the terminal
// Such as clearing the line before printing or moving the location of the "cursor" that will print.
#define ANSI_CLEAR "\033[2J"
#define ANSI_MV_TL "\033[H"
#define ANSI_LN_CLR "\033[K"
#define ANSI_MV_D1 "\033[1B"
#define ANSI_SAVE "\033[s"
#define ANSI_RESTORE "\033[u"

#define TERMINATE    0
#define DISABLED     1
#define SLOW         2
#define STANDARD     3
#define FAST         4

#define STATUS_OK          -1
#define STATUS_EMPTY        0
#define STATUS_LOW          1
#define STATUS_INSUFFICIENT 2
#define STATUS_CAPACITY     3
#define STATUS_PRODUCED     10

#define THRESHOLD_RESOURCE_LOW 0.3  // Percentage of resource before it is considered low.
#define MANAGER_WAIT_TIME 5         // Milliseconds for the manager to wait between popping the queue
#define SYSTEM_WAIT_TIME 20         // Milliseconds between loops of the system when production cannot occur

#define PRIORITY_HIGH 3
#define PRIORITY_MED 2
#define PRIORITY_LOW 1

// Represents the resource amounts for the entire rocket
typedef struct Resource {
    char *name;      // Dynamically allocated string
    int amount;
    int max_capacity;
} Resource;

// Represents the amount of a resource consumed/produced for a single system
typedef struct ResourceAmount {
    Resource *resource;
    int amount;
} ResourceAmount;

// A system which consumes resources, waits for `processing_time` milliseconds, then produced the produced resource
typedef struct System {
    char *name;     // Dynamically allocated string
    ResourceAmount consumed;
    ResourceAmount produced;
    int amount_stored;
    int processing_time;
    int status; 
    struct EventQueue *event_queue;  // Pointer to event queue shared by all systems and manager
} System;

// Used to send notifications to the manager about an issue / state of the system
typedef struct Event {
    System *system;
    Resource *resource;
    int status;     
    int priority;   // Higher values indicate higher priority
    int amount;     // Amount of the resource in question
} Event;

// Linked List Node for the Event queue
typedef struct EventNode {
    Event event;
    struct EventNode *next;
} EventNode;

// Linked List structure with a head and no tail, single instance shared by all systems
typedef struct EventQueue {
    EventNode *head;
    int size;
} EventQueue;

// A basic dynamic array to store all of the systems in the simulation
typedef struct SystemArray {
    System **systems;
    int size;
    int capacity;
} SystemArray;

// A basic resource array to store all resources in the simulation
typedef struct ResourceArray {
    Resource **resources;
    int size;
    int capacity;
} ResourceArray;

// Container structure which contains all of the core data for our simulation
typedef struct Manager {
    int simulation_running; // non-zero if the simulation is running, zero if it should be stopped
    SystemArray system_array;
    ResourceArray resource_array;
    EventQueue event_queue;
} Manager;

// Manager functions
void manager_init(Manager *manager);
void manager_clean(Manager *manager);
void manager_run(Manager *manager);

// System functions
void system_create(System **system, const char *name, ResourceAmount consumed, ResourceAmount produced, int processing_time, EventQueue *event_queue);
void system_destroy(System *system);
void system_run(System *system);

// Resource functions
void resource_create(Resource **resource, const char *name, int amount, int max_capacity);
void resource_destroy(Resource *resource);

// ResourceAmount functions
void resource_amount_init(ResourceAmount *resource_amount, Resource *resource, int amount);

// Event functions
void event_init(Event *event, System *system, Resource *resource, int status, int priority, int amount);

// EventQueue functions
void event_queue_init(EventQueue *queue);
void event_queue_clean(EventQueue *queue);
void event_queue_push(EventQueue *queue, const Event *event); 
int event_queue_pop(EventQueue *queue, Event* event);

// Dynamic array functions for systems and resources
void system_array_init(SystemArray *array);
void system_array_clean(SystemArray *array);
void system_array_add(SystemArray *array, System *system);

void resource_array_init(ResourceArray *array);
void resource_array_clean(ResourceArray *array);
void resource_array_add(ResourceArray *array, Resource *resource);