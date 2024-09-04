# ChangeLog

## [0.1.3] - 2024-09-03

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
    - Player Agents now first check whether they require preprocess actions, sent the ```bool``` as the ```need_preprocess``` field in the ```State``` message. Then, if the server sends the ```ignore_preprocess=false (default value)``` to the proxy, the proxy will call ```doPreprocess``` method. If ther server sends the ```ignore_preprocess=true``` to the proxy, the proxy will not call the ```doPreprocess``` method and execute the ```PlayerActoins```.

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
