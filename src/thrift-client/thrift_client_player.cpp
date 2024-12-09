#include "thrift_client_player.h"
#include "thrift_state_generator.h"
#include "player/basic_actions/body_go_to_point.h"
#include "player/basic_actions/body_smart_kick.h"
#include "player/basic_actions/bhv_before_kick_off.h"
#include "player/basic_actions/bhv_body_neck_to_ball.h"
#include "player/basic_actions/bhv_body_neck_to_point.h"
#include "player/basic_actions/bhv_emergency.h"
#include "player/basic_actions/bhv_go_to_point_look_ball.h"
#include "player/basic_actions/bhv_neck_body_to_ball.h"
#include "player/basic_actions/bhv_neck_body_to_point.h"
#include "player/basic_actions/bhv_scan_field.h"
#include "player/basic_actions/body_advance_ball.h"
#include "player/basic_actions/body_clear_ball.h"
#include "player/basic_actions/body_dribble.h"
#include "player/basic_actions/body_go_to_point_dodge.h"
#include "player/basic_actions/body_hold_ball.h"
#include "player/basic_actions/body_intercept.h"
#include "player/basic_actions/body_kick_one_step.h"
#include "player/basic_actions/body_stop_ball.h"
#include "player/basic_actions/body_stop_dash.h"
#include "player/basic_actions/body_tackle_to_point.h"
#include "player/basic_actions/body_turn_to_angle.h"
#include "player/basic_actions/body_turn_to_point.h"
#include "player/basic_actions/body_turn_to_ball.h"
#include "player/basic_actions/focus_move_to_point.h"
#include "player/basic_actions/focus_reset.h"
#include "player/basic_actions/neck_scan_field.h"
#include "player/basic_actions/neck_scan_players.h"
#include "player/basic_actions/neck_turn_to_ball_and_player.h"
#include "player/basic_actions/neck_turn_to_ball_or_scan.h"
#include "player/basic_actions/neck_turn_to_ball.h"
#include "player/basic_actions/neck_turn_to_goalie_or_scan.h"
#include "player/basic_actions/neck_turn_to_low_conf_teammate.h"
#include "player/basic_actions/neck_turn_to_player_or_scan.h"
#include "player/basic_actions/neck_turn_to_point.h"
#include "player/basic_actions/neck_turn_to_relative.h"
#include "player/basic_actions/view_change_width.h"
#include "player/basic_actions/view_normal.h"
#include "player/basic_actions/view_wide.h"
#include "player/basic_actions/view_synch.h"
#include "player/role_goalie.h"
#include "planner/bhv_strict_check_shoot.h"
#include "player/bhv_basic_move.h"
#include "player/setplay/bhv_set_play.h"
#include "player/bhv_penalty_kick.h"
#include "planner/action_generator.h"
#include "planner/field_evaluator.h"
#include "player/sample_field_evaluator.h"
#include "planner/actgen_cross.h"
#include "planner/actgen_direct_pass.h"
#include "planner/actgen_self_pass.h"
#include "planner/actgen_strict_check_pass.h"
#include "planner/actgen_short_dribble.h"
#include "planner/actgen_simple_dribble.h"
#include "planner/actgen_shoot.h"
#include "planner/actgen_action_chain_length_filter.h"
#include "planner/action_chain_holder.h"
#include "planner/bhv_planned_action.h"
#include "player/strategy.h"
#include "player/bhv_basic_tackle.h"
#include "player/neck_offensive_intercept_neck.h"
#include <rcsc/player/say_message_builder.h>
#include <rcsc/common/player_param.h>

#include <chrono>
#include <rcsc/common/logger.h>
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

#define DEBUG
//#define DEBUG_CLIENT_PLAYER
#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl
#define LOGV(x) std::cout << #x << ": " << x << std::endl
#else
#define LOG(x)
#define LOGV(x)
#endif

ThriftClientPlayer::ThriftClientPlayer()
{
    M_agent_type = soccer::AgentType::PlayerT;
}

void ThriftClientPlayer::init(rcsc::SoccerAgent *agent,
                             std::string target,
                             int port,
                             bool use_same_grpc_port,
                             bool add_20_to_grpc_port_if_right_side)
{
    M_agent = static_cast<rcsc::PlayerAgent *>(agent);
    M_unum = M_agent->world().self().unum();
    M_team_name = M_agent->world().ourTeamName();
    if (add_20_to_grpc_port_if_right_side)
        if (M_agent->world().ourSide() == rcsc::SideID::RIGHT)
            port += 20;

    if (!use_same_grpc_port)
    {
        port += M_agent->world().self().unum();
    }

    this->M_server_host = target;
    this->M_server_port = port;
    sample_communication = Communication::Ptr(new SampleCommunication());
}

void ThriftClientPlayer::updateChainByDefault(const rcsc::WorldModel &wm)
{
    FieldEvaluator::ConstPtr field_evaluator = FieldEvaluator::ConstPtr(new SampleFieldEvaluator);
    CompositeActionGenerator *g = new CompositeActionGenerator();
    
    g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_StrictCheckPass(), 1));
    g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_Cross(), 1));
    g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_ShortDribble(), 1));
    g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_SelfPass(), 1));

    g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_Shoot(),
                                                            2, ActGen_RangeActionChainLengthFilter::MAX));
    ActionGenerator::ConstPtr action_generator = ActionGenerator::ConstPtr(g);
    ActionChainHolder::instance().setFieldEvaluator(field_evaluator);
    ActionChainHolder::instance().setActionGenerator(action_generator);
    ActionChainHolder::instance().update(wm);
}

