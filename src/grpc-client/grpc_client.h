#ifndef GRPC_AGENT_H
#define GRPC_AGENT_H

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <grpcpp/grpcpp.h>
#include "../grpc-generated/service.pb.h"
#include "../grpc-generated/service.grpc.pb.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/coach/coach_agent.h>
#include <rcsc/trainer/trainer_agent.h>
#include "rpc-client/rpc-client.h"


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using protos::Game;
using protos::State;
using protos::PlayerAction;
using protos::CoachAction;
using protos::TrainerAction;


class GrpcClient : public IRpcClient{
public:
    protos::AgentType M_agent_type;
    std::string M_target;
    std::shared_ptr<Channel> M_channel;
    std::unique_ptr<Game::Stub> M_stub_;
    protos::RegisterResponse * M_register_response = new protos::RegisterResponse();

    ~GrpcClient() {}
    
    void sendParams(bool offline_logging) override;
    void addDlog(protos::Log log) const;
    void sendServerParam() const;
    void sendPlayerParams() const;
    void sendPlayerType() const;
    void sendInitMessage(bool offline_logging) const;
    bool Register() override;
    void sendByeCommand() const override;
    bool connectToGrpcServer() override;

    static rcsc::ViewWidth convertViewWidth(protos::ViewWidth view_width);
    static rcsc::SideID convertSideID(protos::Side side_id);
    static rcsc::Vector2D convertVector2D(protos::RpcVector2D vector2d);
};

#endif