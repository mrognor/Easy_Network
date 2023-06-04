#include "EN_ThreadCrossWalk.h"

namespace EN
{
    ThreadCrossWalk::ThreadCrossWalk() 
    { 
        RoadCounter.store(0);
        IsCondVarWaiting.store(false);
    }

    /// If the PedestrianStartCrossRoad method was called before, 
    /// then this method will wait until the PedestrianStopCrossRoad method is called
    void ThreadCrossWalk::CarStartCrossRoad()
    {
        Mtx.lock();
        RoadMtx.lock();
        RoadCounter.fetch_add(1);
        RoadMtx.unlock();
        Mtx.unlock();
    }

    void ThreadCrossWalk::CarStopCrossRoad()
    {
        RoadMtx.lock();
        RoadCounter.fetch_sub(1);

        if (RoadCounter.load() == 0)
            while (IsCondVarWaiting.load()) Cv.notify_all();
        
        RoadMtx.unlock();
    }

    /// If the CarStartCrossRoad methods were called before, 
    /// then this method will wait until the last of the CarStopCrossRoad methods is called
    void ThreadCrossWalk::PedestrianStartCrossRoad()
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> lk(mtx);

        Mtx.lock();
        
        IsCondVarWaiting.store(true);

        if (RoadCounter > 0) Cv.wait(lk);
        
        IsCondVarWaiting.store(false);
    }

    void ThreadCrossWalk::PedestrianStopCrossRoad()
    {
        Mtx.unlock();
    }
}