void ThriftClientPlayer::updateChainByPlannerAction(const rcsc::WorldModel &wm, const soccer::PlayerAction &action)
{
    FieldEvaluator::Ptr field_evaluator = FieldEvaluator::Ptr(new SampleFieldEvaluator);
    if (action.helios_offensive_planner.__isset.evaluation)
        field_evaluator->set_thrift_evalution_method(action.helios_offensive_planner.evaluation);
    CompositeActionGenerator *g = new CompositeActionGenerator();

    if (action.helios_offensive_planner.max_depth > 0)
        g->max_depth = action.helios_offensive_planner.max_depth;
    if (action.helios_offensive_planner.max_nodes > 0)
        g->max_nodes = action.helios_offensive_planner.max_nodes;

    if (action.helios_offensive_planner.lead_pass
        || action.helios_offensive_planner.direct_pass || action.helios_offensive_planner.through_pass)
        g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_StrictCheckPass(), 1));
    if (action.helios_offensive_planner.cross)
        g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_Cross(), 1));
    if (action.helios_offensive_planner.simple_pass)
        g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_DirectPass(),
                                                                2, ActGen_RangeActionChainLengthFilter::MAX));
    if (action.helios_offensive_planner.short_dribble)
        g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_ShortDribble(), 1));
    if (action.helios_offensive_planner.long_dribble)
        g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_SelfPass(), 1));
    if (action.helios_offensive_planner.simple_dribble)
        g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_SimpleDribble(),
                                                                2, ActGen_RangeActionChainLengthFilter::MAX));
    if (action.helios_offensive_planner.simple_shoot)
        g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_Shoot(),
                                                                2, ActGen_RangeActionChainLengthFilter::MAX));
    if (g->M_generators.empty())
    {
        return;
    }
    ActionGenerator::ConstPtr action_generator = ActionGenerator::ConstPtr(g);
    ActionChainHolder::instance().setFieldEvaluator(field_evaluator);
    ActionChainHolder::instance().setActionGenerator(action_generator);
    ActionChainHolder::instance().update(wm);
}

