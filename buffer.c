#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/**
 * Returns a new buffer with "size" bytes
 */
buffer* buffer_new(unsigned int size)
{
    buffer *b;
    
    b = (buffer *)malloc(sizeof(buffer));
    
    if(b == NULL) {
        return NULL;
    }

    b->size = size;
    b->bytes = NULL;
    
    if(size > 0) {
        
        b->bytes = (char *)malloc(sizeof(char) * size);
        
        if(b->bytes == NULL) {
            free(b);
            return NULL;
        }
        
        buffer_clear(b);

    }
    
    return b;
    
}

/**
 * Frees the buffer
 **/
void buffer_free(buffer* b)
{
    if(b->bytes && b->size > 0)
        free(b->bytes);
    
    if(b)
        free(b);
}

/**
 * Clears the buffer
 */
void buffer_clear(buffer *b)
{
    if(b) {
        if(b->bytes && (b->size > 0)) {
            b->size = 0;
            free(b->bytes);
        }
    }
}

/**
 * Appends 'size' bytes from 'data' to 'b'
 * Reallocs buffer in case its size is not enough to handle all data
 * 
 * Returns the number of bytes appended or -1 in case of error
 */
int buffer_append(buffer *b, char* data, unsigned int size)
{
    if(!b)
        return -1;
    
    if(!data)
        return -1;
    
    if(size == 0)
        return 0;
        
    
    b->bytes = (char *)realloc(b->bytes, b->size + size);
    memcpy(&b->bytes[b->size], data, size);
    
    b->size += size;
    
    return size;
}
