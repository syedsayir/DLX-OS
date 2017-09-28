#ifndef __USERPROG__
#define __USERPROG__

#define BUFSIZE (BUFFERSIZE+1)
typedef struct cir_buf {
	int head;
	int tail;
	char buffer[BUFSIZE];
} cir_buf;

int isEmpty(cir_buf *input) {
	return (input->head==input->tail);
}

void incHead(cir_buf *input) {
	input->head = ((input->head + 1) % BUFSIZE);
}

void incTail(cir_buf *input) {
	input->tail = ((input->tail + 1) % BUFSIZE);
}


int isFull(cir_buf *input) {
	return ((input->head+1)%BUFSIZE==input->tail);
}

#define FILENAME_TO_RUN_P "producer.dlx.obj"
#define FILENAME_TO_RUN_C "consumer.dlx.obj"

#endif