void ThriftClientPlayer::getActions()
{
    auto agent = M_agent;
    bool pre_process = checkPreprocess(agent);
    bool do_forceKick = checkdoForceKick(agent);
    bool do_heardPassReceive = checkdoHeardPassReceive(agent);
    soccer::State state = generateState();
    soccer::PlayerActions actions;
    soccer::RegisterResponse response = M_register_response;
    state.need_preprocess = pre_process;
    state.register_response = response;
    try
    {
        M_client->GetPlayerActions(actions, state);
    }
    catch(const std::exception& e){
        std::cout << e.what() << '\n';
        M_is_connected = false;
        return;
    }
    if (pre_process && !actions.ignore_preprocess)
    {
        if (doPreprocess(agent)){
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": preprocess done" );
            return;
        }
    }

        const rcsc::WorldModel & wm = agent->world();

    if ( !actions.ignore_shootInPreprocess )
    {
        if ( wm.gameMode().type() != rcsc::GameMode::IndFreeKick_
            && wm.time().stopped() == 0
            && wm.self().isKickable()
            && Bhv_StrictCheckShoot().execute( agent ) )
        {
            // reset intention
            agent->setIntention( static_cast< rcsc::SoccerIntention * >( 0 ) );
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": shoot in preprocess done" );
            return;
        }
    }
    
    if ( !actions.ignore_doIntention )
    {
        if ( agent->doIntention() )
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doIntention performed" );
            return;
        }
    }

    if(do_forceKick && !actions.ignore_doforcekick)
    {
        if(doForceKick(agent)){
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": doForceKick performed" );
            return;
        }
    }
    if(do_heardPassReceive && !actions.ignore_doHeardPassRecieve)
    {
        if(doHeardPassReceive(agent)){
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": doHeardPassReceive performed" );
            return;
        }
    }

    int planner_action_index = -1;
    for (int i = 0; i < actions.actions.size(); i++)
    {
        auto action = actions.actions[i];
        if (action.__isset.helios_offensive_planner)
        {
            planner_action_index = i;
            break;
        }
    }

    if (planner_action_index != -1)
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": updateChainByPlannerAction" );
        updateChainByPlannerAction(wm, actions.actions[planner_action_index]);
    }
    else
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": updateChainByDefault" );
        updateChainByDefault(wm);
    }
    
    bool action_performed = false;
    for (int i = 0; i < actions.actions.size(); i++)
    {
        auto action = actions.actions[i];
        
        if (action.__isset.dash && !action_performed)
        {
            if (agent->doDash(action.dash.power, action.dash.relative_direction))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doDash performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doDash failed" );
            }
        }
        else if (action.__isset.kick && !action_performed)
        {
            if (agent->doKick(action.kick.power, action.kick.relative_direction))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doKick performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doKick failed" );
            }
        }
        else if (action.__isset.turn && !action_performed)
        {
            if (agent->doTurn(action.turn.relative_direction))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doTurn performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doTurn failed" );
            }
        }
        else if (action.__isset.tackle && !action_performed)
        {
            if (agent->doTackle(action.tackle.power_or_dir, action.tackle.foul))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doTackle performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doTackle failed" );
            }
        }
        else if (action.__isset.catch_action && !action_performed)
        {
            if (agent->doCatch())
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doCatch performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doCatch failed" );
            }
        }
        else if (action.__isset.move && !action_performed)
        {
            if (agent->doMove(action.move.x, action.move.y))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doMove performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doMove failed" );
            }
        }
        else if (action.__isset.turn_neck)
        {
            if (agent->doTurnNeck(action.turn_neck.moment))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doTurnNeck performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": doTurnNeck failed" );
            }
        }
        else if (action.__isset.change_view)
        {
            const rcsc::ViewWidth view_width = ThriftAgent::convertViewWidth(action.change_view.view_width);
            if (agent->doChangeView(view_width))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doChangeView performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doChangeView failed" );
            }
        }
        else if (action.__isset.say)
        {
            addSayMessage(action.say);
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": addSayMessage called" );
        }
        else if (action.__isset.point_to)
        {
            if (agent->doPointto(action.point_to.x, action.point_to.y))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doPointto performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doPointto failed" );
            }
        }
        else if (action.__isset.point_to_of)
        {
            if (agent->doPointtoOff())
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doPointtoOff performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doPointtoOff failed" );
            }
        }
        else if (action.__isset.attention_to)
        {
            const rcsc::SideID side = ThriftAgent::convertSideID(action.attention_to.side);
            if (agent->doAttentionto(side, action.attention_to.unum))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doAttentionto performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__ ": doAttentionto failed" );
            }
        }
        else if (action.__isset.attention_to_of)
        {
            if (agent->doAttentiontoOff())
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doAttentiontoOff performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doAttentiontoOff failed" );
            }
        }
        else if (action.__isset.log)
        {
            addDlog(action.log);
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": addDlog called" );
        }
        else if (action.__isset.body_go_to_point && !action_performed)
        {
            const auto &bodyGoToPoint = action.body_go_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyGoToPoint.target_point);
            if (Body_GoToPoint(targetPoint, bodyGoToPoint.distance_threshold, bodyGoToPoint.max_dash_power).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_GoToPoint performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_GoToPoint failed" );
            }
        }
        else if (action.__isset.body_smart_kick && !action_performed)
        {
            const auto &bodySmartKick = action.body_smart_kick;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodySmartKick.target_point);
            if (Body_SmartKick(targetPoint, bodySmartKick.first_speed, bodySmartKick.first_speed_threshold,
                               bodySmartKick.max_steps).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_SmartKick performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_SmartKick failed" );
            }
        }
        else if (action.__isset.bhv_before_kick_off && !action_performed)
        {
            const auto &bhvBeforeKickOff = action.bhv_before_kick_off;
            const auto &point = ThriftAgent::convertVector2D(bhvBeforeKickOff.point);
            if (Bhv_BeforeKickOff(point).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BeforeKickOff performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BeforeKickOff failed" );
            }
        }
        else if (action.__isset.bhv_body_neck_to_ball && !action_performed)
        {
            if (Bhv_BodyNeckToBall().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BodyNeckToBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BodyNeckToBall failed" );
            }
        }
        else if (action.__isset.bhv_body_neck_to_point && !action_performed)
        {
            const auto &bhvBodyNeckToPoint = action.bhv_body_neck_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bhvBodyNeckToPoint.point);
            if (Bhv_BodyNeckToPoint(targetPoint).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BodyNeckToPoint performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BodyNeckToPoint failed" );
            }
        }
        else if (action.__isset.bhv_emergency)
        {
            if (Bhv_Emergency().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_Emergency performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_Emergency failed" );
            }
        }
        else if (action.__isset.bhv_go_to_point_look_ball && !action_performed)
        {
            const auto &bhvGoToPointLookBall = action.bhv_go_to_point_look_ball;
            const auto &targetPoint = ThriftAgent::convertVector2D(bhvGoToPointLookBall.target_point);
            if (Bhv_GoToPointLookBall(targetPoint, bhvGoToPointLookBall.distance_threshold, bhvGoToPointLookBall.max_dash_power).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_GoToPointLookBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_GoToPointLookBall failed" );
            }                                                               
        }
        else if (action.__isset.bhv_neck_body_to_ball && !action_performed)
        {
            const auto &bhvNeckBodyToBall = action.bhv_neck_body_to_ball;
            if (Bhv_NeckBodyToBall(bhvNeckBodyToBall.angle_buf).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_NeckBodyToBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_NeckBodyToBall failed" );
            }                                                                 
        }
        else if (action.__isset.bhv_neck_body_to_point && !action_performed)
        {
            const auto &bhvNeckBodyToPoint = action.bhv_neck_body_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bhvNeckBodyToPoint.point);
            if (Bhv_NeckBodyToPoint(targetPoint, bhvNeckBodyToPoint.angle_buf).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_NeckBodyToPoint performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_NeckBodyToPoint failed" );
            }                                                                      
        }
        else if (action.__isset.bhv_scan_field && !action_performed)
        {
            if (Bhv_ScanField().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_ScanField performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_ScanField failed" );
            }                                                                      
        }
        else if (action.__isset.body_advance_ball && !action_performed)
        {
            if (Body_AdvanceBall().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_AdvanceBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_AdvanceBall failed" );
            }                                                                      
        }
        else if (action.__isset.body_clear_ball && !action_performed)
        {
            if (Body_ClearBall().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_ClearBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_ClearBall failed" );
            }                                                                      
        }
        else if (action.__isset.body_dribble && !action_performed)
        {
            const auto &bodyDribble = action.body_dribble;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyDribble.target_point);
            if (Body_Dribble(targetPoint, bodyDribble.distance_threshold, bodyDribble.dash_power,bodyDribble.dash_count,bodyDribble.dodge).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_Dribble performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_Dribble failed" );
            }                                                                      
        }
        else if (action.__isset.body_go_to_point_dodge && !action_performed)
        {
            const auto &bodyGoToPointDodge = action.body_go_to_point_dodge;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyGoToPointDodge.target_point);
            if (Body_GoToPointDodge(targetPoint, bodyGoToPointDodge.dash_power).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_GoToPointDodge performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_GoToPointDodge failed" );
            }                                                                      
        }
        else if (action.__isset.body_hold_ball && !action_performed)
        {
            const auto &bodyHoldBall = action.body_hold_ball;
            const auto &turnTargetPoint = ThriftAgent::convertVector2D(bodyHoldBall.turn_target_point);
            const auto &kickTargetPoint = ThriftAgent::convertVector2D(bodyHoldBall.kick_target_point);
            if (Body_HoldBall(bodyHoldBall.do_turn, turnTargetPoint, kickTargetPoint).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_HoldBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_HoldBall failed" );
            }                                                                      
        }
        else if (action.__isset.body_intercept && !action_performed)
        {
            const auto &bodyIntercept = action.body_intercept;
            const auto &facePoint = ThriftAgent::convertVector2D(bodyIntercept.face_point);
            if (Body_Intercept(bodyIntercept.save_recovery, facePoint).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_Intercept performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_Intercept failed" );
            }                                                                      
        }
        else if (action.__isset.body_kick_one_step && !action_performed)
        {
            const auto &bodyKickOneStep = action.body_kick_one_step;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyKickOneStep.target_point);
            if (Body_KickOneStep(targetPoint, bodyKickOneStep.first_speed, bodyKickOneStep.force_mode).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_KickOneStep performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_KickOneStep failed" );
            }                                                                      
        }
        else if (action.__isset.body_stop_ball && !action_performed)
        {
            if (Body_StopBall().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_StopBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_StopBall failed" );
            }                                                                      
        }
        else if (action.__isset.body_stop_dash && !action_performed)
        {
            const auto &bodyStopDash = action.body_stop_dash;
            if (Body_StopDash(bodyStopDash.save_recovery).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_StopDash performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_StopDash failed" );
            }
        }
        else if (action.__isset.body_tackle_to_point && !action_performed)
        {
            const auto &bodyTackleToPoint = action.body_tackle_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyTackleToPoint.target_point);
            if (Body_TackleToPoint(targetPoint, bodyTackleToPoint.min_probability, bodyTackleToPoint.min_speed).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TackleToPoint performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TackleToPoint failed" );
            }                                                                      
        }
        else if (action.__isset.body_turn_to_angle && !action_performed)
        {
            const auto &bodyTurnToAngle = action.body_turn_to_angle;
            if (Body_TurnToAngle(bodyTurnToAngle.angle).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TurnToAngle performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TurnToAngle failed" );
            }                                                                      
        }
        else if (action.__isset.body_turn_to_ball && !action_performed)
        {
            const auto &bodyTurnToBall = action.body_turn_to_ball;
            if (Body_TurnToBall(bodyTurnToBall.cycle).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TurnToBall performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TurnToBall failed" );
            }                                                                      
        }
        else if (action.__isset.body_turn_to_point && !action_performed)
        {
            const auto &bodyTurnToPoint = action.body_turn_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyTurnToPoint.target_point);
            if (Body_TurnToPoint(targetPoint, bodyTurnToPoint.cycle).execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TurnToPoint performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Body_TurnToPoint failed" );
            }
        }
        
        else if (action.__isset.focus_move_to_point)
        {
            const auto &focusMoveToPoint = action.focus_move_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(focusMoveToPoint.target_point);
            if (rcsc::Focus_MoveToPoint(targetPoint).execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Focus_MoveToPoint performed" );                                             
            }            
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Focus_MoveToPoint failed" );                                             
            }                               
        }
        else if (action.__isset.focus_reset)
        {
            if (rcsc::Focus_Reset().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Focus_Reset performed" );                                             
            }            
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Focus_Reset failed" );                                             
            }                                            
        }
        else if (action.__isset.neck_scan_field)
        {
            if (Neck_ScanField().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": Neck_ScanField performed" );
            }  
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_ScanField failed" );
            }
        }
        else if (action.__isset.neck_scan_players)
        {
            if (Neck_ScanPlayers().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_ScanPlayers performed" );
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_ScanPlayers failed" );
            }                                          
        }
        else if (action.__isset.neck_turn_to_ball_and_player)
        {
            const auto &neckTurnToBallAndPlayer = action.neck_turn_to_ball_and_player;
            const rcsc::AbstractPlayerObject *player = nullptr;
            if (neckTurnToBallAndPlayer.side == soccer::Side::LEFT && agent->world().ourSide() == rcsc::SideID::LEFT)
            {
                player = agent->world().ourPlayer(neckTurnToBallAndPlayer.uniform_number);
            }
            else
            {
                player = agent->world().theirPlayer(neckTurnToBallAndPlayer.uniform_number);
            }
            if (player != nullptr)
            {
                if (Neck_TurnToBallAndPlayer(
                    player,
                    neckTurnToBallAndPlayer.count_threshold).execute(agent))
                {
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": Neck_TurnToBallAndPlayer performed" );                                              
                }
                else 
                {
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": Neck_TurnToBallAndPlayer failed" );
                }                                            
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM, __FILE__": Neck_TurnToBallAndPlayer failed (no player)" );
            }
            
        }
        else if (action.__isset.neck_turn_to_ball_or_scan)
        {
            const auto &neckTurnToBallOrScan = action.neck_turn_to_ball_or_scan;
            if (Neck_TurnToBallOrScan(
                neckTurnToBallOrScan.count_threshold)
                .execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToBallOrScan performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToBallOrScan failed" );
            }                                           
        }
        else if (action.__isset.neck_turn_to_ball)
        {
            if (Neck_TurnToBall().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToBall performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToBall failed" );
            }                                             
        }
        else if (action.__isset.neck_turn_to_goalie_or_scan)
        {
            const auto &neckTurnToGoalieOrScan = action.neck_turn_to_goalie_or_scan;
            if (Neck_TurnToGoalieOrScan(
                neckTurnToGoalieOrScan.count_threshold)
                .execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToGoalieOrScan performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToGoalieOrScan failed" );
            }                                           
        }
        else if (action.__isset.neck_turn_to_low_conf_teammate)
        {
            const auto &neckTurnToLowConfTeammate = action.neck_turn_to_low_conf_teammate;
            if (Neck_TurnToLowConfTeammate().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToLowConfTeammate performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToLowConfTeammate failed" );
            }                                            
        }
        else if (action.__isset.neck_turn_to_player_or_scan)
        {
            const auto &neckTurnToPlayerOrScan = action.neck_turn_to_player_or_scan;
            const rcsc::AbstractPlayerObject *player = nullptr;
            if (neckTurnToPlayerOrScan.side == soccer::Side::LEFT && agent->world().ourSide() == rcsc::SideID::LEFT)
            {
                player = agent->world().ourPlayer(neckTurnToPlayerOrScan.uniform_number);
            }
            else
            {
                player = agent->world().theirPlayer(neckTurnToPlayerOrScan.uniform_number);
            }
            if (player != nullptr)
            {
                if (Neck_TurnToPlayerOrScan(
                    player,
                    neckTurnToPlayerOrScan.count_threshold)
                    .execute(agent))
                {
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": Neck_TurnToPlayerOrScan performed" );                                              
                }
                else 
                {
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        __FILE__": Neck_TurnToPlayerOrScan failed" );
                }                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM, __FILE__": Neck_TurnToPlayerOrScan failed (no player)" );
            }
        }
        else if (action.__isset.neck_turn_to_point)
        {
            const auto &neckTurnToPoint = action.neck_turn_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(neckTurnToPoint.target_point);
            if (Neck_TurnToPoint(
                targetPoint)
                .execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToPoint performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToPoint failed" );
            }                                 
        }
        else if (action.__isset.neck_turn_to_relative)
        {
            const auto &neckTurnToRelative = action.neck_turn_to_relative;
            if (Neck_TurnToRelative(
                neckTurnToRelative.angle)
                .execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToRelative performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Neck_TurnToRelative failed" );
            }                                  
        }
        else if (action.__isset.neck_offensive_intercept_neck) {
            if (Neck_OffensiveInterceptNeck().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM, __FILE__": Neck_OffensiveInterceptNeck performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM, __FILE__": Neck_OffensiveInterceptNeck failed" );
            }
        }
        else if (action.__isset.view_change_width)
        {
            const auto &viewChangeWidth = action.view_change_width;
            const rcsc::ViewWidth view_width = ThriftAgent::convertViewWidth(viewChangeWidth.view_width);
            if (View_ChangeWidth(view_width).execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_ChangeWidth performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_ChangeWidth failed" );
            }                 

        }
        else if (action.__isset.view_normal)
        {
            if(View_Normal().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_Normal performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_Normal failed" );
            }
        }
        else if (action.__isset.view_wide)
        {
            if(View_Wide().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_Wide performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_Wide failed" );
            }
        }
        else if (action.__isset.view_synch)
        {
            if(View_Synch().execute(agent))
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_Synch performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": View_Synch failed" );
            }
        }
        else if (action.__isset.helios_goalie && !action_performed)
        {
            RoleGoalie roleGoalie = RoleGoalie();
            if(roleGoalie.execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": RoleGoalie performed" );                                              
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": RoleGoalie failed" );
            }
        }
        else if (action.__isset.helios_goalie_move && !action_performed)
        {
            RoleGoalie roleGoalie = RoleGoalie();
            if(roleGoalie.doMove(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": RoleGoalie doMove performed" );
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": RoleGoalie doMove failed" );
            }
        }
        else if (action.__isset.helios_goalie_kick && !action_performed)
        {
            RoleGoalie roleGoalie = RoleGoalie();
            if (roleGoalie.doKick(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": RoleGoalie doKick performed" );
            }
            else 
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": RoleGoalie doKick failed" );
            }
        }
        else if (action.__isset.helios_shoot && !action_performed)
        {
            const rcsc::WorldModel &wm = agent->world();

            if (wm.gameMode().type() != rcsc::GameMode::IndFreeKick_
                && wm.time().stopped() == 0 && wm.self().isKickable() && Bhv_StrictCheckShoot().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Bhv_StrictCheckShoot performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_StrictCheckShoot failed" );
            }
        }
        else if (action.__isset.helios_basic_move && !action_performed)
        {
            if(Bhv_BasicMove().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Bhv_BasicMove performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BasicMove failed" );
            }
        }
        else if (action.__isset.helios_set_play && !action_performed)
        {
            if(Bhv_SetPlay().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                    __FILE__": Bhv_SetPlay performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_SetPlay failed" );
            }
        }
        else if (action.__isset.helios_penalty && !action_performed)
        {
            if(Bhv_PenaltyKick().execute(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_PenaltyKick performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_PenaltyKick failed" );
            }
        }
        else if (action.__isset.helios_communication)
        {
            if (sample_communication->execute(agent)) {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": sample_communication performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": sample_communication failed" );
            }
        }
        else if (action.__isset.helios_basic_tackle && !action_performed) {
            const auto &helios_basic_tackle = action.helios_basic_tackle;
            if (Bhv_BasicTackle( helios_basic_tackle.min_prob, helios_basic_tackle.body_thr ).execute(agent)) {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BasicTackle performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Bhv_BasicTackle failed" );
            }
        }
        else if (action.__isset.bhv_do_force_kick && !action_performed)
        {
            if(doForceKick(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doForceKick performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doForceKick failed" );
            }
        }
        else if (action.__isset.bhv_do_heard_pass_recieve && !action_performed)
        {
            if(doHeardPassReceive(agent))
            {
                action_performed = true;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doHeardPassReceive performed" );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": doHeardPassReceive failed" );
            }
        }
        else if (action.__isset.helios_offensive_planner && !action_performed)
        {
            if (action.helios_offensive_planner.server_side_decision)
            {
                if (GetBestPlannerAction())
                {
                    action_performed = true;
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                          __FILE__": GetBestPlannerAction performed" );
                }
                else 
                {
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                          __FILE__": GetBestPlannerAction failed" );
                }
            }
            else
            {
                if (Bhv_PlannedAction().execute(agent))
                {
                    action_performed = true;
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                          __FILE__": Bhv_PlannedAction performed" );
                }
                else
                {
                    if (Body_HoldBall().execute(agent))
                    {
                        action_performed = true;
                        agent->setNeckAction(new Neck_ScanField());
                        rcsc::dlog.addText( rcsc::Logger::TEAM,
                          __FILE__": Bhv_PlannedAction failed (Hold Ball performed)" );
                    }
                    else
                    {
                        rcsc::dlog.addText( rcsc::Logger::TEAM,
                          __FILE__": Bhv_PlannedAction failed (Hold Ball Failed)" );
                    }
                }
            }
        }
        else 
        {
            #ifdef DEBUG_CLIENT_PLAYER
            std::cout << "Unkown action"<<std::endl;
            #endif

            rcsc::dlog.addText( rcsc::Logger::TEAM,
                      __FILE__": Unkown action or another main action performed");
        }
    }
}


