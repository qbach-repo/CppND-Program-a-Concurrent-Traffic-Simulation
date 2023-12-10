#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>
#include<future>
/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(_mutex);
    
    _condition.wait(lock, [this] {return !_queue.empty();});
    
    auto msg = std::move(_queue.back());
    
    _queue.pop_back();

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lguard(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (1)
        if (_queue->receive() == TrafficLightPhase::green)
            return;
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    auto start = std::chrono::system_clock::now();

    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_real_distribution<> distr(4.0, 6.0); // define the range

    auto cycle_duration = distr(gen);


    while(1) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;

        if (elapsed.count() >= cycle_duration) 
        {
            if (_currentPhase == TrafficLightPhase::green)
                _currentPhase = TrafficLightPhase::red;
            else
                _currentPhase = TrafficLightPhase::green;
            
            auto message = _currentPhase;
            auto f_send = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _queue, std::move(message));
            f_send.wait();

            //cycle_duration = distr(gen);
            start = std::chrono::system_clock::now();       
        }
    }
}

