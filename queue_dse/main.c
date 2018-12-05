//
//  event.c
//  queue_simulation
//
//  Created by Bradley Morgan on 11/29/18.
//  Copyright © 2018 BiT8. All rights reserved.
//
//  This program uses discrete event simulation (a.k.a. event driven simulation)
//  to measure the performance of a two queue m/m/1 poisson system.
//
//  departure<-[head][*][*][*][tail]<-arrival
//
//  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

const int MAX_TIME = 10000; // time interval of sample (or number of packets)
const int MAX_QLEN = 10; // maximum queue length
const int MAX_SERV = 2; // number of servers
const int MAX_ITER = 50;

const double LAMBDA = 1.0; // base intensity, arrival rate (packets per unit time)
const double MU = 1.1; // base service rate (processed packets per unit time)

FILE *out1;
FILE *out2;
FILE *out3;

//  create an arrival (packet) data structure
//  and a queue data structure holding an array of these arrivals

struct packet {
    
    int id;
    
    double arrival_time;
    double departure_time;
    double service_start_time;
    double service_duration;
    double wait_duration;
    
};

//  simulate the finite queue using a circular array
//  data structure to track arrivals and departures, i.e...
//
//  departure<-[head][*][*][*][tail]<-arrival

struct queue {
    
    char *id;
    
    int head, tail;
    int len, capacity, lost;
    
    double lambda, mu, load;
    
    double total_wait_duration;
    double total_len;
    
    struct packet *array;
    
};

struct queue *init_queue(char *qid) {
    
    struct queue *queue = (struct queue *) malloc(sizeof(struct queue));
    
    queue->id = qid;
    queue->capacity = MAX_QLEN;
    
    queue->lambda = LAMBDA;
    queue->mu = MU;
    
    queue->head = 0;
    queue->tail = 1;
    queue->len =  1;
    queue->lost = 0;
    
    queue->array = (struct packet *) malloc(MAX_QLEN * sizeof(struct packet));
    
    return queue;
    
}

struct packet *init_packet(int id) {
    
    struct packet *p = (struct packet *) malloc(sizeof(struct packet));
    
    p->id = id;
    p->arrival_time = 0.0;
    p->departure_time = 0.0;
    p->service_start_time = 0.0;
    p->service_duration = 0.0;
    
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
    
    return -log(x) / lambda;
    
}

double service_end(struct packet *p) {
    return p->service_start_time + p->service_duration;
}

double service_wait(struct packet *p) {
    return p->service_start_time + p->arrival_time;
}

void enqueue(struct queue *q, struct packet *p) {
    
    // create a reference to the previous packet
    
    struct packet prev = q->array[q->tail];

    // if the queue is full, we cannot use only the previous arrival time
    // as a reference, because the tail is not advanced when the
    // queue is at capacity, so we use the arrival time of the first
    // packet and add a random exponential value for each packet in the
    // queue to approximate the arrival time of a previously dropped packet

    // arrivals occur in exponential distribution
    // lambda * exp(-lambda * x)
    
    if(q->head == q->tail + 1 || (q->head == 0 && q->tail == MAX_QLEN - 1)) {
        p->arrival_time = q->array[q->head].arrival_time + (exp_random(q->lambda) * (q->capacity));
        q->lost++;
    } else {
        p->arrival_time = prev.arrival_time + exp_random(q->lambda);
    }
    
    q->total_len += q->len;
    
    // service duration is also exponentially distributed
    
    p->service_duration = exp_random(q->mu);
    
    // service will start at the time of arrival, if idle
    // or after the previously arriving packet departs, if busy
    
    p->service_start_time = p->arrival_time > service_end(&prev) ? p->arrival_time : service_end(&prev);
    
    // the packet enters the server at the service start time
    // then leaves the system after the service duration
    
    p->departure_time = service_end(p);
    
    // a packet is in the queue until the moment of service
    
    p->wait_duration = p->departure_time - p->arrival_time;
    
    // each queue holds a finite capacity of MAX_QLEN
    // we use a circular array with a head and tail pointer
    // to track the front and back packets

    // [1.0][1.4][1.6][1.5][1.8][1.12][1.9][1.9][1.10][1.11]
    
    int i;
    
    for(i=0; i <= q->len; i++) {
        int index = (i + q->head) % q->capacity;
        if(p->arrival_time >= q->array[index].departure_time) {
            q->head = (q->head + 1) % q->capacity;
            q->len--;
        }
    }

    if(q->head != q->tail + 1 && !(q->head == 0 && q->tail == 9)) {
        q->tail = (q->tail + 1) % q->capacity;
        q->len++;
        q->array[q->tail] = *p;
        q->total_wait_duration += p->wait_duration;
        
    }

    //printf("%s enqueued: %d | arrival_time: %2.6f | departure_time: %2.6f | service_start_time: %2.6f | service_duration: %2.6f | head: %d | tail: %d | len: %d | lost: %d\n", q->id, p->id, p->arrival_time, p->departure_time, p->service_start_time, p->service_duration, q->head, q->tail, q->len, q->lost);
    
    fprintf(out1, "%s,%d,%2.6f,%2.6f,%2.6f,%2.6f,%d,%d,%d\n", q->id, p->id, p->arrival_time, p->service_start_time, p->service_duration, p->departure_time, q->head, q->tail, q->lost);
    
}

