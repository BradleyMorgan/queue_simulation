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

    int MAX_ITER = 20; // number of samples, simulation runs
    int QUE_DISC = 1; // 0 for random, 1 for min
    int QUE_PARM = 1; // 0 for lambda, 1 for mu, 2 for load

    double QUE_PMIN = 1.1; // minimum value of chosen parameter above
    double QUE_PMAX = 3.0; // maximum value of chosen parameter above
    double QUE_INCR = 0.1; // increment parameter by this value each iteration

    double LAMBDA = 1.0; // base intensity, arrival rate (packets per unit time)
    double MU = 1.1; // base service rate (processed packets per unit time)

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
        
        double t;
        
    };

    struct queue *init_queue(char *qid, int parameter, double value) {
        
        struct queue *queue = (struct queue *) malloc(sizeof(struct queue));
        
        queue->id = qid;
        queue->capacity = MAX_QLEN;
        
        queue->t = 0.0;
        queue->lambda = parameter == 0 ? value : LAMBDA;
        queue->mu = parameter == 1 ? value : MU;
        queue->load = parameter == 2 ? value : queue->lambda / queue->mu;
        
        queue->head = 0;
        queue->tail = 1;
        queue->len =  0;
        queue->lost = 0;
        
        queue->total_wait_duration = 0.0;
        queue->total_len = 0.0;
        
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
        p->wait_duration = 0.0;
        
        return p;
        
    }

    void display_queue(struct queue *q) {
        
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
        
        double x, r;
        
        x = (double)rand() / (double)RAND_MAX;
        
        r = -log(x) / lambda;
        
        return r;
        
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
        
        p->arrival_time = q->t + exp_random(q->lambda);
        
        // keep track of the total length
        
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

        //     departures     |       queue            |   arrivals
        // [1.00][1.20]|[1.30]|[1.50][1.80][1.90][2.00]|[xxx][xxx][xxx]
        //                    |  ^                 ^   |
        //                    | head              tail |
        
        // we have to find all departure times that occurred before
        // the current (arrival) time and dequeue them by
        // advancing the head and decreasing the count ...
        
        int i;
        
        for(i=0; i <= q->len + 1; i++) {
            int index = (i + q->head) % q->capacity;
            if(p->arrival_time >= q->array[index].departure_time) {
                q->head = (q->head + 1) % q->capacity;
                q->len--;
            }
        }

        // if the queue is not full, we can add the new packer to the
        // array at the tail and increase the length, otherwise
        // the packet is dropped and the loss counter is incremented ...
        
        if(q->head != q->tail + 1 && !(q->head == 0 && q->tail == q->capacity - 1)) {
            q->total_wait_duration += p->wait_duration;
            q->tail = (q->tail + 1) % q->capacity;
            q->array[q->tail] = *p;
            q->len++;
        } else {
            q->lost++;
        }
        
        // the current time is held for reference by the next arrival ...
        
        q->t = p->arrival_time;
        
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
        
        double f1 = (1-q->load) * pow(q->load, MAX_QLEN);
        double f2 = 1 - pow(q->load, MAX_QLEN+1);
        double f3 = f1 / f2;
        
        return f3;
        
    }

    double calc_qlen(struct queue *q) {

        // average queue length is defined as the average number of packets
        // (including the one being served) in an arbitrary queue sampled
        // at the arrivaltime of a packet per project requirements
        
        double f1 = q->load / (1 - q->load);
        double f2 = (MAX_QLEN + 1) * pow(q->load, MAX_QLEN + 1);
        double f3 = 1 - pow(q->load, MAX_QLEN+1);
        double f4 = f2 / f3;
        double f5 = f1 - f4;
        
        // if load is 1, we use a slightly different formula
        
        return f5;
        
    }

    double calc_wait(struct queue *q) {

        // waiting time is defined as the interval from packet arrival
        // to the moment when its service is finished
        
        double f1 = calc_qlen(q);
        double f2 = (1-q->load) * pow(q->load, MAX_QLEN);
        double f3 = 1 - pow(q->load, MAX_QLEN+1);
        double f4 = f2 / f3;
        double f5 = 1 - f4;
        double f6 = q->lambda * f5;
        double f7 = f1 / f6;
        
        return f7;
        
    }

    int main(int argc, char *argv[]) {
        
        if( argc != 7 ) {
            printf("USAGE: \nλ: intensity or arrival rate (i.e. 1.0)\nμ: service rate (i.e. 1.1)\nassignment strategy: (0=random,1=min)\nvariable parameter: (0=lambda,1=mu)\nrange max: (i.e. 3.0)\niterations: number of times to run each simulation\n\n");
            printf("WARNING: Using default values\n");
        } else {
            LAMBDA = atof(argv[1]);
            MU = atof(argv[2]);
            QUE_DISC = atoi(argv[3]);
            QUE_PARM = atof(argv[4]);
            QUE_PMAX = atof(argv[5]);
            QUE_PMIN = QUE_PARM == 0 ? LAMBDA : MU;
            MAX_ITER = atoi(argv[6]);
        }
        
        printf("λ=%2.4f\nμ=%2.4f\nassignment strategy=%d\nvariable parameter=%d\nrange max=%2.4f\n\n--------- BEGIN SIMULATION ---------\n\n", LAMBDA, MU, QUE_DISC, QUE_PARM, QUE_PMAX);
        
        srand((unsigned int)time(NULL));
        
        out1 = fopen("sim.csv", "w");
        out2 = fopen("perf.csv", "w");
        
        char *fname1;
        asprintf(&fname1, "rnd_avg_%d.csv", QUE_PARM);
        char *fname2;
        asprintf(&fname2, "min_avg_%d.csv", QUE_PARM);
        
        out3 = QUE_DISC == 0 ? fopen(fname1, "w") : fopen(fname2, "w");
        
        fprintf(out1, "queue,packet,arrival_time,service_start_time,service_duration,departure_time,head,tail,lost\n");
        fprintf(out2, "seed,λ,μ,ρ,sbp,savglen,tavgwait,savgwait\n");
        
        printf("seed,λ,μ,ρ,tbp,sbp,tavglen,savglen,tavgwait,savgwait\n");
        
        int j, t;
        double i;
        
        for(i=QUE_PMIN; i<=QUE_PMAX; i+=QUE_INCR) {
        
            double tbp = 0.0;
            double tw = 0.0;
            double tslen = 0.0;
            
            struct queue *q1 = init_queue("q1", QUE_PARM, i);;
            struct queue *q2 = init_queue("q2", QUE_PARM, i);;
            
            for(j=0; j<=MAX_ITER - 1; j++) {
                
                q1 = init_queue("q1", QUE_PARM, i);
                q2 = init_queue("q2", QUE_PARM, i);
                
                for(t = 0; t <= MAX_TIME; t++) {
        
                    struct packet *p = init_packet(t);
                    
                    if(QUE_DISC == 0) {
        
                        int random_packet = rand() % MAX_SERV;

                        if(random_packet == 0) { enqueue(q1, p); } else { enqueue(q2, p); }
                        
                    } else {
                        
                        int random_packet = rand() % MAX_SERV;
                        
                        if(q1->len < q2->len) {
                            enqueue(q1, p);
                        } else if(q2->len < q1->len) {
                            enqueue(q2, p);
                        } else {
                            if(random_packet == 0) { enqueue(q1, p); } else { enqueue(q2, p); }
                        }
                        
                    }

                }
                
                double bp = ((q1->lost + q2->lost) / (double)MAX_TIME);
                double slen = (q1->total_len + q2->total_len) / (double)MAX_TIME;
                double w = q1->total_wait_duration / q1->t;
                
                fprintf(out2, "%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->lambda / q1->mu, bp, slen, calc_wait(q1), w);
                
                //printf("%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.5f,%3.5f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->lambda / q1->mu, calc_bp(q1), bp, calc_qlen(q1), slen, calc_wait(q1), w);
                
                tbp += bp;
                tw += w;
                tslen += slen;
                
            }
            
            fprintf(out3, "%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->load, calc_bp(q1), tbp / (double)MAX_ITER, calc_qlen(q1), tslen / (double)MAX_ITER, calc_wait(q1), tw / (double)MAX_ITER);
            
            printf("%d,%2.2f,%2.2f,%3.6f,%3.6f,%3.6f,%3.5f,%3.5f,%3.6f,%3.6f\n", j, q1->lambda, q1->mu, q1->load, calc_bp(q1), tbp / (double)MAX_ITER, calc_qlen(q1), tslen / (double)MAX_ITER, calc_wait(q1), tw / (double)MAX_ITER);
            
        }
        
        return 0;
        
    }
