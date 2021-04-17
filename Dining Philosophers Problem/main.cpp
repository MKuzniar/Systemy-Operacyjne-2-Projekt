#include <thread>
#include <mutex>
#include <chrono>
#include <unistd.h>
#include <ncurses.h>

#include "philosopher.cpp"

#define MAX_NUMBER_OF_PHILOSOPHERS 30

int number_of_philosophers = 5; //number of philosophers and chopsticks

bool meal_time = true;
bool show_on_screen = true;
bool exit_program = false;

float eating_time = 3;
float thinking_time = 3;

Philosopher philosophers[MAX_NUMBER_OF_PHILOSOPHERS];
Chopstick chopsticks[MAX_NUMBER_OF_PHILOSOPHERS];

void feast(Philosopher* philosopher){ //actions of philosophers during feast
    while(meal_time){
    philosopher -> think(thinking_time);
    philosopher -> eat(eating_time, meal_time);
    };
};

void showOnScreen() { //function resposible for showing results in console
    noecho();

    while(show_on_screen) {
        
        mvprintw(4, 0, "-------------------------------------------------------------------------------------------------------------------");

        for(int i = 0; i < number_of_philosophers; i++) {

            mvprintw(i + 5, 1, "Philosopher %c is", philosophers[i%number_of_philosophers].getPhilosopherName()); //information about philosopher's state
            mvprintw(i + 5, 18, philosophers[i%number_of_philosophers].getPhilosopherStatus().c_str());

            mvprintw(i + 5, 27, "| | Bowls of rice eaten by Philosopher %c: %d", philosophers[i%number_of_philosophers].getPhilosopherName(), philosophers[i%number_of_philosophers].getRiceEaten()); //information about philosopher's meals

            mvprintw(i + 5, 72, "| | Chopstick %d is", ((i % number_of_philosophers) + 1)); //information about chostick's state
            mvprintw(i + 5, 92, chopsticks[i%number_of_philosophers].getChopstickStatus().c_str());

            if (chopsticks[i%number_of_philosophers].getChopstickStatus() != "free ")
            {
                mvprintw(i + 5, 98, "by Philosopher %c", chopsticks[i%number_of_philosophers].getChopstickUser());
            }
            else
            {
                mvprintw(i + 5, 97, "                         ");
            }
        }

        mvprintw(number_of_philosophers + 5, 0, "-------------------------------------------------------------------------------------------------------------------");
        mvprintw(number_of_philosophers + 6, 50, " Press [q] to exit");

        refresh();
        usleep(50000);
    }
}

int main(){ //main function
    
    srand(time(NULL));

    initscr();
    refresh();

    mvprintw(0, 0, "-------------------------------------------------------------------------------------------------------------------");
    mvprintw(1, 47, "DINING PHILOSOPHERS PROBLEM");
    mvprintw(2, 0, "-------------------------------------------------------------------------------------------------------------------");
    mvprintw(3, 11, "Eating time [sek]: ");
    scanw("%f", &eating_time);
    mvprintw(3, 41, "Thinking time [sek]: ");
    scanw("%f", &thinking_time);
    mvprintw(3, 73, "Odd number of Philosophers: ");
    scanw("%d", &number_of_philosophers);

    getch();

    std::thread threads[MAX_NUMBER_OF_PHILOSOPHERS];
   
    for(int i = 0; i < number_of_philosophers-1; i++) { //setup - chopsticks assignment
        philosophers[i].setPhilosopherName((char)i + 65);
        philosophers[i].left_chopstick = &chopsticks[i];
        philosophers[i].right_chopstick = &chopsticks[i+1];
    }
    philosophers[number_of_philosophers - 1].setPhilosopherName((char)number_of_philosophers + 64);
    philosophers[number_of_philosophers - 1].left_chopstick = &chopsticks[0];
    philosophers[number_of_philosophers - 1].right_chopstick = &chopsticks[number_of_philosophers - 1];

    for(int i = 0; i < number_of_philosophers; i++) { //launching threads
        threads[i] = std::thread(feast, &philosophers[i]);
    }
    std::thread displayThread(showOnScreen);
    
    while (!exit_program) //exiting program when 'q' is pressed
    {
        if (getch() == 113)
        {
            exit_program = true;
        }
    }
    meal_time = false;
    show_on_screen = false;

    for(int i = 0 ; i < number_of_philosophers; i ++) { //waiting for threads to finish
        threads[i].join();
    }
    displayThread.join();

    endwin();
    return 0;
}