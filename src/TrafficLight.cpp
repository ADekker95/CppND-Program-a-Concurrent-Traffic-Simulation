#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <class T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition_var.wait(uLock, [this] { return !_queue.empty(); });  
  
      // pull message from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();
  
  	return msg;
  
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // lock the queue 
    std::lock_guard<std::mutex> lockGuard(_mutex);
    // add message to queue 
    _queue.emplace_back(std::move(msg)); 
    // send notification
    _condition_var.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

// needed because rule of 5
TrafficLight::~TrafficLight()
{
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  
    while (true)
    {
    	TrafficLightPhase trafficLight = _msgqueue.receive();
        if (trafficLight == TrafficLightPhase::green)
      		return;
    }  
  
  }

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  // create thread
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));   
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  std::random_device randev;
  std::mt19937 eng(randev());
  std::uniform_int_distribution<> distr(4, 6);
  double CycleDuration = distr(eng); 
  std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();  
  
  while (true){ 
      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // sleep per iteration
      std::chrono::high_resolution_clock::time_point t1 = std::chrono::system_clock::now();
      //std::chrono::duration<double> TimeDelta = t1 - t0;
      auto TimeDelta = std::chrono::duration_cast<std::chrono::microseconds>( t1 - t0 ).count();
      if (TimeDelta > CycleDuration){
        // toggle the current color to the other color
      	_currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::red : TrafficLightPhase::green;
      	_msgqueue.send(std::move(_currentPhase));
      }

      // reset stopwatch
	  std::chrono::high_resolution_clock::time_point t0 = std::chrono::system_clock::now();
      double CycleDuration = distr(eng); 
    }
}

