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
#include "other.cpp"

using namespace std;

// initializing variables

// movie theather is closing
bool is_closing = false; 
bool end_of_work = false;

// number of threads
int no_customers = 8;
int no_employees = 5;
int no_ticket_offices = 2;

// number of products
mutex tickets_mutex, popcorn_mutex, beverage_mutex; 
int no_tickets = (no_customers*3)/2;
int no_popcorn = no_customers;
int no_beverages = no_customers/2;

// customers and their desires
mutex all_products_mutex, some_products_mutex;
mutex some_products_customers_mutex, all_products_customers_mutex; 
int no_some_porducts_customers = no_customers/3;
int no_all_porducts_customers = no_customers/3;

// auxiliary variables
mutex future_mutex, ticket_offices_mutex, served_customers_mutex;
int full_stock_tickets = 0;
int full_stock_popcorn = 0;
int full_stock_beverage = 0;

// main windows size
mutex display_data_mutex;
WINDOW * main_window;
int height = 50;
int width = 100;

struct Customer{
    int id; int desire;
    WINDOW * customer_progress_window; WINDOW * desire_window;

    Customer(int i){
        id = i;
        desire = (i % 3) + 1;

        customer_progress_window = newwin(1, 70, 7+this->getId(), 28);
        desire_window = newwin(1, 20, 6+this->getId()+1, 8);

        refresh();
        wrefresh(customer_progress_window);
    }

    void living(vector<condition_variable> &cv_standings, vector<condition_variable> &cv_customers,
    vector<condition_variable> &cv_employees, vector<condition_variable> &cv_customers_desires,
    vector<SomeProducts> &some_product_customers, vector<Ticket> &tickets, vector<Popcorn> &popcorn, 
    vector<Beverage> &beverages, vector<TicketOffice> &ticket_offices, 
    vector<AllProducts> &all_product_customers, vector<SomeProducts> &some_product_used, 
    vector<AllProducts> &all_product_used, vector<bool> &served_customers)
    {
        while(!is_closing)
        {
            checkingIfPossibleToBuy(ref(some_product_customers), ref(all_product_customers), ref(cv_customers_desires));

            collecting(ref(cv_standings), ref(tickets), ref(popcorn), ref(beverages));

            buying(ref(some_product_customers), ref(all_product_customers), ref(cv_customers_desires), 
            ref(some_product_used), ref(all_product_used), ref(ticket_offices), 
            ref(cv_customers), ref(cv_employees), ref(served_customers));

            desire = rand()%3 + 1;
        }
    }   

    void checkingIfPossibleToBuy(vector<SomeProducts> &some_product_customers, 
    vector<AllProducts> &all_product_customers, vector<condition_variable> &cv_customers_desires)
    {   
        werase(desire_window);

        {
            lock_guard<mutex> lc(display_data_mutex); // lock mutex protecting data to display

            if(desire == 1){ 
                mvwprintw(this->desire_window,0,0,"ticket"); // printing inforamtion about client's desire
            }
            else if(desire == 2){
                mvwprintw(this->desire_window,0,0,"ticket+popcorn"); // printing inforamtion about client's desire
            }
            else{
                mvwprintw(this->desire_window,0,0,"ticket+popcorn+coke"); // printing inforamtion about client's desire
            }
            wrefresh(this->desire_window);
        }
        
        werase(customer_progress_window);

        mvwprintw(this->customer_progress_window,0,0,"|");
        mvwprintw(this->customer_progress_window,0,23,"|");
        mvwprintw(this->customer_progress_window,0,46,"|"); 
        
        if(desire == 3){ // all products = ticket + popcorn + coke
            {
                unique_lock<mutex> lc_all_products(all_products_mutex); // using unique_lock with condition variable
                
                while(all_product_customers.size()==0) // if possibility for buying all products is not present...
                {
                    cv_customers_desires[2].wait(lc_all_products); // wait untill it does (number is going to be incresed when employee is in resting())
                }

                all_product_customers.erase(all_product_customers.begin()); // but if it is decrese number of customers who can benefit
            } 
        }
        else if(desire == 2){ // some products = ticket + popcorn
            {
                unique_lock<mutex> lc_some_products(some_products_mutex); // using unique_lock with condition variable

                while(some_product_customers.size()==0) // if possibility for buying some products is not present...
                {
                    cv_customers_desires[0].wait(lc_some_products); // wait untill it does (number is going to be incresed when employee is in resting())
                }

                some_product_customers.erase(some_product_customers.begin()); // but if it is decrese number of customers who can benefit
            } 
        }

        // print progress bar
        srand (time(nullptr));

        for(int i=0; i<20; i++){
            this_thread::sleep_for (chrono::milliseconds ((1000 + rand()%500)/20));
            {
                lock_guard<mutex> lc(display_data_mutex);
                mvwprintw(this->customer_progress_window,0,i+2,"*");
                wrefresh(this->customer_progress_window);
            }
        }
    }

