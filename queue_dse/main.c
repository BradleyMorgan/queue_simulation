//
//  event.c
//  queue_simulation
//
//  Created by Bradley Morgan on 11/29/18.
//  Copyright © 2018 BiT8. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const int MAX_TIME = 100000;
const int MAX_QLEN = 10;
const int MAX_SERV = 2;

const int LAMBDA = 1;
const int MU = 2;

struct packet {
    
    int id;
    int queue_position;
    
    double arrival_time;
    double departure_time;
    double service_start_time;
    double service_duration;
    
};

struct queue {
    
    char *id;
    
    int head, tail;
    int len, capacity, lost;
    
    struct packet *array;
    
};

struct queue *init_queue(unsigned int capacity, char *qid) {
    
    struct queue *queue = (struct queue *) malloc(sizeof(struct queue));
    
    queue->id = qid;
    queue->capacity = capacity;
    queue->len = queue->lost = 0;
    queue->head = queue->tail = 0;
    
    queue->array = (struct packet *) malloc(queue->capacity * sizeof(struct packet));
    
    return queue;
    
}

struct packet *init_packet(int id) {
    
    struct packet *p = (struct packet *) malloc(sizeof(struct packet));
    
    p->id = id;
    
    return p;
    
}

int full(struct queue *q) {
    
    return q->len >= q->capacity;
    
}

int empty(struct queue *q) {
    
    return q->len == 0;
    
}

void display(struct queue *q) {
    
    if (empty(q)) { printf("\n"); return; }
    
    int i;
    
    if(q->head < q->tail) {
        
        for(i = q->head; i < q->tail; i++) {
            printf("%d ", q->array[i].id);
        }
        
    } else {
        
        for(i = q->head; i < q->capacity; i++) {
            printf("%d ", q->array[i].id);
        }
        
        for(i = 0; i < q->tail; i++) {
            printf("%d ", q->array[i].id);
        }
        
    }
    
    printf("\n");
    
}

double exp_random(int lambda){
    
    // generate an exponentially distributed random number
    // based on the arrival rate ...
    
    double u;
    
    u = rand() / (RAND_MAX + 1.0);
    
    return -log(1- u) / lambda;
    
}

double service_end(struct packet *p) {
    return p->service_start_time + p->service_duration;
}

double service_wait(struct packet *p) {
    return p->service_start_time + p->arrival_time;
}

void enqueue(struct queue *q, struct packet *p) {
    
    if(full(q)) { q->len = q->capacity; q->lost++; return; }
    
    struct packet prev = q->array[q->tail-1];
    
    p->service_duration = exp_random(MU);
    p->arrival_time = prev.arrival_time + exp_random(LAMBDA);
    p->service_start_time = p->arrival_time > service_end(&prev) ? p->arrival_time : service_end(&prev);
    p->departure_time = service_end(p);
    
    q->array[q->tail] = *p;
    q->tail = (q->tail + 1) % q->capacity;
    q->len++;
    
    printf("%s enqueued: 1 | arrival_time: %2.6f | service_time: %2.6f | departure_time: %2.6f | head: %d | tail: %d | count %d | lost: %d | queue: ",q->id, p->arrival_time, p->service_start_time, p->departure_time, q->head, q->tail, q->len, q->lost);
    display(q);
    
}

void dequeue(struct queue *q) {
    
    if(empty(q)) { q->len = 0; return; }

    //q->array[q->head].departure_time = t;
    
    q->head = (q->head + 1) % q->capacity;
    q->len--;
    
    printf("%s dequeued: 1 | head: %d | tail: %d | count %d | lost: %d | queue: ",q->id, q->head, q->tail, q->len, q->lost);
    display(q);
    
}

int main(void) {
    
    struct queue *q1 = init_queue(MAX_QLEN, "q1");
    struct queue *q2 = init_queue(MAX_QLEN, "q2");
    
    int t;
    
    for(t = 0; t <= MAX_TIME; t++) {
        
        struct packet *p = init_packet(t);
        
        int random_packet = rand() % MAX_SERV;
        
        if(random_packet == 0) {
            
            enqueue(q1, p);
            
        } else {
            
            enqueue(q2, p);
            
        }
        
    }
    
    return 0;
    
}
