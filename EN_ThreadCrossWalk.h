#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>

namespace EN
{
        /**
        \brief A class for synchronizing threads

        The work of this class can be represented using a pedestrian crossing. 
        For simplicity, you can imagine the work of this class on a two-lane road, 
        but it can work with any number of roads. 
        Cars can drive on two lanes of the same road completely independently of each other.  
        At the same time, there is a pedestrian crossing on the road, and when a pedestrian approaches it, 
        cars will no longer be able to enter the road, 
        the pedestrian will wait until all the cars that were on the road when he came will finish their movement. 
        After that, he will cross the road and cars will be able to move on the road again. 
        Now let's move on to threads. You have a lot of threads that can run in parallel, 
        i.e. they don't have to be synchronized with each other. 
        And there is one thread that should only work when all other threads are not working. 
        For example, when working with some container. For simplicity, 
        let's imagine that we are working with a vector in which there are 2 elements. 
        2 threads independently work with different elements of the vector. 
        At the same time, you want to change the size of the vector. This class can help you with this. 
        Working with reading and writing to vector cells is machines, and resizing a vector is a pedestrian
    */
    class EN_ThreadCrossWalk
    {
    private:
        std::mutex Mtx, RoadMtx;
        std::atomic_int RoadCounter;
        std::atomic_bool IsCondVarWaiting;
        std::condition_variable Cv;
    public:
        EN_ThreadCrossWalk();

        /// If the PedestrianStartCrossRoad method was called before, 
        /// then this method will wait until the PedestrianStopCrossRoad method is called
        void CarStartCrossRoad();

        void CarStopCrossRoad();

        /// If the CarStartCrossRoad methods were called before, 
        /// then this method will wait until the last of the CarStopCrossRoad methods is called
        void PedestrianStartCrossRoad();

        void PedestrianStopCrossRoad();
    };
}