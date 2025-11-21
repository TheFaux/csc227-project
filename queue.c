#define CQUEUE_CAPACITY 100

typedef struct {
    int data[CQUEUE_CAPACITY];
    int head;
    int tail;
} CQueue;

CQueue cq_new(void) {
    CQueue q;
    q.head = 0;
    q.tail = 0;
    return q;
}

void cq_clear(CQueue *q) {
    q->head = 0;
    q->tail = 0;
}

void cq_enqueue(CQueue *q, int value) {
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % CQUEUE_CAPACITY;
}

int cq_dequeue(CQueue *q) {
    int value = q->data[q->head];
    q->head = (q->head + 1) % CQUEUE_CAPACITY;
    return value;
}

int cq_peek(const CQueue *q) {
    return q->data[q->head];
}

size_t cq_size(const CQueue *q) {
    return (q->tail + CQUEUE_CAPACITY - q->head) % CQUEUE_CAPACITY;
}

int cq_is_empty(const CQueue *q) {
    return q->head == q->tail;
}

int cq_is_full(const CQueue *q) {
    return ((q->tail + 1) % CQUEUE_CAPACITY) == q->head;
}
