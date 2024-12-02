**Compile the Program:**

**Use the GCC compiler to compile the program. Run the following command in your terminal:**


gcc -o rip_router rip_router.c

This will create an executable file named rip_router.


**Run the Program:**
The program requires at least one router ID and its neighbors as arguments. Run the program using the following format:

./rip_router <router_id> <neighbor_router_ids>...


**Example:**
If you want to simulate Router 1 with neighbors Router 2 and Router 3, run:

./rip_router 1 2 3

**Note:**

Each router runs as a separate instance. Open multiple terminal windows, and run the program for each router with its respective router ID and neighbors.


**For Router 1:**

./rip_router 1 2 3


**For Router 2:**

./rip_router 2 1 3


**For Router 3:**

./rip_router 3 1 2
