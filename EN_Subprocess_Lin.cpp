#include "EN_Subprocess_Lin.h"

namespace EN
{
    void Subprocess::Launch(std::string programmName)
    {
        DataCounter.store(0);
        IsProcessEnded.store(false);

        if (pipe(PipeToChild))
        {
            std::cerr << "Failed to create pipe!" << std::endl;
            return;
        }

        if (pipe(PipeFromChild))
        {
            std::cerr << "Failed to create pipe!" << std::endl;
            return;
        }

        // Create fork of current process
        Pid = fork();

        // Parent process
        if (Pid > (pid_t)0)
        {
            // Close unrequired pipes ends
            close(PipeToChild[0]);
            close(PipeFromChild[1]);

            Th = std::thread([&]()
            {
                // Array of descriptors to poll
                fd_set requiredPipeEndsToPoll;

                while (true)
                {
                    // Zeroed set
                    FD_ZERO(&requiredPipeEndsToPoll);
                    // Add fdout[0] to poll set
                    FD_SET(PipeFromChild[0], &requiredPipeEndsToPoll);
      
                    // Wait data in PipeFromChild
                    if (select(PipeFromChild[1] + 1, &requiredPipeEndsToPoll, NULL, NULL, NULL) == -1)
                    {
                        std::cerr << "Failed to select data from pipe! Errno: " << errno << std::endl;
                        return;
                    }

                    // Check if it is data from child inside pipe
                    if(FD_ISSET(PipeFromChild[0], &requiredPipeEndsToPoll))
                    {
                        // Get available data  
                        int availableBytes;
                        ioctl(PipeFromChild[0], FIONREAD, &availableBytes);

                        // Allocate array
                        char* bytes = new char [availableBytes];

                        // Read data from pipe
                        int readed = read(PipeFromChild[0], bytes, availableBytes);

                        // EOF and process finishing
                        if (readed == 0)
                        {
                            // Thread safety append data to queue and trigger wait function
                            while (!Mtx.try_lock())
                                Cv.notify_all();
                            
                            IsProcessEnded.store(true);
                            Mtx.unlock();

                            delete[] bytes;
                            break;
                        }
                        else
                        {
                            if (readed != availableBytes)
                            {
                                std::cerr << "Failed to read data from pipe!" << std::endl;
                                delete[] bytes;
                                return;
                            }
                            else
                            {
                                // Thread safety append data to queue and trigger wait function
                                while (!Mtx.try_lock())
                                    Cv.notify_all();

                                MessagesQueue.push(std::string(bytes, readed));
                                DataCounter.fetch_add(1);
                                Mtx.unlock();
                            }
                        }

                        delete[] bytes;
                    }
                }
            });
            return;
        }

        // Child process
        if (Pid == (pid_t)0)
        {
            close(STDOUT_FILENO);
            close(STDIN_FILENO);

            close(PipeToChild[1]);
            close(PipeFromChild[0]);

            dup2(PipeToChild[0], STDIN_FILENO);
            dup2(PipeFromChild[1], STDOUT_FILENO);

            close(PipeToChild[0]);
            close(PipeFromChild[1]);

            std::string command = programmName + " 2>&1";
            
            system(command.c_str());

            exit(EXIT_SUCCESS);
        }
        
        // Error
        if (Pid < (pid_t)0)
        {
            std::cerr << "Failed to create fork!" << std::endl;
            return;
        }
    }

    void Subprocess::SendData(std::string data, bool isRequiredNewLineSymbol)
    {
        Mtx.lock();
        
        if (IsProcessEnded.load())
        {
            Mtx.unlock();
            return;
        }

        if (isRequiredNewLineSymbol)
            data += '\n';

        write(PipeToChild[1], data.c_str(), data.length());
        Mtx.unlock();
    }

    std::string Subprocess::GetData(bool isRemoveNewLineSymbols)
    {
        std::string res = "";
        Mtx.lock();
        if (!MessagesQueue.empty())
        {
            res = MessagesQueue.front();
            MessagesQueue.pop();
            DataCounter.fetch_sub(1);
        }
        Mtx.unlock();

        if (isRemoveNewLineSymbols)
        {
            // Remove LF from res
            if(!res.empty())
                res.pop_back();
        }

        return res;
    }

    std::string Subprocess::WaitData(bool isRemoveNewLineSymbols)
    {
        std::string res = "";
        std::mutex cvMtx;
        std::unique_lock <std::mutex> lk(cvMtx);

StartWaiting:
        Mtx.lock();

        if (IsProcessEnded.load())
        {
            Mtx.unlock();
            return "";
        }

        if (!MessagesQueue.empty())
        {
            res = MessagesQueue.front();
            MessagesQueue.pop();
            DataCounter.fetch_sub(1);
            Mtx.unlock();
        }
        else
        {
            Cv.wait(lk);
            Mtx.unlock();
            goto StartWaiting;
        }

        if (isRemoveNewLineSymbols)
        {
            // Remove LF from res
            if(!res.empty())
                res.pop_back();
        }

        return res;
    }

    void Subprocess::StopProcess(std::string commandToSendToProcessToStop, bool isRequiredNewLineSymbol)
    {
        SendData(commandToSendToProcessToStop, isRequiredNewLineSymbol);
        Th.join();

        int a;
        waitpid(Pid, &a, 0);
        close(PipeToChild[1]);
        close(PipeFromChild[0]);
    }

    bool Subprocess::IsData()
    {
        return DataCounter.load() > 1; 
    }

    bool Subprocess::GetIsProcessEnded()
    {
        return IsProcessEnded.load();
    }

    Subprocess::~Subprocess()
    {
        if (Th.joinable())
        {
            std::cerr << "Thread was not joined! Higly possible thar you dont stop process" << std::endl;
        }
    }
};