    //
    //  main.c
    //  queue_simulation
    //
    //  Created by Bradley Morgan on 11/14/18.
    //  Copyright © 2018 BiT8. All rights reserved.
    //

    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <sys/time.h>
    #include <math.h>
    #include <unistd.h>
    #include <string.h>

    const int MAX_PACKETS = 100;
    const int MAX_QUEUE = 9;
    const float TIME_ARRIVAL_MS = 1.0;
    const float TIME_SERVICE_MS = 1.0;

    struct packet {
        int id;
        int queue_position;
        struct timeval arrival_time;
        struct timeval departure_time;
    };

    struct queue {
        char *id;
        int head, tail, size, lost;
        unsigned int capacity;
        struct packet *array;
    };

    struct queue *init_queue(unsigned int capacity, char *qid) {
        
        struct queue *queue = (struct queue *) malloc(sizeof(struct queue));
        
        queue->id = qid;
        queue->capacity = capacity;
        queue->head = queue->size = queue->lost = 0;
        queue->tail = -1;
        
        queue->array = (struct packet *) malloc(queue->capacity * sizeof(struct packet));

        return queue;
        
    }

    struct packet *init_packet(int id) {
        
        struct packet *p = (struct packet *) malloc(sizeof(struct packet));
        
        p->id = id;
        
        return p;
        
    }

    int full(struct queue *q) {
        return q->size == q->capacity;
    }

    int empty(struct queue *q) {
        return q->size == 0;
    }

    void display(struct queue *q) {
        
        int i;
        
        for(i = 0; i <= (q->size - 1); i++) {
        
            printf("%d ", q->array[i].id);
        
        }
        
        printf("\n");
        
    }

    void enqueue(struct queue *q, struct packet *p) {
        
        if(full(q)) { q->lost++; return; }
        
        struct timeval t;
        
        gettimeofday(&t, 0);
        
        p->arrival_time = t;
        q->tail = (q->tail + 1) % q->capacity;
        q->array[q->tail] = *p;
        q->size++;
        
        printf("%s enqueued: 1 | head: %d | tail: %d | count %d | lost: %d | queue: ",q->id, q->head, q->tail, q->size, q->lost);
        display(q);
        
    }

    void dequeue(struct queue *q) {
        
        if(empty(q)) { q->head = q->tail = 0; q->size = 0; return; }
        
        struct timeval t;
        
        gettimeofday(&t, 0);

        struct packet p = q->array[q->tail];
        
        p.departure_time = t;
        
        q->head = (q->head + 1) % q->capacity;
        q->size--;
        
        printf("%s dequeued: 1 | head: %d | tail: %d | count %d | lost: %d | queue: ",q->id, q->head, q->tail, q->size, q->lost);
        display(q);
        
    }

    int main(int argc, const char * argv[]) {
        
        // initialize the queues
        
        struct queue *q1 = init_queue(MAX_QUEUE, "1Q");
        struct queue *q2 = init_queue(MAX_QUEUE, "2Q");
        struct timeval start, end, t;
        
        int i;

        struct packet *p = init_packet(0);
        
        for(i = 0; i <= (q1->capacity - 1); i++) {
            enqueue(q1, p);
            enqueue(q2, p);
        }
        
        for(i = 0; i <= (q1->capacity - 1); i++) {
            dequeue(q1);
            dequeue(q2);
        }
        
        gettimeofday(&start, 0);
        
        for(i = 0; i <= MAX_PACKETS; i++) {
        
            gettimeofday(&t, 0);
        
            struct packet *p = init_packet(i);
            
            if(fmod(t.tv_usec, TIME_SERVICE_MS) == 0) {
                
                dequeue(q1);
                dequeue(q2);
                
            }
            
            int random_packet = rand() % 2;

            if(random_packet == 0) {

                if(fmod(t.tv_usec, TIME_ARRIVAL_MS) == 0) {

                    enqueue(q1, p);
                    
                }
                
            } else {
                
                if(fmod(t.tv_usec, TIME_ARRIVAL_MS) == 0) {
                    
                    enqueue(q2, p);
                    
                }
                
            }
            
        }
        
        float blocking_probability = ((float)q1->lost + (float)q1->lost) / (float)MAX_PACKETS;
        
        printf("λ,µ,ρ,bp,\n");
        printf("%g,%g,%5.5f,%5.5f\n", TIME_ARRIVAL_MS, TIME_SERVICE_MS, (TIME_ARRIVAL_MS / (TIME_SERVICE_MS * 2.0)), blocking_probability);
        
        printf("Blocking Probability (%d + %d / %d): %5.5f\n", q1->lost, q2->lost, MAX_PACKETS, blocking_probability);
        
        gettimeofday(&end, 0);
        
        return 0;
        
    }
