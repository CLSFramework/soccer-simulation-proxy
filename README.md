[![GitHub license](https://img.shields.io/github/license/helios-base/helios-base)](https://github.com/helios-base/helios-base/blob/master/LISENCE)

# About
RoboCup is an international competition aimed at advancing autonomous robotics and AI through tasks like soccer and rescue. The RoboCup Soccer Simulation 2D league focuses on developing intelligent agents that play soccer in a simulated 2D environment. This league is ideal for testing and developing AI and ML algorithms, including reinforcement learning and multi-agent systems. [more details](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Definitions)

![image](https://github.com/Cross-Language-Soccer-Framework/cross-language-soccer-framework/assets/25696836/7b0b1d49-7001-479c-889f-46a96a8802c4)

To run a game in the **RoboCup Soccer Simulation 2D**, you need to operate the [rcssserver](https://github.com/rcsoccersim/rcssserver) for hosting games, [rcssmonitor](https://github.com/rcsoccersim/rcssmonitor) to display them, and engage 12 agents (11 players and a coach) per team. Each cycle, agents receive data from the server and must execute actions such as dash and kick.

Developing a team can be complex due to the environment's intricacy, typically necessitating C++ programming. However, our framework allows for development in other languages by leveraging the [helios-base](https://github.com/helios-base/helios-base) features. By using **SoccerSimulationProxy**, you can develop a team in any language supported by **gRPC** or **Thrift**, such as **C#, C++, Dart, Go, Java, Kotlin, Node.js, Objective-C, PHP, Python, and Ruby**.

To use **gRPC**, you just need to develop a gRPC server based on proto messages and gRPC services ([protofile](https://github.com/CLSFramework/soccer-simulation-proxy/blob/master/idl/grpc/service.proto), check the [wiki](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Protobuf) to become more familiar with messages and services) to receive data from the **SoccerSimulationProxy** and send actions back to it.

To use **Thrift**, you can similarly develop a Thrift server by generating code from the Thrift [IDL](https://github.com/CLSFramework/soccer-simulation-proxy/blob/master/idl/thrift/soccer_service.thrift) (Interface Definition Language) provided. This way, your team can interact with the proxy using the Thrift-generated code to receive data and send actions.

This allows you to focus on developing your team's strategy and AI algorithms without worrying about the server's underlying complexity.

If you would like to develop a team or conduct research using **Python**, **C#**, or **JavaScript**, you can check the following links:

- [Playmaker-Server-Python](https://github.com/CLSFramework/playmaker-server-python)
- [Playmaker-Server-CSharp](https://github.com/CLSFramework/playmaker-server-csharp)
- [Playmaker-Server-NodeJs](https://github.com/CLSFramework/playmaker-server-nodejs)

![image](https://github-production-user-asset-6210df.s3.amazonaws.com/25696836/364993436-4daee216-1479-4acd-88f2-9e772b8c7837.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20240923%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20240923T175355Z&X-Amz-Expires=300&X-Amz-Signature=f985fcc6c8a34d6db99f322fd5f3a0dacee317097dd5519329ea3bdb7cb5d818&X-Amz-SignedHeaders=host)

To find more information about the framework, you can visit the [CLSFramework Wiki Pages](https://github.com/CLSFramework/cross-language-soccer-framework/wiki)


This new base code is powered by Helios-Base code and gRPC to help researcher developing a soccer simulation 2D team or researching in this area by using any languages supported by gRPC: 
 - [C#](https://grpc.io/docs/languages/csharp/) 
   - [Playmaker-Server-CSharp](https://github.com/CLSFramework/playmaker-server-csharp)
 - [C++](https://grpc.io/docs/languages/cpp/)
 - [Dart](https://grpc.io/docs/languages/dart/)
 - [Go](https://grpc.io/docs/languages/go/)
 - [Java](https://grpc.io/docs/languages/java/)
 - [Kotlin](https://grpc.io/docs/languages/kotlin/)
 - [Node](https://grpc.io/docs/languages/node/)
   - [Playmaker-Server-NodeJs](https://github.com/CLSFramework/playmaker-server-nodejs)
 - [Objective-C](https://grpc.io/docs/languages/objective-c/)
 - [PHP](https://grpc.io/docs/languages/php/)
 - [Python](https://grpc.io/docs/languages/python/)
   - [Playmaker-Server-Python](https://github.com/CLSFramework/playmaker-server-python)
 - [Ruby](https://grpc.io/docs/languages/ruby/)

## How To Use it?

To use this framework, you need to run rcssserver to host a game, run SoccerSimulationProxy (Agents) to connect to the rcssserver to receive information and send actions, and run Playmaker-Server (gRPC-Server) to receive information from agents and send appropriate actions back. To watch the game, you can run rcssmonitor or SoccerWindow2.

To run the rcssserver and rcssmonitor, you can follow the instructions in the [rcssserver](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/RoboCup-Soccer-Simulation-Server) and [rcssmonitor](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Soccer-Simulation-Monitor).

To run the SoccerSimulationProxy, there are some different ways such as using Docker, building from source, or using AppImage.

In this page, we will explain how you can run the SoccerSimulationProxy by using AppImage. To find more information about building from source or using Docker, you can visit the [CLSFramework Wiki Pages](https://github.com/CLSFramework/cross-language-soccer-framework/wiki/Soccer-Simulation-Proxy).

## Use AppImage (Linux, WSL)

### Download
You can download the AppImage of the Soccer Simulation Proxy from the [release page](https://github.com/CLSFramework/soccer-simulation-proxy/releases).

Or download the latest version by using the following command:
```bash
wget $(curl -s "https://api.github.com/repos/clsframework/soccer-simulation-proxy/releases/latest" | grep -oP '"browser_download_url": "\K[^"]*' | grep "soccer-simulation-proxy.tar.gz")
```

### Extract
```bash
tar -xvf soccer-simulation-proxy.tar.gz
```

### Run
```bash
cd SoccerSimulationProxy
./start.sh
```

## References

The paper about HELIOS Base:
- Hidehisa Akiyama, Tomoharu Nakashima, HELIOS Base: An Open Source
Package for the RoboCup Soccer 2D Simulation, In Sven Behnke, Manuela
Veloso, Arnoud Visser, and Rong Xiong editors, RoboCup2013: Robot
World XVII, Lecture Notes in Artificial Intelligence, Springer Verlag,
Berlin, 2014. http://dx.doi.org/10.1007/978-3-662-44468-9_46