bool ThriftClientPlayer::GetBestPlannerAction()
{
    soccer::BestPlannerActionRequest action_state_pairs =  soccer::BestPlannerActionRequest();
    soccer::RegisterResponse  response =  M_register_response;
    action_state_pairs.register_response = response;

    soccer::State state = generateState();
    state.register_response = M_register_response;
    action_state_pairs.state = state;
    #ifdef DEBUG_CLIENT_PLAYER
    std::cout << "GetBestActionStatePair:" << "c" << M_agent->world().time().cycle() << std::endl;
    #endif
    convertResultPairToRpcActionStatePair(&action_state_pairs.pairs);
    #ifdef DEBUG_CLIENT_PLAYER
    std::cout << "map size:" << action_state_pairs.pairs.size() << std::endl;
    #endif

    soccer::BestPlannerActionResponse best_action;
    try
    {
        M_client->GetBestPlannerAction(best_action, action_state_pairs);
    }
    catch(const std::exception& e){
        std::cout << e.what() << '\n';
        M_is_connected = false;
        return false;
    }
    auto agent = M_agent;

    ActionChainHolder::instance().updateBestChain(best_action.index);

    if (Bhv_PlannedAction().execute(agent))
    {
        #ifdef DEBUG_CLIENT_PLAYER
        std::cout << "PlannedAction = "<< best_action.index  << std::endl;
        #endif
        agent->debugClient().addMessage("PlannedAction");
        return true;
    }

    return false;
}


