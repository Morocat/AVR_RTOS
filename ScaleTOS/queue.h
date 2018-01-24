/*
 * queue.h
 *
 * Created: 1/8/2018 1:38:37 PM
 *  Author: alelop
 */ 

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct QueueNode{
	void* data;
	struct QueueNode *next;
	uint16_t priority;
}QueueNode;

typedef struct Queue{
	QueueNode *head;
	uint8_t size;
	bool hasPriority;
}Queue;

//void queue_add(Queue *queue, void *item);
void queue_add_with_priority(Queue *queue, void *item, uint16_t priority);
void* queue_take(Queue *queue);
void* queue_peek(Queue *queue);
void queue_remove(Queue *queue, void *data);
void queue_sort(Queue *queue);
QueueNode* queue_find_node(Queue *queue, void* data);

#endif /* QUEUE_H_ */