#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef unsigned int uint;
typedef unsigned long int uint64_t;

volatile uint a = 0;
volatile uint b = 0;

volatile bool stop = false;

void *reader(void *p) {
    uint64_t iter_counter = 0;
    uint cnt_less = 0,
         cnt_eq = 0,
         cnt_more = 0;

    uint aa, bb;

    uint old_aa, old_bb;

    printf("reader started\n");

    while(!stop) {
        iter_counter++;
        bb = b;

        if (bb != old_bb) {
            asm volatile ("mfence" ::: "memory");

            aa = a;

            if (old_aa < aa ) {
                cnt_less++;
            }
            else if (old_aa > aa) {
                cnt_more++;
            } else {
                cnt_eq++;
            }
            old_aa = aa;
            old_bb = bb;
        }
        asm volatile ("mfence" ::: "memory");

//        asm volatile ("mfence" ::: "memory");
    }
    printf("iters=%lu, less=%u, eq=%u, more=%u\n", iter_counter, cnt_less, cnt_eq, cnt_more);

    return NULL;
}

void *writer(void *p) {
    printf("writer started\n");
    uint counter = 0;
    while(!stop) {
        a = counter;
        // asm volatile ("" ::: "memory");
        asm volatile ("mfence" ::: "memory");
        b = counter;
        asm volatile ("mfence" ::: "memory");

        counter++;
    }
}

int main() {
    pthread_t reader_thr, writer_thr;
    int t1 = pthread_create(&reader_thr, NULL, reader, NULL);
    int t2 = pthread_create(&writer_thr, NULL, writer, NULL);
    if (t1 !=0 || t2!=0) { printf("fail start\n");
        return 1;
    }

    void *ret;

    sleep(3);
    stop = true;
    pthread_join(reader_thr, &ret);
    pthread_join(writer_thr, &ret);
}

