#include "EN_ThreadCrossWalk.h"

namespace EN
{
    EN_ThreadCrossWalk::EN_ThreadCrossWalk() 
    { 
        RoadCounter.store(0);
        IsCondVarWaiting.store(false);
    }

    /// If the PedestrianStartCrossRoad method was called before, 
    /// then this method will wait until the PedestrianStopCrossRoad method is called
    void EN_ThreadCrossWalk::CarStartCrossRoad()
    {
        Mtx.lock();
        RoadMtx.lock();
        RoadCounter.fetch_add(1);
        RoadMtx.unlock();
        Mtx.unlock();
    }

    void EN_ThreadCrossWalk::CarStopCrossRoad()
    {
        RoadMtx.lock();
        RoadCounter.fetch_sub(1);

        if (RoadCounter.load() == 0)
            while (IsCondVarWaiting.load()) Cv.notify_all();
        
        RoadMtx.unlock();
    }

    /// If the CarStartCrossRoad methods were called before, 
    /// then this method will wait until the last of the CarStopCrossRoad methods is called
    void EN_ThreadCrossWalk::PedestrianStartCrossRoad()
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> lk(mtx);

        Mtx.lock();
        
        IsCondVarWaiting.store(true);

        if (RoadCounter > 0) Cv.wait(lk);
        
        IsCondVarWaiting.store(false);
    }

    void EN_ThreadCrossWalk::PedestrianStopCrossRoad()
    {
        Mtx.unlock();
    }
}