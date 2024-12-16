# ChangeLog

## [1.1.3] - 2024-12-15

### Added
- 

### Fixed
-

### Changed
- Increase gRPC client deadline to 3 seconds 
- Copy additional proto files to binary directory

### Developers
- [NaderZare](https://github.com/naderzare)

=======

## [1.1.2] - 2024-12-08

### Added
- ServerParams.pitch_margin
- Player.inertia_final_point, PenaltyKickState.cycle, self.get_safety_dash_power.
- bhv_goalieFreeKick added

### Fixed
-

### Changed
- 

### Developers
- [SoroushMazloum](https://github.com/SoroushMazloum)

=======

## [1.1.1] - 2024-12-01

### Added

- added Neck_OffensiveInterceptNeck into idls
- added HeliosBasicTackle into idls
- added start-debug-agent.sh file

### Fixed

- bug fixed in start-agent.sh

### Changed

-

### Developers

- [NaderZare](https://github.com/naderzare)
  
=======

## [1.1.0] - 2024-11-17

### Added
- 

### Fixed
- 

### Changed
- If the server sends some main actions to the proxy like doForceKick, dash, smartkick, etc, the proxy will just run the first action and ignore the rest of the main actions.

### Developers
- [NaderZare](https://github.com/naderzare)
- [Sadra Khanjari](https://github.com/sk2ip)
  
=======

## [1.0.7] - 2024-11-11

### Added
- 

### Fixed
- Performance improvement
- Fixed bugs in the server side planner

### Changed
- 

### Developers
- [NaderZare](https://github.com/naderzare)

=======

## [1.0.6] - 2024-10-19

### Added
- ServerParam.center_circle_r, ServerParam.goal_post_radius, WorldModel.game_mode_side

### Fixed
-

### Changed
- 

### Developers
- [ErfanFathi](https://github.com/ErfanFathii)

=======

## [1.0.5] - 2024-10-16

### Added
- wm.time_stopped, wm.set_play_count, serverParams.goal_area_width/length

### Fixed
-

### Changed
- 

### Developers
- [NaderZare](https://github.com/naderzare)
- [SadraKhanjari](https://github.com/SK2iP)
- [SoroushMazloum](https://github.com/SoroushMazloum)

=======

## [1.0.4] - 2024-10-8

### Added
- self.effort and wm.see_time are added

### Fixed
-

### Changed
- 

### Developers 
- [SoroushMazloum](https://github.com/SoroushMazloum)


## [1.0.3] - 2024-10-7

### Added
- penalty_kick_state has been added to the proxy in the WorldModel message.

### Fixed
-

### Changed
- 

### Developers
- [SadraKhanjari](https://github.com/SK2iP)
- [SoroushMazloum](https://github.com/SoroushMazloum)


## [1.0.2] - 2024-09-15

### Added
- ignore_doforcekick and ignore_doHeardPassRecieve has been added to the proxy in the actions message.
- now users can decide to do the doForceKick and doHeardPassRecieve or not

### Fixed
- now state only generate once per cycle 

### Changed
- 

### Developers
- [NaderZare](https://github.com/naderzare)
- [SadraKhanjari](https://github.com/SK2iP)

## [1.0.1] - 2024-09-15

### Added
- catch_time has been added to the proxy in the self message.
- kickable_opponent_existance and kickable_teammate_existance has been added to the proxy in the worldmodel message.
- bhv_doforceKick action has been added as a message and to the actions message.

### Fixed
- 

### Changed
- 

### Developers
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

### Developers
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