    void collecting(vector<condition_variable> &cv_standings,
    vector<Ticket> &tickets, vector<Popcorn> &popcorn, vector<Beverage> &beverages)
    {
        werase(customer_progress_window);

        mvwprintw(this->customer_progress_window,0,0,"|");
        mvwprintw(this->customer_progress_window,0,23,"|");
        mvwprintw(this->customer_progress_window,0,46,"|");

        srand (time(nullptr));

        if(desire==3){ // ticket + popcorn + coke
            {
                unique_lock<mutex> lc_tickets(tickets_mutex); // using unique_lock with condition variable

                while(tickets.size()==0) // if there is no more tickets...
                {
                    cv_standings[0].wait(lc_tickets); // wait untill tickets are avaliable (number is going to be increased when employee is in standingService())
                }

                tickets.erase(tickets.begin()); // but if they are decrese number of tickets

                // print progress bar
                for(int i=0; i<6; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
                    {
                        lock_guard<mutex> lock_1(display_data_mutex);
                        mvwprintw(this->customer_progress_window,0,i+25,"*");
                        wrefresh(this->customer_progress_window);
                    }
                }
            }   

            {
                unique_lock<mutex> lc_popcorn(popcorn_mutex); // using unique_lock with condition variable

                while(popcorn.size()==0) // if there is no more popcorn...
                {
                    cv_standings[1].wait(lc_popcorn); // wait untill popcorn is avaliable (number is going to be increased when employee is in standingService())
                }

                popcorn.erase(popcorn.begin()); // but if it is decrese number of popcorn

                // print progress bar
                for(int i=6; i<13; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
                    {
                        lock_guard<mutex> lock_2(display_data_mutex);
                        mvwprintw(this->customer_progress_window,0,i+25,"*");
                        wrefresh(this->customer_progress_window);
                    }
                }
            }

            {
                unique_lock<mutex> lc_beverage(beverage_mutex); // using unique_lock with condition variable

                while(beverages.size()==0) // if there is no more beverages...
                {
                    cv_standings[2].wait(lc_beverage); // wait untill beverages are avaliable (number is going to be increased when employee is in standingService())
                }

                beverages.erase(beverages.begin()); // but if they are decrese number of beverages

                // print progress bar
                for(int i=13; i<20; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
                    {
                        lock_guard<mutex> lock_3(display_data_mutex);
                        mvwprintw(this->customer_progress_window,0,i+25,"*");
                        wrefresh(this->customer_progress_window);
                    }
                }
            }
        }
        else if(desire==2){ // ticket + popcorn
            {
                unique_lock<mutex> lc_tickets(tickets_mutex); // using unique_lock with condition variable

                while(tickets.size()==0) // if there is no more tickets...
                {
                    cv_standings[0].wait(lc_tickets); // wait untill tickets are avaliable (number is going to be increased when employee is in standingService())
                }

                tickets.erase(tickets.begin()); // but if they are decrese number of tickets

                // print progress bar
                for(int i=0; i<10; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
                    {
                        lock_guard<mutex> lock_1(display_data_mutex);
                        mvwprintw(this->customer_progress_window,0,i+25,"*");
                        wrefresh(this->customer_progress_window);
                    }
                }
            }

            {
                unique_lock<mutex> lc_popcorn(popcorn_mutex); // using unique_lock with condition variable

                while(popcorn.size()==0) // if there is no more popcorn...
                {
                    cv_standings[1].wait(lc_popcorn); // wait untill popcorn is avaliable (number is going to be increased when employee is in standingService())
                }

                popcorn.erase(popcorn.begin()); // but if it is decrese number of popcorn

                // print progress bar
                for(int i=10; i<20; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
                    {
                        lock_guard<mutex> lock_2(display_data_mutex);
                        mvwprintw(this->customer_progress_window,0,i+25,"*");
                        wrefresh(this->customer_progress_window);
                    }
                }
            }
        }
        else if(desire==1){ // ticket
            {
                unique_lock<mutex> lc_tickets(tickets_mutex); // using unique_lock with condition variable

                while(tickets.size()==0) // if there is no more tickets...
                {
                    cv_standings[0].wait(lc_tickets); // wait untill tickets are avaliable (number is going to be increased when employee is in standingService())
                }

                tickets.erase(tickets.begin()); // but if they are decrese number of tickets

                // print progress bar
                for(int i=0; i<20; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
                    {
                        lock_guard<mutex> lock_1(display_data_mutex);
                        mvwprintw(this->customer_progress_window,0,i+25,"*");
                        wrefresh(this->customer_progress_window);
                    }
                }
            }
        }
    }

