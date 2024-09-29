# ChangeLog

## [1.0.1] - 2024-09-15

### Added
- catch_time has been added to the proxy in the self message.
- kickable_opponent_existance and kickable_teammate_existance has been added to the proxy in the worldmodel message.
- bhv_doforceKick action has been added as a message and to the actions message.

### Fixed
- 

### Changed
- 

### Engineers
- [SoroushMazloum](https://github.com/SoroushMazloum)
- [SadraKhanjari](https://github.com/SK2iP)


## [1.0.0] - 2024-09-15

### Added
- added rpc_version to the RegisterRequest message.
- added rpc_server_language_type to the RegisterResponse message.
- added server side planner decision maker.

### Fixed
- fixed bugs in the getActions functions in thrift and grpc. 

### Changed
- changed chain_action messages name to planner

### Engineers
- [NaderZare](https://github.com/naderzare)
- [SadraKhanjari](https://github.com/SK2iP)


## [0.1.4] - 2024-09-03

### Added
-

### Fixed
- 

### Changed
- Change the structure of the RPC clients.
    - Move some fields and methods in gRrpc/thrift client to the base class (```IRpcClient```).
    - Add ```RpcPlayerClient``` that handles the preprocess check and execution.
    - The ```ThriftPlayerClient``` and ```GrpcPlayerClient```  inherit from the ```RpcPlayerClient``` for preprocess handling.
- Preprocess:
    - Add ```need_preprocess``` to the ```State``` message.
    - Add ```ignore_preprocess``` to the ```PlayerActions``` message.
    - Player Agents now first check whether they require preprocess actions, send the ```bool``` as the ```need_preprocess``` field in the ```State``` message. Then, if the server sends the ```ignore_preprocess=false (default value)``` to the proxy, the proxy will call ```doPreprocess``` method. If ther server sends the ```ignore_preprocess=true``` to the proxy, the proxy will not call the ```doPreprocess``` method and execute the ```PlayerActoins```.


## [0.1.3] - 2024-09-02

### Added
- 

### Fixed
- bug fixed in start files (by [NaderZare](https://github.com/naderzare), [ArefSayareh](https://github.com/Arefsa78))

### Changed
- change input arguments names in start files (by [NaderZare](https://github.com/naderzare), [ArefSayareh](https://github.com/Arefsa78))


## [0.1.2] - 2024-09-01

### Added
- Support gRPC and Thrift. (by [NaderZare](https://github.com/naderzare), [ArefSayareh](https://github.com/Arefsa78))

### Fixed
- bug fixed in grpc trainer

### Changed
- 

## [0.1.1] - 2024-08-25

### Added
-

### Fixed
-

### Changed
-
