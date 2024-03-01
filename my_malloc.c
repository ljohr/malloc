#include <unistd.h>

typedef struct freenode
{
    size_t size;
    struct freenode *next;
} freenode;

#define HEAP_CHUNK_SIZE 4096

// head node for the freelist
freenode *freelist = NULL;

/* allocate size bytes from the heap */
void *malloc(size_t size)
{
    // can't have less than 1 byte requested
    if (size < 1)
    {
        return NULL;
    }

    // add 8 bytes for bookkeeping
    size += 8;

    // 32 bytes is the minimum allocation
    if (size < 32)
    {
        size = 32;
    }

    // round up to the nearest 16-byte multiple
    else if (size%16 != 0)
    {
        size = ((size/16)+1)*16;
    }

    freenode *temp = freelist;
    freenode *prev_node = NULL;

    // if we have no memory, grab one chunk to start
    if(freelist == NULL)
    {
        void *tmp_size;
        tmp_size = sbrk(HEAP_CHUNK_SIZE);
        if(tmp_size == (void *)-1)
        {
            return NULL;
        }

        // skip first 8-bytes so addresses are 16-byte aligned
        freelist = (freenode *)(tmp_size + 8);
        freelist->size = HEAP_CHUNK_SIZE - 8;
        freelist->next = NULL;
    }

    // look for a freenode that's large enough for this request
    // have to track the previous node also for list manipulation later
    while(temp != NULL && temp->size < size)
    {
        prev_node = temp;
        temp = temp->next;
    }

    // if there is no freenode that's large enough, allocate more memory
    if (temp == NULL)
    {
        int total_new_bytes = ((size/HEAP_CHUNK_SIZE)+1)*HEAP_CHUNK_SIZE;
        void *add_size = sbrk(total_new_bytes);

        if(add_size == (void *)-1)
        {
            return NULL;
        }

        size_t *addr_temp = (size_t *)freelist;
        freelist = (freenode *)(add_size + 8);
        freelist->size = ((size/HEAP_CHUNK_SIZE)+1)*HEAP_CHUNK_SIZE - 8;
        freelist->next = (freenode *)addr_temp;

        temp = freelist;
    }

    // here, should have a freenode with enough space for the request
    // - if there would be less than 32 bytes left, then return the entire chunk
    void *user_ptr;

    // check if size is less than 32
    // if so, update list, and return address
    if ((temp->size) - size < 32)
    {
        if(temp == freelist)
        {
            freelist = temp->next; 
        }
        else
        {
            prev_node->next = temp->next;
        }
        
        user_ptr = (void *)temp;
        return user_ptr;
    }

    // - if there are remaining bytes, then break up the chunk into two pieces 
    freenode *split_node = (freenode *)(((void *)temp) + size);
    split_node->size = (temp->size) - size;
    split_node->next = temp->next;

    // add partitioned chunk to the linked list
    if(temp == freelist)
    {
        freelist = split_node;
    }
    else
    {
        prev_node->next = split_node;
    }
       
    temp->size = size; 
    temp->next = NULL; 
    
    // here, get the address for the chunk being returned to the user and return it
    user_ptr = ((void *)temp) + 8; 
    return user_ptr;
}


/* return a previously allocated memory chunk to the allocator */
void free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    // make a new freenode starting 8 bytes before the provided address
    freenode *new_node = (freenode *)(ptr-8);

    // the size is already in memory at the right location (ptr-8)

    // add this memory chunk back to the beginning of the freelist
    new_node->next = freelist;
    freelist = new_node;

    return;
}