float factorial(float f) {
    
    if ( f == 0.0 ) {
        return 1.0;
    }
    
    float res = f * factorial(f - 1.0);
    
    return res;
}

double calc_bp(struct queue *q) {
    
    // for an m/m/1/k queue, load is arrival rate divided by the service rate
    // multiplied by the number of servers ...
    
    double load = q->lambda / q->mu;
    
    double f1 = (1-load) * pow(load, MAX_QLEN);
    double f2 = 1 - pow(load, MAX_QLEN+1);
    double f3 = f1 / f2;
    
    return f3;
    
}

double calc_qlen(struct queue *q) {
    
    double load = q->lambda / q->mu;
    
    double f1 = load / (1 - load);
    double f2 = (MAX_QLEN + 1) * pow(load, MAX_QLEN+1);
    double f3 = 1 - pow(load, MAX_QLEN+1);
    
    double f4 = f2 / f3;
    double f5 = f1 - f4;
    
    return f5;
    
}

double calc_wait(struct queue *q) {
    
    double load = q->lambda / q->mu;
    
    double f1 = calc_qlen(q);
    double f2 = (1-load) * pow(load, MAX_QLEN);
    double f3 = 1 - pow(load, MAX_QLEN+1);
    double f4 = f2 / f3;
    double f5 = 1 - f4;
    double f6 = q->lambda * f5;
    double f7 = f1 / f6;
    
    return f7;
    
}

int main(void) {
    
    srand((unsigned int)time(NULL));
    
    out1 = fopen("sim.csv", "w");
    out2 = fopen("perf.csv", "w");
    out3 = fopen("avg.csv", "w");
    
    fprintf(out1, "queue,packet,arrival_time,service_start_time,service_duration,departure_time,head,tail,lost\n");
    fprintf(out2, "seed,λ,μ,ρ,tbp,sbp,tavglen,savglen,tavgwait,savgwait\n");
    
    printf("seed,λ,μ,ρ,tbp,sbp,tavglen,savglen,tavgwait,savgwait\n");
    
    int j, t;
    double i;
    
    for(i=MU; i<=2.0; i+=0.1) {
    
        double tbp = 0.0;
        double tw = 0.0;
        double tslen = 0.0;
        
        struct queue *q1;
        struct queue *q2;
        
        for(j=0; j<=MAX_ITER - 1; j++) {
            
            q1 = init_queue("q1");
            q2 = init_queue("q2");
            
            q1->mu = i;
            q2->mu = i;
            
            for(t = 0; t <= MAX_TIME; t++) {
                
                struct packet *p = init_packet(t);
                
                int random_packet = rand() % MAX_SERV;

                if(random_packet == 0) {

                    enqueue(q1, p);
                
                } else {

                    enqueue(q2, p);

                }

            }
            
            double bp = ((q1->lost + q2->lost) / (double)MAX_TIME);
            double w = (q1->total_wait_duration + q2->total_wait_duration) / (double)MAX_TIME;
            double slen = (q1->total_len + q2->total_len) / (double)MAX_TIME;
            
            fprintf(out2, "%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->lambda / q1->mu, calc_bp(q1), bp, calc_qlen(q1), slen, calc_wait(q1), w);
            
            printf("%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.5f,%3.5f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->lambda / q1->mu, calc_bp(q1), bp, calc_qlen(q1), slen, calc_wait(q1), w);
            
            tbp += bp;
            tw += w;
            tslen += slen;
            
        }
        
        printf("-----------------------------\n");
        
        fprintf(out2, "%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->lambda / q1->mu, calc_bp(q1), tbp / MAX_ITER, calc_qlen(q1), tslen / MAX_ITER, calc_wait(q1), tw / MAX_ITER);
        
        printf("%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.5f,%3.5f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->lambda / q1->mu, calc_bp(q1), tbp / MAX_ITER, calc_qlen(q1), tslen / MAX_ITER, calc_wait(q1), tw / MAX_ITER);
        
        printf("-----------------------------\n");
        
    }
    
    return 0;
    
}