void ThriftClientPlayer::addSayMessage(soccer::Say sayMessage) const
{
    auto agent = M_agent;

    if (sayMessage.__isset.ball_message)
    {
        const auto &ballMessage = sayMessage.ball_message;
        const auto &ballPosition = ThriftAgent::convertVector2D(ballMessage.ball_position);
        const auto &ballVelocity = ThriftAgent::convertVector2D(ballMessage.ball_velocity);
        agent->addSayMessage(new rcsc::BallMessage(ballPosition, ballVelocity));
    }
    if (sayMessage.__isset.pass_message)
    {
        const auto &passMessage = sayMessage.pass_message;
        const auto &receiverPoint = ThriftAgent::convertVector2D(passMessage.receiver_point);
        const auto &ballPosition = ThriftAgent::convertVector2D(passMessage.ball_position);
        const auto &ballVelocity = ThriftAgent::convertVector2D(passMessage.ball_velocity);
        agent->addSayMessage(new rcsc::PassMessage(passMessage.receiver_uniform_number,
                                                   receiverPoint,
                                                   ballPosition,
                                                   ballVelocity));
    }
    if (sayMessage.__isset.intercept_message)
    {
        const auto &interceptMessage = sayMessage.intercept_message;
        agent->addSayMessage(new rcsc::InterceptMessage(interceptMessage.our,
                                                        interceptMessage.uniform_number,
                                                        interceptMessage.cycle));
    }
    if (sayMessage.__isset.goalie_message)
    {
        const auto &goalieMessage = sayMessage.goalie_message;
        const auto &goaliePosition = ThriftAgent::convertVector2D(goalieMessage.goalie_position);
        agent->addSayMessage(new rcsc::GoalieMessage(goalieMessage.goalie_uniform_number,
                                                     goaliePosition,
                                                     goalieMessage.goalie_body_direction));
    }
    if (sayMessage.__isset.goalie_and_player_message)
    {
        const auto &goalieAndPlayerMessage = sayMessage.goalie_and_player_message;
        const auto &goaliePosition = ThriftAgent::convertVector2D(goalieAndPlayerMessage.goalie_position);
        const auto &playerPosition = ThriftAgent::convertVector2D(goalieAndPlayerMessage.player_position);
        agent->addSayMessage(new rcsc::GoalieAndPlayerMessage(goalieAndPlayerMessage.goalie_uniform_number,
                                                              goaliePosition,
                                                              goalieAndPlayerMessage.goalie_body_direction,
                                                              goalieAndPlayerMessage.player_uniform_number,
                                                              playerPosition));
    }
    if (sayMessage.__isset.offside_line_message)
    {
        const auto &offsideLineMessage = sayMessage.offside_line_message;
        agent->addSayMessage(new rcsc::OffsideLineMessage(offsideLineMessage.offside_line_x));
    }
    if (sayMessage.__isset.defense_line_message)
    {
        const auto &defenseLineMessage = sayMessage.defense_line_message;
        agent->addSayMessage(new rcsc::DefenseLineMessage(defenseLineMessage.defense_line_x));
    }
    if (sayMessage.__isset.wait_request_message)
    {
        const auto &waitRequestMessage = sayMessage.wait_request_message;
        agent->addSayMessage(new rcsc::WaitRequestMessage());
    }
    if (sayMessage.__isset.setplay_message)
    {
        const auto &setplayMessage = sayMessage.setplay_message;
        agent->addSayMessage(new rcsc::SetplayMessage(setplayMessage.wait_step));
    }
    if (sayMessage.__isset.pass_request_message)
    {
        const auto &passRequestMessage = sayMessage.pass_request_message;
        const auto &targetPoint = ThriftAgent::convertVector2D(passRequestMessage.target_point);
        agent->addSayMessage(new rcsc::PassRequestMessage(targetPoint));
    }
    if (sayMessage.__isset.stamina_message)
    {
        const auto &staminaMessage = sayMessage.stamina_message;
        agent->addSayMessage(new rcsc::StaminaMessage(staminaMessage.stamina));
    }
    if (sayMessage.__isset.recovery_message)
    {
        const auto &recoveryMessage = sayMessage.recovery_message;
        agent->addSayMessage(new rcsc::RecoveryMessage(recoveryMessage.recovery));
    }
    if (sayMessage.__isset.stamina_capacity_message)
    {
        const auto &staminaCapacityMessage = sayMessage.stamina_capacity_message;
        agent->addSayMessage(new rcsc::StaminaCapacityMessage(staminaCapacityMessage.stamina_capacity));
    }
    if (sayMessage.__isset.dribble_message)
    {
        const auto &dribbleMessage = sayMessage.dribble_message;
        const auto &targetPoint = ThriftAgent::convertVector2D(dribbleMessage.target_point);
        agent->addSayMessage(new rcsc::DribbleMessage(targetPoint, dribbleMessage.queue_count));
    }
    if (sayMessage.__isset.ball_goalie_message)
    {
        const auto &ballGoalieMessage = sayMessage.ball_goalie_message;
        const auto &ballPosition = ThriftAgent::convertVector2D(ballGoalieMessage.ball_position);
        const auto &ballVelocity = ThriftAgent::convertVector2D(ballGoalieMessage.ball_velocity);
        const auto &goaliePosition = ThriftAgent::convertVector2D(ballGoalieMessage.goalie_position);
        agent->addSayMessage(new rcsc::BallGoalieMessage(ballPosition, ballVelocity, goaliePosition, ballGoalieMessage.goalie_body_direction));
    }
    if (sayMessage.__isset.one_player_message)
    {
        const auto &onePlayerMessage = sayMessage.one_player_message;
        const auto &playerPosition = ThriftAgent::convertVector2D(onePlayerMessage.position);
        agent->addSayMessage(new rcsc::OnePlayerMessage(onePlayerMessage.uniform_number, playerPosition));
    }
    if (sayMessage.__isset.two_player_message)
    {
        const auto &twoPlayersMessage = sayMessage.two_player_message;
        const auto &player1Position = ThriftAgent::convertVector2D(twoPlayersMessage.first_position);
        const auto &player2Position = ThriftAgent::convertVector2D(twoPlayersMessage.second_position);
        agent->addSayMessage(new rcsc::TwoPlayerMessage(twoPlayersMessage.first_uniform_number,
                                                        player1Position,
                                                        twoPlayersMessage.second_uniform_number,
                                                        player2Position));
    }
    if (sayMessage.__isset.three_player_message)
    {
        const auto &threePlayersMessage = sayMessage.three_player_message;
        const auto &player1Position = ThriftAgent::convertVector2D(threePlayersMessage.first_position);
        const auto &player2Position = ThriftAgent::convertVector2D(threePlayersMessage.second_position);
        const auto &player3Position = ThriftAgent::convertVector2D(threePlayersMessage.third_position);
        agent->addSayMessage(new rcsc::ThreePlayerMessage(threePlayersMessage.first_uniform_number,
                                                          player1Position,
                                                          threePlayersMessage.second_uniform_number,
                                                          player2Position,
                                                          threePlayersMessage.third_uniform_number,
                                                          player3Position));
    }
    if (sayMessage.__isset.self_message)
    {
        const auto &selfMessage = sayMessage.self_message;
        const auto &selfPosition = ThriftAgent::convertVector2D(selfMessage.self_position);
        agent->addSayMessage(new rcsc::SelfMessage(selfPosition, selfMessage.self_body_direction, selfMessage.self_stamina));
    }
    if (sayMessage.__isset.teammate_message)
    {
        const auto &teammateMessage = sayMessage.teammate_message;
        const auto &teammatePosition = ThriftAgent::convertVector2D(teammateMessage.position);
        agent->addSayMessage(new rcsc::TeammateMessage(teammateMessage.uniform_number, teammatePosition, teammateMessage.body_direction));
    }
    if (sayMessage.__isset.opponent_message)
    {
        const auto &opponentMessage = sayMessage.opponent_message;
        const auto &opponentPosition = ThriftAgent::convertVector2D(opponentMessage.position);
        agent->addSayMessage(new rcsc::OpponentMessage(opponentMessage.uniform_number, opponentPosition, opponentMessage.body_direction));
    }
    if (sayMessage.__isset.ball_player_message)
    {
        const auto &ballPlayerMessage = sayMessage.ball_player_message;
        const auto &ballPosition = ThriftAgent::convertVector2D(ballPlayerMessage.ball_position);
        const auto &ballVelocity = ThriftAgent::convertVector2D(ballPlayerMessage.ball_velocity);
        const auto &playerPosition = ThriftAgent::convertVector2D(ballPlayerMessage.player_position);
        agent->addSayMessage(new rcsc::BallPlayerMessage(ballPosition, ballVelocity, ballPlayerMessage.uniform_number, playerPosition, ballPlayerMessage.body_direction));
    }
}
//
soccer::State ThriftClientPlayer::generateState() 
{
    const rcsc::WorldModel &wm = M_agent->world();
    if (M_state_update_time == wm.time())
    {
        return M_state;
    }
    M_state_update_time = wm.time();
    soccer::WorldModel worldModel = ThriftStateGenerator::convertWorldModel(wm);
    addHomePosition(&worldModel);
    soccer::State state;
    state.world_model = worldModel;
    M_state = state;
    return M_state;
}

