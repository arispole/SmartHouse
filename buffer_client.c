#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>

#include "buffer_client.h"

// Opaque circular buffer structure
struct circul_buf_t {
	uint8_t* buffer;
	size_t head;
	size_t temp_head;
	size_t tail;
	size_t max; 
	bool full;
};

cbuf_handle_t circular_buf_init(uint8_t* buffer, size_t size) {
	
    assert(buffer && size);

	cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
	assert(cbuf);

	cbuf->buffer = buffer;
	cbuf->max = size;
	circular_buf_reset(cbuf);

	assert(circular_buf_empty(cbuf));

	return cbuf;
}

void circular_buf_reset(cbuf_handle_t cbuf) {
    assert(cbuf);

    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

void circular_buf_free(cbuf_handle_t cbuf) {

	assert(cbuf);
	free(cbuf);
}

bool circular_buf_full(cbuf_handle_t cbuf) {

	assert(cbuf);

    return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf) {
    
    assert(cbuf);

    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

size_t circular_buf_capacity(cbuf_handle_t cbuf) {

	assert(cbuf);

	return cbuf->max;
}

size_t circular_buf_size(cbuf_handle_t cbuf) {

	assert(cbuf);

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

	assert(cbuf);

	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf) {
	
	assert(cbuf);

	cbuf->full = false;
	cbuf->head = (cbuf->head + 1) % cbuf->max;
}

void save_current_head_pointer(cbuf_handle_t cbuf) {

	assert(cbuf);

	cbuf->temp_head = cbuf->head;
}

void update_pointer(cbuf_handle_t cbuf) {

	assert(cbuf);

	cbuf->head = cbuf->temp_head;
}

int circular_buf_put(cbuf_handle_t cbuf, uint8_t data) {

    int r = -1;

    assert(cbuf && cbuf->buffer);

    if(!circular_buf_full(cbuf))
    {
		cbuf->buffer[cbuf->tail] = data;
		advance_pointer(cbuf);
        r = 0;
    }
    return r;
}

int circular_buf_get(cbuf_handle_t cbuf, uint8_t* data) {
    assert(cbuf && data && cbuf->buffer);

    int r = -1;

    if(!circular_buf_empty(cbuf))
    {
        *data = cbuf->buffer[cbuf->head];
        retreat_pointer(cbuf);

        r = 0;
    }
    return r;
}