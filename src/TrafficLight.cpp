#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    int waitGreenCycleDuration = 1;
    std::chrono::time_point<std::chrono::system_clock> start;

    start = std::chrono::system_clock::now();
    while (true) 
    {
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();

        long currentDuration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count();
        if (currentDuration < waitGreenCycleDuration) 
        {
            continue;
        }
        if (_messageQueue.receive() == TrafficLightPhase::green)
        {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method 
    // „simulate“ is called. To do this, use the thread queue in the base class.

    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

int TrafficLight::getRandomCycleDuration()
{
    // https://stackoverflow.com/a/13445752
    std::random_device dev;  
    std::mt19937 rng(dev()); 
    std::uniform_int_distribution<std::mt19937::result_type> distribution(4000, 6000); 
    int cycleDuration = distribution(rng); 
    return cycleDuration;
}

void TrafficLight::toggleCurrentPhase()
{
    if(_currentPhase == TrafficLightPhase::red)
    {
        _currentPhase = TrafficLightPhase::green;
    }
    else
    {
        _currentPhase = TrafficLightPhase::red;
    }
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that 
    // 1) Measures the time between two loop cycles 
    // 2) Toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. 
    // 3) The cycle duration should be a random value between 4000 and 6000 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    int cycleDuration = getRandomCycleDuration();
    std::chrono::time_point<std::chrono::system_clock> start;

    start = std::chrono::system_clock::now();
    
    while (true) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();
        
        long duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count();
        
        if (duration < cycleDuration) 
        {
            continue;
        }
        
        toggleCurrentPhase();
        _messageQueue.send(std::move(_currentPhase));

        start = std::chrono::system_clock::now();
        cycleDuration = getRandomCycleDuration();
    }
}
