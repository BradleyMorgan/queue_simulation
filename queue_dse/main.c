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

const int MAX_TIME = 10000;
const int MAX_QLEN = 10;
const int MAX_SERV = 2;

const double LAMBDA = 1.0;
const double MU = 4.0;

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
    
    queue->array = (struct packet *) malloc(MAX_TIME * sizeof(struct packet));
    
    return queue;
    
}

struct packet *init_packet(int id) {
    
    struct packet *p = (struct packet *) malloc(sizeof(struct packet));
    
    p->id = id;
    p->arrival_time = 0.0;
    
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

double exp_random(double mean){
    
    // generate an exponentially distributed random number
    // based on the supplied rate ...
    
    double u;
    
    u = (double)rand() / (double)RAND_MAX;
    
    return -log(1-u) / mean;
    
}

double service_end(struct packet *p) {
    return p->service_start_time + p->service_duration;
}

double service_wait(struct packet *p) {
    return p->service_start_time + p->arrival_time;
}

void enqueue(struct queue *q, struct packet *p) {
    
    struct packet prev = q->len > 0 ? q->array[q->len] : *p;

    p->service_duration = exp_random(1.0 / MU);
    p->arrival_time = q->len > 0 ? prev.arrival_time + exp_random(1.0 / LAMBDA) : 0.0;
    p->service_start_time = p->arrival_time > service_end(&prev) ? p->arrival_time : service_end(&prev);
    p->departure_time = service_end(p);
    
    q->len++;
    q->array[q->len] = *p;
    q->tail = q->len;
    q->head = q->len - q->capacity;
    
    printf("%s enqueued: %d | arrival_time: %2.6f | service_time: %2.6f | service_duration: %2.6f | departure_time: %2.6f | count %d\n",q->id, p->id, p->arrival_time, p->service_start_time, p->service_duration, p->departure_time, q->len);
    
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
