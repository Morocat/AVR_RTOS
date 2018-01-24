/*
 * queue.c
 *
 * Created: 1/8/2018 1:38:28 PM
 *  Author: alelop
 */ 
 #include "queue.h"

 #include "mem_manage.h"
 #include "common.h"
 #include "config.h"
 #include <string.h>
 //#include <stdlib.h>

 void queue_add_with_priority(Queue *queue, void *item, uint16_t priority) {
	 QueueNode *n, *cur, *prev;

	 m_assert(queue != NULL);
	 queue->hasPriority = true;
	 n = m_malloc(sizeof(QueueNode));
	 m_assert(n != NULL);
	 memset(n, 0, sizeof(QueueNode));
	 n->data = item;
	 n->priority = priority;
	 queue->size++;
	 m_assert(queue->size < 255);

	 cur = queue->head;
	 if (cur == NULL || priority <= cur->priority) {
		 n->next = queue->head;
		 queue->head = n;
	 }
	 else {
		 prev = queue->head;
		 while (cur != NULL && cur->priority < priority) {
			 prev = cur;
			 cur = cur->next;
		 }
		 n->next = cur;
		 prev->next = n;
	 }
 }

 void* queue_take(Queue *queue) {
	 QueueNode *n;
	 void *data;
	 m_assert(queue != NULL);
	 if (queue->head == NULL)
	 return NULL;

	 n = queue->head;
	 data = n->data;
	 queue->head = n->next;
	 free(n);
	 queue->size--;

	 return data;
 }

 void queue_remove(Queue *queue, void *data) {
	 QueueNode *cur, *prev;

	 if (queue->size == 0)
		return;
	 queue->size--;

	 prev = cur = queue->head;
	 if (cur->data == data)
		queue->head = cur->next;
	 else {
		 while (cur != NULL && cur->data != data) {
			 prev = cur;
			 cur = cur->next;
		 }
		 if (cur == NULL)
			return;
		 prev->next = cur->next;
	 }
	 //memset(cur, 0, sizeof(QueueNode));
	 free(cur);
 }

void queue_sort(Queue *queue) {
	int i, j;
	QueueNode *prev = NULL, *cur;

	if (queue->size <= 1)
		return;

	prev = queue->head;
	cur = prev;

	for (i = 0; i < queue->size; i++) {
		for (j = 0; j < queue->size - 1; j++) {
			if (cur->priority > cur->next->priority) {
				if (j == 0) {
					prev = queue->head;
					queue->head = cur->next;
					prev->next = cur->next->next;
					queue->head->next = prev;
					cur = queue->head;
				} else {
					prev->next = cur->next;
					cur->next = cur->next->next;
					prev->next->next = cur;
					cur = prev->next;
				}
			}
			prev = cur;
			cur = cur->next;
		}
		prev = queue->head;
		cur = prev;
	}
}

 void* queue_peek(Queue *queue) {
	 return queue->head->data;
 }

 QueueNode* queue_find_node(Queue *queue, void* data) {
	QueueNode *n = queue->head;

	while (n != NULL && n->data != data)
		n = n->next;

	return n;
 }

 //#if DEBUG_TRACE
 //void queue_print(Queue *queue) {
	 //QueueNode *n;
	 //int i = 0;
	 //n = queue->head;
//
	 //while (n != NULL) {
		 //printf("Node %d\n", i++);
		 //printf("Addr: %d\n", n);
		 //printf("Priority: %d\n", n->priority);
		 //printf("Data Addr: %d\n", n->data);
		 //printf("Next Addr: %d\n", n->next);
		 //printf("\n");
		 //n = n->next;
	 //}
 //}
//#endif