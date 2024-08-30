#include "rpc-client/rpc-player-client.h"


RpcPlayerClient::GrpcClientPlayer()
{
    M_agent_type = protos::AgentType::PlayerT;
}

void
RpcPlayerClient::init(rcsc::PlayerAgent *agent,
                            std::string target,
                            int port,
                            bool use_same_grpc_port,
                            bool add_20_to_grpc_port_if_right_side)
{
    M_agent = agent;
    M_unum = agent->world().self().unum();
    M_team_name = agent->world().ourTeamName();
    if (add_20_to_grpc_port_if_right_side)
        if (M_agent->world().ourSide() == rcsc::SideID::RIGHT)
            port += 20;

    if (!use_same_grpc_port)
    {
        port += M_agent->world().self().unum();
    }

    this->M_target = target + ":" + std::to_string(port);
    sample_communication = Communication::Ptr(new SampleCommunication());
}