void ThriftClientPlayer::addHomePosition(soccer::WorldModel *res) const
{
    for (int i = 1; i < 12; i++)
    {
        auto home_pos = Strategy::i().getPosition(i);
        auto vec_msg = soccer::RpcVector2D();
        vec_msg.x = home_pos.x;
        vec_msg.y = home_pos.y;
        res->helios_home_positions[i] = vec_msg;
    }
}

void ThriftClientPlayer::convertResultPairToRpcActionStatePair( std::map<int32_t, soccer::RpcActionState> * pairs)
{
    for (auto & index_resultPair : ActionChainHolder::instance().graph().getAllResults())
    {
        try
        {

            auto & result_pair = index_resultPair.second;
            auto action_ptr = result_pair.first->actionPtr();
            auto state_ptr = result_pair.first->statePtr();
            int unique_index = action_ptr->uniqueIndex();
            int parent_index = action_ptr->parentIndex();
            auto eval = result_pair.second;
            auto map = pairs;
            auto rpc_action_state_pair = soccer::RpcActionState();
            auto rpc_cooperative_action = soccer::RpcCooperativeAction();
            auto rpc_predict_state = soccer::RpcPredictState();
            auto category = soccer::RpcActionCategory::AC_Hold;
            
            switch (action_ptr->category())
            {
            case CooperativeAction::Hold:
                category = soccer::RpcActionCategory::AC_Hold;
                break;
            case CooperativeAction::Dribble:
                category = soccer::RpcActionCategory::AC_Dribble;
                break;
            case CooperativeAction::Pass:
                category = soccer::RpcActionCategory::AC_Pass;
                break;
            case CooperativeAction::Shoot :
                category = soccer::RpcActionCategory::AC_Shoot;
                break;
            case CooperativeAction::Clear:
                category = soccer::RpcActionCategory::AC_Clear;
                break;
            case CooperativeAction::Move:
                category = soccer::RpcActionCategory::AC_Move;
                break;            
            case CooperativeAction::NoAction:
                category = soccer::RpcActionCategory::AC_NoAction;
                break;
            default:
                break;
            }
            rpc_cooperative_action.category = category;
            rpc_cooperative_action.index = unique_index;
            rpc_cooperative_action.sender_unum = action_ptr->playerUnum();
            rpc_cooperative_action.target_unum = action_ptr->targetPlayerUnum();
            rpc_cooperative_action.target_point = ThriftStateGenerator::convertVector2D(action_ptr->targetPoint());
            rpc_cooperative_action.first_ball_speed = action_ptr->firstBallSpeed();
            rpc_cooperative_action.first_turn_moment = action_ptr->firstTurnMoment();
            rpc_cooperative_action.first_dash_power = action_ptr->firstDashPower();
            rpc_cooperative_action.first_dash_angle_relative = action_ptr->firstDashAngle().degree();
            rpc_cooperative_action.duration_step = action_ptr->durationStep();
            rpc_cooperative_action.kick_count = action_ptr->kickCount();
            rpc_cooperative_action.turn_count = action_ptr->turnCount();
            rpc_cooperative_action.dash_count = action_ptr->dashCount();
            rpc_cooperative_action.final_action = action_ptr->isFinalAction();
            rpc_cooperative_action.description = action_ptr->description();
            rpc_cooperative_action.parent_index = parent_index;

            // RpcPredictState
            rpc_predict_state.spend_time = state_ptr->spendTime();
            rpc_predict_state.ball_holder_unum = state_ptr->ballHolderUnum();
            rpc_predict_state.ball_position = ThriftStateGenerator::convertVector2D(state_ptr->ball().pos());
            rpc_predict_state.ball_velocity = ThriftStateGenerator::convertVector2D(state_ptr->ball().vel());
            rpc_predict_state.our_defense_line_x = state_ptr->ourDefenseLineX();
            rpc_predict_state.our_offense_line_x = state_ptr->ourOffensePlayerLineX();

            // RpcActionState
            rpc_action_state_pair.action = rpc_cooperative_action;
            rpc_action_state_pair.predict_state = rpc_predict_state;
            rpc_action_state_pair.evaluation = eval;

            (*map)[unique_index] = rpc_action_state_pair;

        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << '\n';
        }
    }
}
