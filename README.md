# threads_when_needed
A set of programs capable of using as much threads as needed, to perform some tasks.
But what "as needed" means, techically? Simply put, main thread handles requests from users and it wakes as much threads he needs to carry out those requests.

Each of this programs handles a different task (you can guess which from their names), the surrounding logic is pretty much the same for each one of them, what differs is the data structures used and how to access them. 