    void buying(vector<SomeProducts> &some_product_customers, vector<AllProducts> &all_product_customers,
    vector<condition_variable> &cv_customers_desires, vector<SomeProducts> &some_product_used,
    vector<AllProducts> &all_product_used, vector<TicketOffice> &ticket_offices,
    vector<condition_variable> &cv_customers, vector<condition_variable> &cv_employees, vector<bool> &served_customers)
    {
        {
            lock_guard<mutex> lc_served(served_customers_mutex); // locking mutex
            served_customers[id] = false; // setting that customer hasn't yet been served
        }

        werase(customer_progress_window);

        mvwprintw(this->customer_progress_window,0,0,"|");
        mvwprintw(this->customer_progress_window,0,23,"|");
        mvwprintw(this->customer_progress_window,0,46,"|");

        int ticket_office_number = id%2; // establishing to which ticket office customer go

        {
            lock_guard<mutex> lc_ticket_office(ticket_offices_mutex); // locking mutex
            ticket_offices[ticket_office_number].customer_id.push_back(id); // adding customer to ticket's office queue
        }

        {
            unique_lock<mutex> lc_ticket_office(ticket_offices_mutex); // using unique_lock with condition variable

            while(std::count(ticket_offices[ticket_office_number].customer_id.begin(),
            ticket_offices[ticket_office_number].customer_id.end(), id)) // until there is a given client in queue...
            {
                cv_customers[id].wait(lc_ticket_office); // wait till he is going to be served (customer is going to be taken down form vector in employee handleTicketOffice())
            }
        }

        srand (time(nullptr));

        // print progress bar
        for(int i=0; i<20; i++){
            this_thread::sleep_for (chrono::milliseconds ((1000*desire + rand()%500)/20));
            {
                lock_guard<mutex> lc(display_data_mutex);
                mvwprintw(this->customer_progress_window,0,i+48,"*");
                wrefresh(this->customer_progress_window);
            }
        }

        {
            lock_guard<mutex> lc_served(served_customers_mutex); // locking mutex
            served_customers[id] = true;  // setting that customer has been served
        }
        
        cv_employees[ticket_offices[ticket_office_number].employee_id].notify_one(); // notifing employee that customer has ben served

        if(desire == 3)
        {
            {
                lock_guard<mutex> lc(all_products_customers_mutex); // locking mutex
                all_product_used.push_back(AllProducts()); // auxiliary operation 
            }
        }
        else if(desire == 2)
        {
            {
                lock_guard<mutex> lc(some_products_customers_mutex); // locking mutex
                some_product_used.push_back(SomeProducts()); // auxiliary operation 
            }
        }

        werase(desire_window);
    }

    int getId() {return id;}
};

struct Employee{
    int id; int product;
    WINDOW * employee_progress_window;

