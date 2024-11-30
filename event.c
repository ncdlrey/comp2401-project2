#include "defs.h"
#include <stdlib.h>
#include <stdio.h>

/* Event functions */

/**
 * Initializes an `Event` structure.
 *
 * Sets up an `Event` with the provided system, resource, status, priority, and amount.
 *
 * @param[out] event     Pointer to the `Event` to initialize.
 * @param[in]  system    Pointer to the `System` that generated the event.
 * @param[in]  resource  Pointer to the `Resource` associated with the event.
 * @param[in]  status    Status code representing the event type.
 * @param[in]  priority  Priority level of the event.
 * @param[in]  amount    Amount related to the event (e.g., resource amount).
 */
void event_init(Event *event, System *system, Resource *resource, int status, int priority, int amount) {
    event->system = system;
    event->resource = resource;
    event->status = status;
    event->priority = priority;
    event->amount = amount;
}

/* EventQueue functions */

/**
 * Initializes the `EventQueue`.
 *
 * Sets up the queue for use, initializing any necessary data (e.g., semaphores when threading).
 *
 * @param[out] queue  Pointer to the `EventQueue` to initialize.
 */
void event_queue_init(EventQueue *queue) {
    queue -> head = NULL;
    queue -> size = 0;

    // Initialize the semaphore with a value of 1 (unlocked)
    // 0 indicates that this semaphore is shared between threads within a single process
    sem_init(&queue->mutex, 0, 1);
}

/**
 * Cleans up the `EventQueue`.
 *
 * Frees any memory and resources associated with the `EventQueue`.
 * 
 * @param[in,out] queue  Pointer to the `EventQueue` to clean.
 */
void event_queue_clean(EventQueue *queue) {
    if(queue == NULL){
        return;
    }

    sem_destroy(&queue->mutex);

    EventNode *current = queue -> head;
    EventNode *temp;

    while(current != NULL){
        temp = current;
        current = current -> next;
        free(temp);
    }

    
    queue -> head = NULL;
    queue -> size = 0;
    
}

/**
 * Pushes an `Event` onto the `EventQueue`.
 *
 * Adds the event to the queue in a thread-safe manner, maintaining priority order (highest first).
 *
 * @param[in,out] queue  Pointer to the `EventQueue`.
 * @param[in]     event  Pointer to the `Event` to push onto the queue.
 */
void event_queue_push(EventQueue *queue, const Event *event) {
    if(queue == NULL || event == NULL){
        return;
    }

    

    // Create new node for event and initialize
    EventNode *newEventNode = (EventNode*) malloc(sizeof(EventNode));
    if (newEventNode == NULL) {
        printf("Error with memory allocation for EventNode");
        sem_post(&queue->mutex); // release in case of error
        return;
    }

    newEventNode -> event = *(event);
    newEventNode -> next = NULL;
    
    // Lock list
    sem_wait(&queue->mutex);

    // Case 1: If the queue is empty or the new event has higher priority than the head, insert at the head
    if (queue->head == NULL || (event -> priority) > (queue -> head -> event.priority)) {
        newEventNode -> next = queue->head; 
        queue->head = newEventNode;
    } 
    
    // Case 2: Normal
    else {
        // Traverse the queue to find the correct insertion point
        EventNode *current = queue->head;

        

        while (current->next != NULL && current->next->event.priority >= event->priority) {
            current = current->next;
        }

        newEventNode->next = current->next;
        current->next = newEventNode;
    }

    queue->size++;
    // Unlock the list once we're done
    sem_post(&queue->mutex);
}

/**
 * Pops an `Event` from the `EventQueue`.
 *
 * Removes the highest priority event from the queue in a thread-safe manner.
 *
 * @param[in,out] queue  Pointer to the `EventQueue`.
 * @param[out]    event  Pointer to the `Event` structure to store the popped event.
 * @return               Non-zero if an event was successfully popped; zero otherwise.
 */
int event_queue_pop(EventQueue *queue, Event *event) {
    if(queue == NULL || event == NULL || queue->head == NULL){
        return 0;
    }

    // Lock list
    sem_wait(&queue->mutex);

    EventNode *temp = queue->head;
    *event = temp -> event;
    queue->head = temp->next;

    free(temp);    
    queue->size--;

    // Unlock list
    sem_post(&queue->mutex);

    return 1;
}
