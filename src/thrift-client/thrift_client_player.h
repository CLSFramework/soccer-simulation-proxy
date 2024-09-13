#include "thrift_client.h"
#include "player/sample_communication.h"
#include "rpc-client/rpc-player-client.h"

class ThriftClientPlayer : public ThriftAgent, public RpcPlayerClient {
    rcsc::PlayerAgent * M_agent;
    Communication::Ptr sample_communication;
    public:
    ThriftClientPlayer();

    void init(rcsc::SoccerAgent * agent,
              std::string target="localhost",
              int port=50051,
              bool use_same_grpc_port=true,
              bool add_20_to_grpc_port_if_right_side=false) override;

    void getActions();
    bool GetBestPlannerAction();
    void addSayMessage(soccer::Say sayMessage) const;
    soccer::State generateState() const;
    void addHomePosition(soccer::WorldModel * world_model) const;
};