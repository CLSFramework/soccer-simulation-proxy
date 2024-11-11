#include "thrift_client.h"
#include "player/sample_communication.h"
#include "rpc-client/rpc-player-client.h"
#include "planner/action_chain_graph.h"

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

    void updateChainByDefault(const rcsc::WorldModel &wm);
    void updateChainByPlannerAction(const rcsc::WorldModel &wm, const soccer::PlayerAction &action);
    void getActions();
    bool GetBestPlannerAction();
    void convertResultPairToRpcActionStatePair( std::map<int32_t, soccer::RpcActionState> * pairs);
    void addSayMessage(soccer::Say sayMessage) const;
    soccer::State generateState() ;
    void addHomePosition(soccer::WorldModel * world_model) const;
    private:
    rcsc::GameTime M_state_update_time;
    soccer::State M_state;
};