#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ncurses.h>

using namespace std;

// TICKET
struct Ticket{
    Ticket(){};
    int id;
};

// POPCORN
struct Popcorn{
    Popcorn(){};
    int id;
};

// BEVERAGE
struct Beverage{
    Beverage(){};
    int id;
};

// TICKET OFFICE
struct TicketOffice{
    TicketOffice(){};
    vector <int> customer_id; // customer's id that is in queue
    int id;
    bool served = false; // is employee handling the ticket office?
    int employee_id; // employee id
};

// STANDING
struct Standing{
    Standing(){};
    int id;
    bool isReady = true;
};

// ALL PRODUCTS
struct AllProducts{
    AllProducts(){};
    int id;
};

// SOME PRODUCTS
struct SomeProducts{
    SomeProducts(){};
    int id;
};