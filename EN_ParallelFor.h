#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace EN
{
    class EN_ParallelFor
    {
    private:
        std::mutex StepMutex, ThreadFinishingMutex, ConditionVarMutex;
        std::condition_variable CondVar;
        int ThreadAmount, EndedThreads;

    private:

        template<class T, class F>
        void ThreadFunc(T& Pos, const T& End, const F& f)
        {
            T LocalPos;
            while (true)
            {
                StepMutex.lock();
                LocalPos = Pos;

                if (Pos != End)
                    Pos++;
                else
                {
                    ThreadFinishingMutex.lock();
                    EndedThreads++;
                    StepMutex.unlock();
                    break;
                }

                StepMutex.unlock();

                f(LocalPos);
            }

            if (EndedThreads >= ThreadAmount)
                CondVar.notify_all();
            ThreadFinishingMutex.unlock();
        }

    public:
        template<class T, class F>
        void operator() (T start, T end, int thread_amount, const F& f)
        {
            EndedThreads = 0;
            T Pos = start; T End = end; ThreadAmount = thread_amount;

            std::unique_lock<std::mutex> CondVarUniqueLock(ConditionVarMutex);

            for (int i = 0; i < thread_amount; i++)
            {
                std::thread th([this, f, &Pos, &End]()
                    {
                        ThreadFunc(Pos, End, f);
                    });
                th.detach();
            }

            CondVar.wait(CondVarUniqueLock);
        }
    };
}