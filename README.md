# Introduction
This is a program capable of using as much threads as needed, to perform linked list operations.
But what "as needed" means, techically? Simply put, main thread handles requests from users and it wakes as much threads he needs to carry out those requests.

I made this program as a university project, in particular for the "Multicore Parallel Programming" course.

## Technologies

* C Programming Language
* Pthread Library

## Setup

To run this program on Linux, compile and run it like this:


```
gcc -g -Wall linked_lists_tasks.c -lpthread
./a.out x  (where x is the number of threads you intend to use).
```

## What I Learned

I learned how to work in a multithread environment using the Pthread library, this means handling critical sections, thread's communications, barriers and more.

Furthermore, I learned how to effectively use Queues and Linked Lists in C programming language.
