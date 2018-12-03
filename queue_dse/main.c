//
//  event.c
//  queue_simulation
//
//  Created by Bradley Morgan on 11/29/18.
//  Copyright © 2018 BiT8. All rights reserved.
//
//  This program uses discrete event simulation (a.k.a. event driven simulation)
//  to measure the performance of a M/M/2/10 poisson system.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const int MAX_TIME = 1000; // time interval of sample (or number of packets)
const int MAX_QLEN = 10; // maximum queue length
const int MAX_SERV = 2; // number of servers

const double LAMBDA = 1.0; // intensity, arrival rate (packets per unit time)
const double MU = 1.0; // service rate (processed packets per unit time)

FILE *f;

struct packet {
    
    int id;
    int queue_position;
    
    double arrival_time;
    double departure_time;
    double service_start_time;
    double service_duration;
    double wait_duration;
    double wait_overlap;
    
};

struct queue {
    
    char *id;
    
    int head, tail;
    int len, capacity, lost;
    
    double wait;
    
    struct packet *array;
    
};

struct queue *init_queue(unsigned int capacity, char *qid) {
    
    struct queue *queue = (struct queue *) malloc(sizeof(struct queue));
    
    queue->id = qid;
    queue->capacity = capacity;
    queue->len = queue->lost = 0;
    queue->head = queue->tail = 0;
    
    queue->array = (struct packet *) malloc(MAX_QLEN * sizeof(struct packet));
    
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

double exp_random(double lambda){
    
    // generate an exponentially distributed random number
    // based on the supplied average rate ...
    
    double x;
    
    x = (double)rand() / (double)RAND_MAX;
    
    return -log(1-x) / lambda;
    
}

double service_end(struct packet *p) {
    return p->service_start_time + p->service_duration;
}

double service_wait(struct packet *p) {
    return p->service_start_time + p->arrival_time;
}

void enqueue(struct queue *q, struct packet *p) {
    
    if(full(q)) { q->lost++; return; }
    
    // create a reference to the previous packet
    
    struct packet prev = q->array[q->tail - 1];

    // arrivals occur in exponential distribution
    // lambda * exp(-lambda * x)
    
    p->arrival_time = prev.arrival_time + exp_random(LAMBDA);
    
    // service duration is also exponentially distributed
    
    p->service_duration = exp_random(MU);
    
    // service will start at the time of arrival, if idle
    // or after the previously arriving packet departs, if busy
    
    p->service_start_time = p->arrival_time > service_end(&prev) ? p->arrival_time : service_end(&prev);
    
    // the packet enters the server at the service start time
    // then leaves the system after the service duration
    
    p->departure_time = service_end(p);

    // each queue holds a finite capacity of MAX_QLEN
    // we use a circular array with a head and tail pointer
    // to track the front and back packets
    
    q->array[q->tail] = *p;
    q->tail = (q->tail + 1) % q->capacity;
    
    if(prev.departure_time < p->arrival_time) {
        q->head = (q->head + 1) % q->capacity;
    }
    
    printf("%s enqueued: %d | arrival_time: %2.6f | departure_time: %2.6f | service_start_time: %2.6f | service_duration: %2.6f | head: %d | tail: %d | lost: %d | queue_position: %d\n", q->id, p->id, p->arrival_time, p->departure_time, p->service_start_time, p->service_duration, q->head, q->tail, q->lost, p->queue_position);
    
    fprintf(f, "%s,%d,%2.6f,%2.6f,%2.6f,%2.6f,%d,%d,%d\n", q->id, p->id, p->arrival_time, p->service_start_time, p->service_duration, p->departure_time, q->head, q->tail, q->lost);
    
}

int main(void) {
    
    f = fopen("out.csv", "w");
    
    fprintf(f, "queue,packet,arrival_time,service_start_time,service_duration,departure_time,wait_duration,head,tail,lost\n");
    
    struct queue *q1 = init_queue(MAX_QLEN, "q1");
//    struct queue *q2 = init_queue(MAX_QLEN, "q2");
    
    int t;
    
    for(t = 0; t <= MAX_TIME; t++) {
        
        struct packet *p = init_packet(t);
        
//        int random_packet = rand() % MAX_SERV;
//
//        if(random_packet == 0) {
//
            enqueue(q1, p);
            
//        } else {
//
//            enqueue(q2, p);
//
//        }
//
    }
    
    return 0;
    
}
