#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "buffer_client.h"

cbuf_handle_t circular_buf_init() {
	
	uint8_t* tx_buffer;
	cbuf_handle_t cbuf;
	
    tx_buffer  = malloc(BUFFER_MAX_SIZE * sizeof(uint8_t));
	cbuf = malloc(sizeof(circular_buf_t));
	cbuf->buffer = tx_buffer;
	cbuf->max = BUFFER_MAX_SIZE;
	circular_buf_reset(cbuf);
	return cbuf;
}

void circular_buf_reset(cbuf_handle_t cbuf) {

    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

void circular_buf_free(cbuf_handle_t cbuf) {

	free(cbuf->buffer);
	free(cbuf);
}

bool circular_buf_full(cbuf_handle_t cbuf) {

    return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf) {
    
    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

size_t circular_buf_capacity(cbuf_handle_t cbuf) {

	return cbuf->max;
}

size_t circular_buf_size(cbuf_handle_t cbuf) {

	size_t size = cbuf->max;

	if(!cbuf->full)
	{
		if(cbuf->head >= cbuf->tail)
		{
			size = (cbuf->head - cbuf->tail);
		}
		else
		{
			size = (cbuf->max + cbuf->head - cbuf->tail);
		}
	}

	return size;
}

static void advance_pointer(cbuf_handle_t cbuf) {

	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf) {
	
	cbuf->full = false;
	cbuf->head = (cbuf->head + 1) % cbuf->max;
}

void save_current_head_pointer(cbuf_handle_t cbuf) {

	cbuf->temp_head = cbuf->head;
}

void update_pointer(cbuf_handle_t cbuf) {

	cbuf->head = cbuf->temp_head;
}

int circular_buf_put(cbuf_handle_t cbuf, uint8_t data) {

    int r = -1;

    if(!circular_buf_full(cbuf))
    {
		cbuf->buffer[cbuf->tail] = data;
		advance_pointer(cbuf);
        r = 0;
    }
    return r;
}

int circular_buf_get(cbuf_handle_t cbuf, uint8_t* data) {

    int r = -1;

    if(!circular_buf_empty(cbuf))
    {
        *data = cbuf->buffer[cbuf->head];
        retreat_pointer(cbuf);

        r = 0;
    }
    return r;
}