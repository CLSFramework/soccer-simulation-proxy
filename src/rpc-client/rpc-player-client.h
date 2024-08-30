#ifndef RPC_PLAYER_CLIENT_H
#define RPC_PLAYER_CLIENT_H

#include "player/sample_communication.h"
#include "rpc-client/rpc-client.h"

class RpcPlayerClient : public IRpcClient {
    rcsc::PlayerAgent * M_agent;
    Communication::Ptr sample_communication;
    public:
    GrpcClientPlayer();

    void init(rcsc::PlayerAgent * agent,
              std::string target="localhost",
              int port=50051,
              bool use_same_grpc_port=true,
              bool add_20_to_grpc_port_if_right_side=false);
};


#endif