    Employee(int i){
        id = i;

        employee_progress_window = newwin(1, 100, 19+this->getId(), 7);

        refresh();
        wrefresh(employee_progress_window);
    }

    void working(vector<condition_variable> &cv_standings, vector<condition_variable> &cv_customers,
    vector<condition_variable> &cv_employees, vector<condition_variable> &cv_customers_desires,
    vector<SomeProducts> &some_product_customers, vector<Ticket> &tickets, vector<Popcorn> &popcorn, 
    vector<Beverage> &beverages, vector<TicketOffice> &ticket_offices, 
    vector<AllProducts> &all_product_customers, vector<SomeProducts> &some_product_used,
    vector<AllProducts> &all_product_used, vector<bool> &served_customers)
    {
        while(!end_of_work)
        {
            standingService(ref(cv_standings),ref(tickets), ref(popcorn), ref(beverages));

            handleTicketOffice(ref(cv_customers), ref(cv_employees), ref(ticket_offices),
            ref(served_customers));

            resting(ref(cv_customers_desires), ref(some_product_customers), ref(all_product_customers),
            ref(some_product_used), ref(all_product_used));
        }
    }
    
    void standingService(vector<condition_variable> &cv_standings, vector<Ticket> &tickets, 
    vector<Popcorn> &popcorn, vector<Beverage> &beverages)
    {
        {
            // employee can add products to standings but only one type at a time
            
            lock_guard<mutex> lc_tic(tickets_mutex); // locking mutextes
            lock_guard<mutex> lc_pop(popcorn_mutex);
            lock_guard<mutex> lc_bev(beverage_mutex);

            // deciding which product will be added based on it's quantity
            if(tickets.size() < popcorn.size() && tickets.size() < beverages.size())
            {
                product = 1;
            }
            else if(popcorn.size() < tickets.size() && popcorn.size() < beverages.size())
            {
                product = 2; 
            }
            else
            {
                product = 3;
            }
        }

        if(product==1) // tickets
        {
            {
                lock_guard<mutex> lc_tickets(tickets_mutex); // locking mutex

                werase(employee_progress_window);

                mvwprintw(this->employee_progress_window,0,22,"|");
                mvwprintw(this->employee_progress_window,0,45,"|");

                srand (time(nullptr));
        
                // printing progress status
                for(int i=0; i<20; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000 + rand()%500)/20));
                    {
                        lock_guard<mutex> lc(display_data_mutex);
                        mvwprintw(this->employee_progress_window,0,i+1,"*");
                        wrefresh(this->employee_progress_window);
                    }
                }

