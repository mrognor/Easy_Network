#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace EN
{
    /// Class implementing a parallel for
    class EN_ParallelFor
    {
    private:
        std::mutex StepMutex, ThreadFinishingMutex, ConditionVarMutex;
        std::condition_variable CondVar;
        int ThreadAmount, EndedThreads;

    private:

        // A function running in a separate thread. Calls a custom function
        // Gets template position and end. Its can be iterator or a numbers
        // Gets lambda function to invoke it
        template<class T, class F>
        void ThreadFunc(T& Pos, const T& End, const F& lambda_func)
        {
            T LocalPos; // Local var to store thread position
            while (true)
            {
                StepMutex.lock(); // Start working with positions
                LocalPos = Pos;

                if (Pos != End) // Check if its last element
                    Pos++;
                else
                {
                    ThreadFinishingMutex.lock(); // Start working with thread killing
                    EndedThreads++; // Increment stopped threads amount
                    StepMutex.unlock(); // Stop working with positions
                    break;
                }

                StepMutex.unlock(); // Stop working with positions

                lambda_func(LocalPos); // Invokes user function 
            }

            if (EndedThreads >= ThreadAmount) // Checks if all threads ended
                CondVar.notify_all(); // Notify condition variable to exit from operator()
            ThreadFinishingMutex.unlock(); // Stop working with thread killing
        }

    public:
        /**
            \brief Method that start and execute thread pool

            \param[in] start Template variable to indicate the start of the cycle. Can be an iterator or a number
            \param[in] end Template variable to indicate the end of the cycle. Can be an iterator or a number
            \param[in] thread_amount The thread amount
            \param[in] lambda_func The user lambda function to invokes in every iterarion. 
            The lambda function accepts one parameter of the same type as the first two parameters. 
            This function is called for all values between start and end.
        */
        template<class T, class F>
        void operator() (T start, T end, int thread_amount, const F& lambda_func)
        {
            EndedThreads = 0; 
            T Pos = start; T End = end; ThreadAmount = thread_amount;

            std::unique_lock<std::mutex> CondVarUniqueLock(ConditionVarMutex);

            // Creating threads with lambdas
            for (int i = 0; i < thread_amount; i++)
            {
                std::thread th([this, lambda_func, &Pos, &End]()
                    {
                        ThreadFunc(Pos, End, lambda_func);
                    });
                th.detach();
            }

            CondVar.wait(CondVarUniqueLock);
        }
    };
}