#ifndef BUFFER_H_
#define BUFFER_H_

typedef struct _buffer {
    unsigned int size;
    char *bytes;
} buffer;

buffer* buffer_new(unsigned int size);
void buffer_free(buffer* b);
void buffer_clear(buffer *b);
int buffer_append(buffer *b, char* data, unsigned int size);

#endif