                // untill standing with tickets isn't full, add tickets
                while(tickets.size()<no_tickets){
                    tickets.push_back(Ticket());
                }
            }
            cv_standings[0].notify_one(); // unblock thread currently waiting for this cv
        }
        else if(product==2) // popcorn
        {
            {
                lock_guard<mutex> lc_popcorn(popcorn_mutex); // locking mutex

                werase(employee_progress_window);

                mvwprintw(this->employee_progress_window,0,22,"|");
                mvwprintw(this->employee_progress_window,0,45,"|");

                srand (time(nullptr));

                // printing progress status
                for(int i=0; i<20; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000 + rand()%500)/20));
                    {
                        lock_guard<mutex> lc(display_data_mutex);
                        mvwprintw(this->employee_progress_window,0,i+1,"*");
                        wrefresh(this->employee_progress_window);
                    }
                }

                // untill standing with popcorn isn't full, add popcorn
                while(popcorn.size()<no_popcorn){
                    popcorn.push_back(Popcorn());
                }
            }
            cv_standings[1].notify_one(); // unblock thread currently waiting for this cv
        }
        else if(product==3) // beverage
        {
            {
                lock_guard<mutex> lc_beverage(beverage_mutex); // locking mutex

                werase(employee_progress_window);

                mvwprintw(this->employee_progress_window,0,22,"|");
                mvwprintw(this->employee_progress_window,0,45,"|");

                srand (time(nullptr));

                // printing progress status
                for(int i=0; i<20; i++){
                    this_thread::sleep_for (chrono::milliseconds ((1000 + rand()%500)/20));
                    {
                        lock_guard<mutex> lc(display_data_mutex);
                        mvwprintw(this->employee_progress_window,0,i+1,"*");
                        wrefresh(this->employee_progress_window);
                    }
                }

                // untill standing with beverages isn't full, add beverages
                while(beverages.size()<no_beverages){
                    beverages.push_back(Beverage());
                }
            }
            cv_standings[2].notify_one(); // unblock thread currently waiting for this cv
        }
    }
    
    void handleTicketOffice(vector<condition_variable> &cv_customers,
    vector<condition_variable> &cv_employees, vector<TicketOffice> &ticket_offices,
    vector<bool> &served_customers)
    {
        werase(employee_progress_window);

        mvwprintw(this->employee_progress_window,0,22,"|");
        mvwprintw(this->employee_progress_window,0,45,"|");
        
        {
            lock_guard<mutex> lc(display_data_mutex); // locking mutex
            wrefresh(this->employee_progress_window);
        }

        int ticket_office_number = id%(no_employees/2); // assigning employee to ticket office
        
        {
            lock_guard<mutex> lc_ticket_office(ticket_offices_mutex); // locking mutex

            if(!ticket_offices[ticket_office_number].served) // if ticket office isn't handled by employee... 
            {
                ticket_offices[ticket_office_number].served = true; // then it is handled...
                ticket_offices[ticket_office_number].employee_id = id; // by employee of given id
            }
            else
            {
                return;
            }
        }

        bool is_queue_empty; 
        
        {
            lock_guard<mutex> lc(ticket_offices_mutex);
            is_queue_empty = ticket_offices[ticket_office_number].customer_id.empty(); 
        }
        
        while(!is_queue_empty) // checking if queue to ticket office is not empty
        {
            int customer_id_tmp;
            {
                lock_guard<mutex> lc_ticket_office(ticket_offices_mutex); // locking mutex
                customer_id_tmp = ticket_offices[ticket_office_number].customer_id.front(); // fetching first customer in queue
                ticket_offices[ticket_office_number].customer_id.erase(ticket_offices[ticket_office_number].customer_id.begin()); // serving him (done by ereasng him from queue)
            }
            
            {
                lock_guard<mutex> lc(display_data_mutex);

                // printing information in console
                mvwprintw(this->employee_progress_window,0,24,"SERVING CUSTOMER #%1d", customer_id_tmp+1);
                wrefresh(this->employee_progress_window);

                werase(employee_progress_window);

                mvwprintw(this->employee_progress_window,0,22,"|");
                mvwprintw(this->employee_progress_window,0,45,"|");
            }
            cv_customers[customer_id_tmp].notify_one(); // unblocking customer thread currently waiting for this cv

            {
                unique_lock<mutex> lc_served(served_customers_mutex);
                while(!served_customers[customer_id_tmp]) // while customer hasn't been served...
                {
                    cv_employees[id].wait(lc_served); // block employee thread till it's notified that given customer has been served
                }
            }

            {
                lock_guard<mutex> lc_ticket_office(ticket_offices_mutex);
                is_queue_empty = ticket_offices[ticket_office_number].customer_id.empty(); // updating queue to ticket office
            }
        }

        {
            lock_guard<mutex> lc_ticket_office(ticket_offices_mutex);
            ticket_offices[ticket_office_number].served = false; // queue is empty so employee is leaving ticket office
        }
    }
    
    void resting(vector<condition_variable> &cv_customers_desires, 
    vector<SomeProducts> &some_product_customers, vector<AllProducts> &all_product_customers,
    vector<SomeProducts> &some_product_used, vector<AllProducts> &all_product_used)
    {
        int option = 1;
        int count = 0;
        {
            lock_guard<mutex> lc_spc(some_products_customers_mutex); // locking mutexes
            lock_guard<mutex> lc_apc(all_products_customers_mutex);

            // deciding which avalibility will be increased
            if(some_product_used.size() > all_product_used.size())
            {
                option = 0;
                count = some_product_used.size();
                some_product_used.clear();
            }
            else
            {
                count = all_product_used.size();
                all_product_used.clear();
            }
        }

        werase(employee_progress_window);

        mvwprintw(this->employee_progress_window,0,22,"|");
        mvwprintw(this->employee_progress_window,0,45,"|");
        
        srand (time(nullptr));

        // printing progress status
        for(int i=0; i<20; i++){
            this_thread::sleep_for (chrono::milliseconds ((1000 + rand()%500)/20));
            {
                lock_guard<mutex> lc(display_data_mutex);
                mvwprintw(this->employee_progress_window,0,i+47,"*");
                wrefresh(this->employee_progress_window);
            }
        }

        if(option==1)
        {
            // increasing avaliability for all products desire
            {
                lock_guard<mutex> lc(all_products_mutex);

                int i = 0;

                while(i<count)
                {
                    all_product_customers.push_back(AllProducts());
                    i++;
                }
            }
            cv_customers_desires[2].notify_all(); // unblocking all threads waiting for cv
        }
        else
        {
            // increasing avaliability for some products desires
            {
                lock_guard<mutex> lc(some_products_mutex);

                int i = 0;

                while(i<count)
                {
                    some_product_customers.push_back(SomeProducts());
                    i++;
                }   
            }
            cv_customers_desires[0].notify_all(); // unblocking all threads waiting for cv
        }
    }

    int getId() {return id;}
};

