[![GitHub license](https://img.shields.io/github/license/helios-base/helios-base)](https://github.com/helios-base/helios-base/blob/master/LISENCE)

# About
RoboCup is an international competition aimed at advancing autonomous robotics and AI through tasks like soccer and rescue. The RoboCup Soccer Simulation 2D league focuses on developing intelligent agents that play soccer in a simulated 2D environment. This league is ideal for testing and developing AI and ML algorithms, including reinforcement learning and multi-agent systems. [more details](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Definitions)

![image](https://github.com/Cross-Language-Soccer-Framework/cross-language-soccer-framework/assets/25696836/7b0b1d49-7001-479c-889f-46a96a8802c4)

To run a game in the **RoboCup Soccer Simulation 2D**, you need to operate the [rcssserver](https://github.com/rcsoccersim/rcssserver) for hosting games, [rcssmonitor](https://github.com/rcsoccersim/rcssmonitor) to display them, and engage 12 agents (11 players and a coach) per team. Each cycle, agents receive data from the server and must execute actions such as dash and kick.

Developing a team can be complex due to the environment's intricacy, typically necessitating C++ programming. However, our framework allows for development in other languages by leveraging the [helios-base](https://github.com/helios-base/helios-base) features. By using **SoccerSimulationProxy**, you can develop a team in any language supported by **gRPC** or **Thrift**, such as **C#, C++, Dart, Go, Java, Kotlin, Node.js, Objective-C, PHP, Python, and Ruby**.

To use **gRPC**, you can check out our [gRPC server](https://github.com/CLSFramework/sample-playmaker-server-python-grpc), which is based on proto messages and gRPC services. This server provides a helpful base to get more familiar with the gRPC implementation. 

To use **Thrift**, you can check out our [thrift server](https://github.com/CLSFramework/sample-playmaker-server-python-thrift), which is based on proto messages and thrift services. This server provides a helpful base to get more familiar with the thrift implementation.

This allows you to focus on developing your team's strategy and AI algorithms without worrying about the server's underlying complexity.

Check our [wiki](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Protobuf) to be more familier with messages and services.

![image](https://github-production-user-asset-6210df.s3.amazonaws.com/25696836/364993436-4daee216-1479-4acd-88f2-9e772b8c7837.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20240923%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20240923T175355Z&X-Amz-Expires=300&X-Amz-Signature=f985fcc6c8a34d6db99f322fd5f3a0dacee317097dd5519329ea3bdb7cb5d818&X-Amz-SignedHeaders=host)


If you would like to develop a team or conduct research using **Python**, **C#**, or **JavaScript**, you can check the following links:

- [Playmaker-Server-Python-grpc](https://github.com/CLSFramework/sample-playmaker-server-python-grpc)
- [Playmaker-Server-Python-thrift](https://github.com/CLSFramework/sample-playmaker-server-python-thrift)
- [Playmaker-Server-CSharp](https://github.com/CLSFramework/playmaker-server-csharp)
- [Playmaker-Server-NodeJs](https://github.com/CLSFramework/playmaker-server-nodejs)

To find more information about the framework, you can visit the [CLSFramework Wiki Pages](https://github.com/CLSFramework/cross-language-soccer-framework/wiki)



This new base code is powered by Helios-Base and gRPC, designed to assist researchers in developing a Soccer Simulation 2D team or conducting research in this area. It supports development in any language compatible with gRPC.
## How To Use it?

To use this framework, follow the steps below in order:

### 1. Start the **rcssserver**
The **rcssserver** hosts the game. You can follow the instructions for setting it up in the [RoboCup Soccer Simulation Server Wiki](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/RoboCup-Soccer-Simulation-Server).

### 2. Run the **Playmaker-Server**
Next, run one of the sample Playmaker Servers, such as this [gRPC Server](https://github.com/CLSFramework/sample-playmaker-server-python-grpc), to receive information from the agents and send appropriate actions back to the game.

### 3. Set up the **Soccer Simulation Proxy**
Now, run the **Soccer Simulation Proxy** to connect to the **rcssserver** and handle information exchange between agents and the server. You can do this using AppImage, Docker, or by building from source. 

Here, we’ll explain how to run the Soccer Simulation Proxy using AppImage

For more information on using Docker or building from source, visit the [CLSFramework Wiki Pages](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Soccer-Simulation-Proxy).

### AppImage

#### 1. Download the AppImage
 You can download the AppImage from the [release page](https://github.com/CLSFramework/soccer-simulation-proxy/releases) or use the following command to download the latest version:
   ```bash
   wget $(curl -s "https://api.github.com/repos/clsframework/soccer-simulation-proxy/releases/latest" | grep -oP '"browser_download_url": "\K[^"]*' | grep "soccer-simulation-proxy.tar.gz")
   ```
### 2. Extract the AppImage
After downloading, extract the tar.gz file:
```bash
tar -xvf soccer-simulation-proxy.tar.gz
```
### 3. Run the Proxy
After downloading, you can run the proxy:
```bash
cd SoccerSimulationProxy
./start.sh
```
If you want to connect the proxy to a grpc server change this parameter to `grpc` in start.sh file.
```bash
rpc_type="thrift"
```
### 4. Watch the Game
To watch the game, you can use either of the following:

- **[rcssmonitor](https://github.com/rcsoccersim/rcssmonitor)**: A tool to visualize the game.
- **[SoccerWindow2](https://github.com/helios-base/soccerwindow2)**: Another visualization tool for RoboCup Soccer Simulation.

For instructions on how to run **rcssmonitor**, check the [Soccer Simulation Monitor Wiki](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Soccer-Simulation-Monitor).

![Screenshot 2024-04-07 012226](https://github.com/Cyrus2D/SoccerSimulationProxy/assets/25696836/abb24e0c-61b9-497d-926f-941d1c90e2ee)


## References

The paper about HELIOS Base:
- Hidehisa Akiyama, Tomoharu Nakashima, HELIOS Base: An Open Source
Package for the RoboCup Soccer 2D Simulation, In Sven Behnke, Manuela
Veloso, Arnoud Visser, and Rong Xiong editors, RoboCup2013: Robot
World XVII, Lecture Notes in Artificial Intelligence, Springer Verlag,
Berlin, 2014. http://dx.doi.org/10.1007/978-3-662-44468-9_46

# Citation

- [Cross Language Soccer Framework](https://arxiv.org/pdf/2406.05621)
- Zare, N., Sayareh, A., Sadraii, A., Firouzkouhi, A. and Soares, A., 2024. Cross Language Soccer Framework: An Open Source Framework for the RoboCup 2D Soccer Simulation. arXiv preprint arXiv:2406.05621.