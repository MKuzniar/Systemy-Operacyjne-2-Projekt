#include <mutex>

class Chopstick{ //Chopstick class
    std::mutex chopstick_mutex;
    std::string chopstick_status;
    char chopstick_user;

    public:

    Chopstick(){
        chopstick_status = "free ";
    }

    std::string getChopstickStatus(){ //information about chopstick's status - free or taken
        return chopstick_status;
    };

    char getChopstickUser(){
        return chopstick_user;
    }

    void pickUpChopstick(char chopstick_user){
        chopstick_mutex.lock(); //locking chopstick from other philosophers/threads
        chopstick_status = "taken";
        this->chopstick_user = chopstick_user;
    };

    void putDownChopstick(){
        chopstick_mutex.unlock(); //unlocking chopstick for other philosopher/threads to get access to 
        chopstick_status = "free ";
    };
};