void ticketOfficesStatus(vector<TicketOffice> &ticket_offices, int no_ticket_offices)
{
    // while employees are still working... 
    while(!end_of_work)
    {
        {
            lock_guard<mutex> lc(display_data_mutex); // lock mutex protecting data to display

            for(int i=0; i<no_ticket_offices; i++) // for every ticket office...
            {
                if(ticket_offices[i].served) // check if employee handles the ticket office 
                {
                    mvwprintw(main_window, 24+i, 28, "Ticket office #%1d: Opened |", i); // if he is than print that ticket office is open
                }
                else
                {
                    mvwprintw(main_window, 24+i, 28, "Ticket office #%1d: Closed |", i); // if he isn't print that ticket office is closed
                }
            }
            wrefresh(main_window);

        } // unlock mutex protecting data to display
    }
}

void ticketOfficesQueues(vector<bool> &served_customers, vector<TicketOffice> &ticket_offices)
{
    // while employees are still working... 
    while(!end_of_work)
    {
        {
            lock_guard<mutex> lc(display_data_mutex); // lock mutex protecting data to display
            {
                lock_guard<mutex> lc_ticket_offices(ticket_offices_mutex); // lock mutex protecting ticket offices' data

                int tmp = 0;

                for(TicketOffice &tic_office :ticket_offices) // for every ticket office...
                {
                    mvwprintw(main_window, 24 + tmp, 55, "Ticket office #%d queue:           ", tmp);

                    for(int i=0; i<tic_office.customer_id.size(); i++)
                    {
                        mvwprintw(main_window, 24 + tmp, 80+2*i, "%d,", tic_office.customer_id[i]+1); // print customer's id for those who are waiting in queue for that ticket office
                    }
                    tmp++;
                }

            } // unlock mutex protecting ticket offices' data

            wrefresh(main_window);

        } // unlock mutex protecting data to display
    }
}

void endProgram()
{
    {
        lock_guard<mutex> lc(display_data_mutex); // lock mutex protecting data to display

        for (int i = 0; i < width; i++)
        {
            mvwprintw(main_window,27,i, "-");
        }

        mvwprintw(main_window,28,width/2 - 6,"Press [q] to exit"); // print text
        wrefresh(main_window);

    } // unlock mutex protecting data to display

    do{
        if(getch()==113) // check for [q]
        {
            { // lock mutex protecting data to display

                lock_guard<mutex> lc(display_data_mutex);

                mvwprintw(main_window,28,width/2 - 14,"Movie theater is closing. Please wait..."); // print text
                wrefresh(main_window);
            
            } // unlock mutex protecting data to display
            
            is_closing = true; // since now movie theater is closing
            break;
        }
    }while(true);
}

