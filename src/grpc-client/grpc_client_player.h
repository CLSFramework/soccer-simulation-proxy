#include "grpc_client.h"
#include "player/sample_communication.h"
#include "rpc-client/rpc-player-client.h"

class GrpcClientPlayer : public GrpcClient, public RpcPlayerClient
{
    rcsc::PlayerAgent *M_agent;
    Communication::Ptr sample_communication;

public:
    GrpcClientPlayer();

    void init(rcsc::SoccerAgent *agent,
              std::string target = "localhost",
              int port = 50051,
              bool use_same_grpc_port = true,
              bool add_20_to_grpc_port_if_right_side = false) override;

    void getActions();
    bool GetBestPlannerAction();
    void addSayMessage(protos::Say sayMessage) const;
    State generateState() const;
    void addHomePosition(protos::WorldModel *world_model) const;
};