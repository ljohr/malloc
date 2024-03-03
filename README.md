# malloc
This project is a simplified implementation of the malloc() function in C. 

### Efficient Free Memory Management:  
Utilizes a singly-linked list to effectively organize and track free memory blocks, ensuring quick allocation and deallocation.

### Dynamic Memory Allocation with sbrk():  
Employs the sbrk() system call to dynamically adjust the program's data segment, enabling precise control over heap expansion and memory allocation.

`my_malloc.c` contains the custom malloc function that can be used to allocate and free memory.
