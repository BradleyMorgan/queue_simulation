    //
    //  main.c
    //  queue_simulation
    //
    //  Created by Bradley Morgan on 11/14/18.
    //  Copyright © 2018 BiT8. All rights reserved.
    //
    //

    // this program simulates a queue using a circular array
    // data structure to track arrivals and departures, i.e...
    //
    // departure<-[head][*][*][*][tail]<-arrival
    //
    // this data structure is used as an efficient way
    // to simulate a queue at O(1) complexity (if implemented
    // correctly)
    //
    // this implementation uses an arrival (packet) data structure
    // and a queue data structure holding an array of these arrivals
    //
    // arrival rate (λ) and service rate (µ) are defined
    //
    //

    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <sys/time.h>
    #include <math.h>
    #include <unistd.h>
    #include <string.h>

    const int MAX_PACKETS = 10000;
    const int MAX_QUEUE = 9;
    const int MAX_SERVERS = 2;

    const float TIME_ARRIVAL_MS = 100000.0;
    const float TIME_SERVICE_MS = 200000.0;

    struct packet {
        
        int id;
        int queue_position;
        
        struct timeval arrival_time;
        struct timeval departure_time;
        
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

    void enqueue(struct queue *q, struct packet *p) {
        
        if(full(q)) { q->len = q->capacity; q->lost++; return; }
        
        struct timeval t;
        
        gettimeofday(&t, 0);
        
        p->arrival_time = t;
        q->array[q->tail] = *p;
        q->tail = (q->tail + 1) % q->capacity;
        q->len++;
        
        printf("%s enqueued: 1 | head: %d | tail: %d | count %d | lost: %d | queue: ",q->id, q->head, q->tail, q->len, q->lost);
        display(q);
        
    }

    void dequeue(struct queue *q) {
        
        if(empty(q)) { q->len = 0; return; }
        
        struct timeval t;
        
        gettimeofday(&t, 0);

        q->array[q->head].departure_time = t;
        
        q->head = (q->head + 1) % q->capacity;
        q->len--;
        
        printf("%s dequeued: 1 | head: %d | tail: %d | count %d | lost: %d | queue: ",q->id, q->head, q->tail, q->len, q->lost);
        display(q);
        
    }

    float factorial(float f) {
        
        if ( f == 0.0 ) {
            return 1.0;
        }
        
        float res = f * factorial(f - 1.0);
        
        return res;
    }

    float calc_bp(float lambda, float mu) {
        
        // for an m/m/c/k queue, load is arrival rate divided by the service rate
        // multiplied by the number of servers ...
        
        float load = lambda / (MAX_SERVERS * mu);
        
        //float k = MAX_QUEUE * MAX_SERVERS;
        
        float f1 = powf(load, MAX_SERVERS) / factorial(MAX_SERVERS);
    
        printf("\nload: %3.5f, f1: %3.5f ", load, f1);
        
        float f2 = 0.0;
        
        int i;
        
        for (i = 0; i <= MAX_SERVERS; i++) {
            f2 += (powf(load, i) / factorial(i));
        }
        
        float f3 = f1 / f2;
        
        printf(" f2: %3.5f, f3: %3.5f", f2, f3);
        
        return f3;
        
    }

    int main(int argc, const char * argv[]) {
        
        // initialize the queues
        
        calc_bp(TIME_ARRIVAL_MS, TIME_SERVICE_MS);
        
        struct queue *q1 = init_queue(MAX_QUEUE, "1Q");
        struct queue *q2 = init_queue(MAX_QUEUE, "2Q");
        struct timespec start, end, t;
        
        int i;

        for(i = 0; i <= (q1->capacity - 1); i++) {
            struct packet *p = init_packet(i);
            enqueue(q1, p);
            enqueue(q2, p);
        }
        
        for(i = 0; i <= (q1->capacity - 1); i++) {
            dequeue(q1);
            dequeue(q2);
        }
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        printf("\n*** start ***\n\n");
        
        for(i = 0; i <= MAX_PACKETS; i++) {

            int random_packet = rand() % MAX_SERVERS;
            
            struct packet *p = init_packet(i);
            
            do {
                
                clock_gettime(CLOCK_MONOTONIC, &t);
                
            } while(fmod(t.tv_nsec, TIME_ARRIVAL_MS) != 0.0 && fmod(t.tv_nsec, TIME_SERVICE_MS) != 0.0);
            
            if(fmod(t.tv_nsec, TIME_SERVICE_MS) == 0.0) {
                
                printf("DEPARTURE: %lu\n", t.tv_nsec);
                
                dequeue(q1);
                dequeue(q2);
                
            }
            
            if(fmod(t.tv_nsec, TIME_ARRIVAL_MS) == 0.0) {
                
                printf("ARRIVAL: %lu\n", t.tv_nsec);
                
                if(random_packet == 0) {

                    enqueue(q1, p);

                } else {

                    enqueue(q2, p);

                }
                
            }
            
        }
        
        float blocking_probability = ((float)q1->lost + (float)q1->lost) / (float)MAX_PACKETS;
        
        printf("λ,µ,ρ,bp,\n");
        printf("%g,%g,%5.5f,%5.5f\n", TIME_ARRIVAL_MS, TIME_SERVICE_MS, (TIME_ARRIVAL_MS / (TIME_SERVICE_MS * 2.0)), blocking_probability);
        
        printf("Blocking Probability (%d + %d / %d): %5.5f\n", q1->lost, q2->lost, MAX_PACKETS, blocking_probability);
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        return 0;
        
    }
