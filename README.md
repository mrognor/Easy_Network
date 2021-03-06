\mainpage
# Easy_Network
Simple C++ network library
# Library documentation:  
https://mrognor.github.io/Easy_Network_Documentation/doc/html/index.html  

# Library usage 
Before you start, I advise you to download the code examples. For linux, you can save files to one folder, and for visual studio, you will need to add these files to different projects  
Examples available here: https://github.com/mrognor/Easy_Network/tree/master/Examples

## Windows library usage guide
1. Download the latest release from github
2. Create a new project in visual studio
3. Click project -> properties

![VisualStudioProjectProperties](https://user-images.githubusercontent.com/47296449/165805905-60992708-92ed-4107-a858-e94e13581369.png)

4. Make sure that the configuration is selected to "All Configurations"

Open menu on C/C++ -> General -> Additional Include Directories
and add the path to the include library folder there

![VisualStudioProjectInclude](https://user-images.githubusercontent.com/47296449/165806651-ee06a01e-4438-4b5b-a2e9-bf17fd1a938d.png)

5. Open menu on Linker -> General -> Additional Library Directories
and add the path to the lib library folder there

![image](https://user-images.githubusercontent.com/47296449/165807739-68487caa-84ea-4caf-8630-63198174dea2.png)

6. Open menu on Linker -> Input -> Additional Dependencies and add "EasyNetwork.lib" here. 
![image](https://user-images.githubusercontent.com/47296449/165808413-9a2e783d-5a25-4829-858e-c48ed7267bac.png)

7. The configuration of the project for working with the library is completed. Now you can use the code samples and try to make own programm  


## Linux library usage
1. Download the latest release from github
2. You can extract library release archive to any folder. I will save everything in one folder for convenience

![image](https://user-images.githubusercontent.com/47296449/166112771-628c328f-cef5-4c75-a440-d018841f351b.png)

3. Now all ready to compile your project  
You can compile server using command:  
`g++ -pthread TCPServer.cpp bin/libEasyNetwork.a -o server.sh`  
You can compile client using command:   
`g++ -pthread TCPClient.cpp bin/libEasyNetwork.a -o client.sh`  
4. Run your programs using command:  
`./server.sh` and `./client.sh`

![image](https://user-images.githubusercontent.com/47296449/166113135-c9fc8b7d-bf3a-4589-bdce-3582c5f7abd0.png)

## Important
I know that no one reads further than the readme, so I want to bring some important things here  
1. When working with linux, you may notice that std::cout does not
output characters to the console. Don't worry, 
your code works as it should, you can check it yourself with cerr.  
2. On linux, after disconnecting clients from the server, 
the port may be blocked for a small amount of time. 
Due to when trying to start the server again, 
errors will come out. We just have to wait a little  
3. 