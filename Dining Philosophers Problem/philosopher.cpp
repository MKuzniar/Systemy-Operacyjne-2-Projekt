#include <mutex>
#include <chrono>
#include <unistd.h>

#include "chopstick.cpp"

class Philosopher{ //Philosopher class
    std::string philosopher_status; 
    char philosopher_name;
    
public:
    Chopstick* right_chopstick;
    Chopstick* left_chopstick;
    int rice_eaten = 0;
   
    Philosopher() {}
    Philosopher(char philosopher_name, Chopstick* right_chopstick, Chopstick* left_chopstick): philosopher_name(philosopher_name), right_chopstick(right_chopstick), left_chopstick(left_chopstick){}
        
    int getRiceEaten(){ //information about how much rice philosopher ate
        return rice_eaten;
    }
    
    std::string getPhilosopherStatus(){ //information about philosopher's status - thinking, hungry, eating
        return philosopher_status;
    }

    void setPhilosopherName(char philosopher_name){ //setting philosopher's name (unique)
        this->philosopher_name = philosopher_name;
    }

    char getPhilosopherName(){ //information about philosopher's name
        return philosopher_name;
    }

    void eat(float eating_time, bool meal_time){ //'eat' function
        if(meal_time)
        {
            right_chopstick->pickUpChopstick(philosopher_name); //philosopher is picking up chopsticks
            left_chopstick->pickUpChopstick(philosopher_name);
            philosopher_status = "eating  ";
            rice_eaten++;
        
            usleep((((8 + (std::rand()%(12-8+1)))* eating_time)/10)*1000000); //eating time +- 20%
        
            right_chopstick->putDownChopstick(); //philosopher is putting down chopsticks
            left_chopstick->putDownChopstick();
        }
    };
    
    void think(float thinking_time){ //'think' function
        philosopher_status = "thinking";

        usleep((((8 + (std::rand()%(12-8+1))) * thinking_time)/10)*1000000); // thinking time +- 20%

        philosopher_status = "hungry  ";
    }
};