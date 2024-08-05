#include "grpc_agent_coach.h"
// #include "state_generator.h"

#include <rcsc/player/say_message_builder.h>
#include <rcsc/common/player_param.h>
#include "coach/sample_coach.h"
#include <chrono>
#include <rcsc/common/logger.h>
#include "grpc/state_generator.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

#define DEBUG

#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl
#define LOGV(x) std::cout << #x << ": " << x << std::endl
#else
#define LOG(x)
#define LOGV(x)
#endif

GrpcAgentCoach::GrpcAgentCoach()
{
    agent_type = soccer::AgentType::CoachT;
}

void GrpcAgentCoach::init(rcsc::CoachAgent *agent,
                          std::string target,
                          int port,
                          bool use_same_grpc_port,
                          bool add_20_to_grpc_port_if_right_side)
{
    M_agent = agent;
    if (add_20_to_grpc_port_if_right_side)
        if (M_agent->world().ourSide() == rcsc::SideID::RIGHT)
            port += 20;

    if (!use_same_grpc_port)
    {
        port += 13;
    }

    this->server_host = target;
    this->server_port = port;
}

void GrpcAgentCoach::getActions() const
{
    auto agent = M_agent;
    soccer::State state = generateState();
    state.agent_type = soccer::AgentType::CoachT;
    soccer::CoachActions actions;
    try{
        transport->open();
        client->GetCoachActions(actions, state);
    }
    catch (const std::exception &e)
    {
        transport->close();
        std::cerr << e.what() << std::endl;
        return;
    }
    transport->close();

    for (int i = 0; i < actions.actions.size(); i++)
    {
        auto action = actions.actions[i];
        if (action.__isset.change_player_types)
        {
            const auto &changePlayerTypes = action.change_player_types;
            const auto &playerType = changePlayerTypes.type;
            const auto &unum = changePlayerTypes.uniform_number;

            agent->doChangePlayerType(unum, playerType);
            break;
        }
        if (action.__isset.do_helios_say_player_types)
        {
            auto sample_coach = dynamic_cast<SampleCoach *>(agent);
            sample_coach->sayPlayerTypes();
            break;
        }
        if (action.__isset.do_helios_substitute)
        {
            auto sample_coach = dynamic_cast<SampleCoach *>(agent);
            sample_coach->doSubstitute();
            break;
        }
    }
}

soccer::State GrpcAgentCoach::generateState() const
{
    auto &wm = M_agent->world();
    soccer::WorldModel worldModel = StateGenerator::convertCoachWorldModel(wm);
    soccer::State state;
    state.world_model = worldModel;
    return state;
}