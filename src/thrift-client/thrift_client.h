#ifndef THRIFT_AGENT_H
#define THRIFT_AGENT_H

#include <rcsc/player/player_agent.h>
#include <rcsc/coach/coach_agent.h>
#include <rcsc/trainer/trainer_agent.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "thrift-generated/Game.h"
#include "thrift-generated/soccer_service_types.h"
#include "rpc-client/rpc-client.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


class ThriftAgent : public IRpcClient{
public:
    std::string M_server_host;
    int M_server_port;
    std::shared_ptr<TTransport> M_socket;
    std::shared_ptr<TTransport> M_transport;
    std::shared_ptr<TProtocol> M_protocol;
    std::shared_ptr<soccer::GameClient> M_client;
    soccer::AgentType::type M_agent_type;
    soccer::RegisterResponse M_register_response;

    ~ThriftAgent() {}
    
    void sendParams(bool offline_logging) override;
    void addDlog(soccer::Log log) const;
    void sendServerParam() const;
    void sendPlayerParams() const;
    void sendPlayerType() const;
    void sendInitMessage(bool offline_logging) const;
    bool Register() override;
    void sendByeCommand() const override;
    bool connectToGrpcServer() override;

    static rcsc::ViewWidth convertViewWidth(soccer::ViewWidth::type view_width);
    static rcsc::SideID convertSideID(soccer::Side::type side_id);
    static rcsc::Vector2D convertVector2D(soccer::RpcVector2D vector2d);
};

#endif