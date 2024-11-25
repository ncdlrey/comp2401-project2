#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// This function is only used by this file, so declared here and set to static to avoid having it linked by any other file

static void display_simulation_state(Manager *manager);

/**
 * Initializes the `Manager`.
 *
 * Sets up the manager by initializing the system array, resource array, and event queue.
 * Prepares the simulation to be run.
 *
 * @param[out] manager  Pointer to the `Manager` to initialize.
 */
void manager_init(Manager *manager) {
    manager->simulation_running = 1; // Any non-zero value to state the sim is running
    system_array_init(&manager->system_array);
    resource_array_init(&manager->resource_array);
    event_queue_init(&manager->event_queue);
}

/**
 * Cleans up the `Manager`.
 *
 * Frees all resources associated with the manager.
 *
 * @param[in,out] manager  Pointer to the `Manager` to clean.
 */
void manager_clean(Manager *manager) {}

/**
 * Runs the manager loop.
 *
 * Handles event processing, updates system statuses, and displays the simulation state.
 * Continues until the simulation is no longer running. (In a multi-threaded implementation)
 *
 * @param[in,out] manager  Pointer to the `Manager`.
 */
void manager_run(Manager *manager) {
    Event event;
    int i, status;
    int event_found_flag = 0, no_oxygen_flag = 0, distance_reached_flag = 0, need_more_flag = 0, need_less_flag = 0;
    
    System *sys = NULL;

    // Update the display of the current state of things
    display_simulation_state(manager);

    // Process events if one is popped
    
    event_found_flag = event_queue_pop(&manager->event_queue, &event);

    while (event_found_flag) {
        // Handle the event
        printf("Event: [%s] Reported Resource [%s : %d] Status [%d]\n",
                event.system->name,
                event.resource->name,
                event.amount,
                event.status);

        // Set some flags based on the event that we can react to below
        no_oxygen_flag        = (event.status == STATUS_EMPTY && strcmp(event.resource->name, "Oxygen") == 0);
        distance_reached_flag = (event.status == STATUS_CAPACITY && strcmp(event.resource->name, "Distance") == 0);
        need_more_flag        = (event.status == STATUS_LOW || event.status == STATUS_EMPTY || event.status == STATUS_INSUFFICIENT);
        need_less_flag        = (event.status == STATUS_CAPACITY);

        if (no_oxygen_flag) {
            printf("Oxygen depleted. Terminating all systems.\n");
        }

        if (distance_reached_flag) {
            printf("Destination reached. Terminating all systems.\n");
        }

        if (no_oxygen_flag || distance_reached_flag) {
            status = TERMINATE;
            manager->simulation_running = 0;
        }
        else if (need_more_flag) {
            status = FAST;
        }
        else if (need_less_flag) {
            status = SLOW;
        }

        if (no_oxygen_flag || distance_reached_flag || need_more_flag || need_less_flag) {
            // Update all of the systems to speed up or slow down production, or terminate
            for (i = 0; i < manager->system_array.size; i++) {
                sys = manager->system_array.systems[i];
                if (status == TERMINATE || sys->produced.resource == event.resource) {
                    sys->status = status;
                }
            }   
        }

        event_found_flag = event_queue_pop(&manager->event_queue, &event);
    }
    
}

// Don't worry much about these! These are special codes that allow us to do some formatting in the terminal
// Such as clearing the line before printing or moving the location of the "cursor" that will print.
#define ANSI_CLEAR "\033[2J"
#define ANSI_MV_TL "\033[H"
#define ANSI_LN_CLR "\033[K"
#define ANSI_MV_D1 "\033[1B"
#define ANSI_SAVE "\033[s"
#define ANSI_RESTORE "\033[u"

/**
 * Displays the current simulation state.
 *
 * Outputs the statuses of resources and systems to the console.
 * This function is typically called periodically to update the display.
 *
 * @param[in] manager  Pointer to the `Manager` containing the simulation state.
 */
void display_simulation_state(Manager *manager) {
    // Static integers are allocated to the data segment, so they persist between function calls
    static const int display_interval = 1;
    static time_t last_display_time = 0;

    // If it has not been long enough since our previous display refresh, keep waiting.
    time_t current_time = time(NULL);
    if (difftime(current_time, last_display_time) < display_interval) {
        return;
    }

    // Otherwise display to the screen by resetting the timer
    printf(ANSI_CLEAR);

    // Move the cursor to the top-left corner
    // But only after saving it's previous location
    printf(ANSI_MV_TL);

    // Display Resource Amounts
    printf(ANSI_LN_CLR "Current Resource Amounts:\n");
    printf(ANSI_LN_CLR "-------------------------\n");

    Resource *resource = NULL;
    int amount = 0; 
    int max_capacity = 0;
    for (int i = 0; i < manager->resource_array.size; i++) {
        resource = manager->resource_array.resources[i];

        amount = resource->amount;
        max_capacity = resource->max_capacity;

        printf(ANSI_LN_CLR "%s: %d / %d\n", resource->name, amount, max_capacity);
    }

    printf(ANSI_LN_CLR "\n");

    // Display System Statuses
    printf(ANSI_LN_CLR "System Statuses:\n");
    printf(ANSI_LN_CLR "---------------\n");

    System *system = NULL;
    for (int i = 0; i < manager->system_array.size; i++) {
        system = manager->system_array.systems[i];

        // Map system status code to a human-readable string
        const char *status_str;
        switch (system->status) {
            case TERMINATE:
                status_str = "TERMINATE";
                break;
            case DISABLED:
                status_str = "DISABLED";
                break;
            case SLOW:
                status_str = "SLOW";
                break;
            case STANDARD:
                status_str = "STANDARD";
                break;
            case FAST:
                status_str = "FAST";
                break;
            default:
                status_str = "UNKNOWN";
                break;
        }

        printf(ANSI_LN_CLR  "%-20s: %-10s\n", system->name, status_str);
    }

    printf(ANSI_LN_CLR  "\n");

    last_display_time = current_time;
    // Flush the output to ensure it appears immediately
    fflush(stdout);
}

