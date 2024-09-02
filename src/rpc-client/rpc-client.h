//
// Created by nader on 8/13/24.
//

#ifndef HELIOS_BASE_RPC_CLIENT_H
#define HELIOS_BASE_RPC_CLIENT_H

#include <rcsc/player/player_agent.h>

class IRpcClient {
public:
    bool M_is_connected = false;
    int M_unum;
    std::string M_team_name;
    bool M_param_sent = false;


    virtual bool connectToGrpcServer() = 0;
    virtual void sendParams(bool offline_logging) = 0;
    virtual void getActions() = 0;
    virtual void sendByeCommand() const = 0;
    virtual bool Register() = 0;

    virtual void init(rcsc::SoccerAgent * agent,
                        std::string target="localhost",
                        int port=50051,
                        bool use_same_grpc_port=true,
                        bool add_20_to_grpc_port_if_right_side=false) = 0;

    bool isConnected() const{
        return M_is_connected;
    }


};
#endif //HELIOS_BASE_RPC_CLIENT_H