int main()
{
    // setting up the ncurses
    initscr();
    noecho();

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    // declaring variables for customer
    vector<Customer> customers;
    vector<thread> customer_threads;
    vector<bool> served_customers_vector(no_customers);
    vector<condition_variable> cv_customers(no_customers);
    
    // declaring variables for employee
    vector<Employee> employees;
    vector<thread> threads;
    vector<condition_variable> cv_employees(no_employees);

    // declaring variables for standings
    vector<TicketOffice> ticket_offices(no_ticket_offices);
    vector<condition_variable> cv_standings(3);
    
    // declaring variables for products
    vector<Ticket> tickets(no_tickets);
    vector<Popcorn> popcorn(no_popcorn);
    vector<Beverage> beverages(no_beverages);

    // declaring variables for customer's desires
    vector<SomeProducts> some_products_vector(no_some_porducts_customers);
    vector<SomeProducts> some_products_customers;
    vector<AllProducts> all_products_vector(no_all_porducts_customers);
    vector<AllProducts> all_products_customers;
    vector<condition_variable> cv_desires(4);

    // printing not changing information in console        
    mvprintw(1, width/2 - 3, "MOVIE THEATER");

    for (int i = 0; i < width; i++)
    {
        mvprintw(2,i, "-");
    }

    main_window = newwin(height, width, 3, 1);
    refresh();
    
    // customers' section   
    mvwprintw(main_window, 1, width/2-4, "  CUSTOMERS");
    mvwprintw(main_window, 2, width/2-4, "-------------");

    wattron(main_window, COLOR_PAIR(1));
    mvwprintw(main_window, 3, 1, " Nr | Desire              | Thinking             | Collecting           | Buying               ");
    wattroff(main_window, COLOR_PAIR(1));

    // employees' section
    mvwprintw(main_window, 13, width/2-4, "  EMPLOYEES");
    mvwprintw(main_window, 14, width/2-4, "-------------");

    wattron(main_window, COLOR_PAIR(1));
    mvwprintw(main_window, 15, 1, " Nr | Standing service     | Checkout service     | Break time           ");
    wattroff(main_window, COLOR_PAIR(1));

    // ticket offices' section
    mvwprintw(main_window, 22, width/2-6, "  TICKET OFFICES");
    mvwprintw(main_window, 23, width/2-6, "------------------");

    wrefresh(main_window);

    // creating customers
    for(int i = 0; i < no_customers; i++)
    {
        customers.push_back(Customer(i));
        mvwprintw(main_window, 4+i, 2, "%d  |", i+1);
        served_customers_vector[i]=true;
    }

    // creating employees
    for(int i = 0; i < no_employees; i++)
    {
        employees.push_back(Employee(i));
        mvwprintw(main_window, 16+i, 2, "%d  |", i+1);
    }
    wrefresh(main_window);

    // lauching threads for customers
    for(int i = 0; i < no_customers; i++)
    {
        customer_threads.push_back(thread(&Customer::living, &customers[i], ref(cv_standings), ref(cv_customers),
        ref(cv_employees), ref(cv_desires), ref(some_products_vector), ref(tickets), ref(popcorn), ref(beverages),
        ref(ticket_offices), ref(all_products_vector), ref(some_products_customers), ref(all_products_customers),
        ref(served_customers_vector)));
    }

    // lauching threads for employees
    for(int i = 0; i < no_employees; i++)
    {
        threads.push_back(thread(&Employee::working, &employees[i], ref(cv_standings), ref(cv_customers),
        ref(cv_employees), ref(cv_desires), ref(some_products_vector), ref(tickets), ref(popcorn), ref(beverages),
        ref(ticket_offices), ref(all_products_vector), ref(some_products_customers), ref(all_products_customers),
        ref(served_customers_vector)));
    }

    // lauching threads for others
    threads.push_back(thread(ticketOfficesStatus, ref(ticket_offices), ref(no_ticket_offices)));
    threads.push_back(thread(ticketOfficesQueues, ref(served_customers_vector),ref(ticket_offices)));
    threads.push_back(thread(endProgram));

    // joining customers threads
    for(thread &thrc : customer_threads)
    {
        thrc.join();
    }

    end_of_work = true;

    // joining employees and others threads
    for(thread &thr : threads)
    {
        thr.join();
    }

    // end of program
   	endwin();
    return 0;	
}
