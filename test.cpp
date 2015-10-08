#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <atomic>

typedef unsigned int uint;
typedef unsigned long int uint64_t;

std::atomic<uint> a(0);
std::atomic<uint> b(0);

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
        bb = b.load();
        atomic_thread_fence(std::memory_order_seq_cst);

        if (bb != old_bb) {

            aa = a.load();

            if (old_aa < aa ) {
                cnt_less++;
            } else if (old_aa > aa) {
                cnt_more++;
            } else {
                cnt_eq++;
                printf("old_aa=%u old_bb=%u; aa=%u bb=%u\n", old_aa, old_bb, aa,bb);
            }
            old_aa = aa;
            old_bb = bb;
        }

    }
    printf("iters=%lu, less=%u, eq=%u, more=%u\n", iter_counter, cnt_less, cnt_eq, cnt_more);

    return NULL;
}

void *writer(void *p) {
    printf("writer started\n");
    uint counter = 0;
    while(!stop) {
        a.store(counter);
        b.store(counter);

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

