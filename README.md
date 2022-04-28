# Easy_Network
Simple C++ network library
# Installation guide
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

7. The configuration of the project for working with the library is completed. Now you can take the code samples and try to compile yourself
Full client example: https://github.com/mrognor/Easy_Network/blob/master/Client.cpp  
Full server example: https://github.com/mrognor/Easy_Network/blob/master/Server.cpp
