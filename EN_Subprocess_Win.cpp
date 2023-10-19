#include "EN_Subprocess_Win.h"

#pragma once

namespace EN
{
    void Subprocess::Launch(std::string programmName)
    {
        DataCounter.store(0);
        IsProcessEnded.store(false);

        SECURITY_ATTRIBUTES saAttr;

        // Set the bInheritHandle flag so pipe handles are inherited.
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        // Create a pipe for the child process's STDOUT. Read from 0, write to 1
        if (!CreatePipe(&pipeFromChild[0], &pipeFromChild[1], &saAttr, 0))
        {
            std::cerr << "Failed to create pipe!" << std::endl;
            return;
        }

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!SetHandleInformation(pipeFromChild[0], HANDLE_FLAG_INHERIT, 0))
        {
            std::cerr << "Failed to create pipe!" << std::endl;
            return;
        }

        // Create a pipe for the child process's STDIN.
        if (!CreatePipe(&pipeToChild[0], &pipeToChild[1], &saAttr, 0))
        {
            std::cerr << "Failed to create pipe!" << std::endl;
            return;
        }

        // Ensure the write handle to the pipe for STDIN is not inherited.
        if (!SetHandleInformation(pipeToChild[1], HANDLE_FLAG_INHERIT, 0))
        {
            std::cerr << "Failed to create pipe!" << std::endl;
            return;
        }

        STARTUPINFOA siStartInfo;
        BOOL bSuccess = FALSE;

        // Set up members of the PROCESS_INFORMATION structure.

        ZeroMemory(&ProcInfo, sizeof(PROCESS_INFORMATION));

        // Set up members of the STARTUPINFO structure.
        // This structure specifies the STDIN and STDOUT handles for redirection.

        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = pipeFromChild[1];
        siStartInfo.hStdOutput = pipeFromChild[1];
        siStartInfo.hStdInput = pipeToChild[0];
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process.
        bSuccess = CreateProcessA(NULL,
            (LPSTR)programmName.c_str(),    // command line
            NULL,         // process security attributes
            NULL,         // primary thread security attributes
            TRUE,         // handles are inherited
            0,            // creation flags
            NULL,         // use parent's environment
            NULL,         // use parent's current directory
            &siStartInfo, // STARTUPINFO pointer
            &ProcInfo); // receives PROCESS_INFORMATIONx

        if (!bSuccess)
        {
            std::cerr << "Failed to create child process!" << std::endl;
            return;
        }

        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.
        CloseHandle(pipeFromChild[1]);
        CloseHandle(pipeToChild[0]);

        Th = std::thread([&]()
            {
                while (true)
                {
                    // Wait while some bytes write
                    ReadFile(pipeFromChild[0], NULL, 0, NULL, NULL);

                    // Get amount of available bytes
                    DWORD availableBytes;
                    PeekNamedPipe(pipeFromChild[0], NULL, 0, NULL, &availableBytes, NULL);

                    char* bytes = new char[availableBytes];
                    DWORD readed;

                    bSuccess = ReadFile(pipeFromChild[0], bytes, availableBytes, &readed, NULL);

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
                            {
                                Cv.notify_all();
                            }

                            MessagesQueue.push(std::string(bytes, readed));
                            DataCounter.fetch_add(1);
                            Mtx.unlock();
                        }

                    }
                    delete[] bytes;
                }
            });
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

        DWORD dwWritten;
        WriteFile(pipeToChild[1], data.c_str(), data.length(), &dwWritten, NULL);

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
            // Remove CR LF from res
            if(!res.empty())
                res.pop_back();
            
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
            // Remove CR LF from res
            if(!res.empty())
                res.pop_back();
            
            if(!res.empty())
                res.pop_back();
        }

        return res;
    }

    void Subprocess::StopProcess(std::string commandToSendToProcessToStop, bool isRequiredNewLineSymbol)
    {
        SendData(commandToSendToProcessToStop, isRequiredNewLineSymbol);
        Th.join();

        DWORD result = WaitForSingleObject(ProcInfo.hProcess, INFINITE);
        
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 
        CloseHandle(ProcInfo.hProcess);
        CloseHandle(ProcInfo.hThread);

        if (result != WAIT_OBJECT_0 && result != WAIT_TIMEOUT)
            std::cerr << "Process finished with errors!" << std::endl;
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