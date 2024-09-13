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

void ThriftClientPlayer::getActions()
{
    auto agent = M_agent;
    bool pre_process = checkPreprocess(agent);
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
    std::cout<<"action size:"<<actions.actions.size()<<std::endl;
    int body_action_done = 0;
    for (int i = 0; i < actions.actions.size(); i++)
    {
        auto action = actions.actions[i];
        
        if (action.__isset.dash)
        {
            std::cout<<"Dash:"<<action.dash.power<<","<<action.dash.relative_direction<<std::endl;
            agent->doDash(action.dash.power, action.dash.relative_direction);
            body_action_done++;
            continue;
        }
        if (action.__isset.kick)
        {
            agent->doKick(action.kick.power, action.kick.relative_direction);
            body_action_done++;
            continue;
        }
        if (action.__isset.turn)
        {
            agent->doTurn(action.turn.relative_direction);
            body_action_done++;
            continue;
        }
        if (action.__isset.tackle)
        {
            agent->doTackle(action.tackle.power_or_dir, action.tackle.foul);
            body_action_done++;
            continue;
        }
        if (action.__isset.catch_action)
        {
            agent->doCatch();
            body_action_done++;
            continue;
        }
        if (action.__isset.move)
        {
            agent->doMove(action.move.x, action.move.y);
            body_action_done++;
            continue;
        }
        if (action.__isset.turn_neck)
        {
            agent->doTurnNeck(action.turn_neck.moment);
            continue;
        }
        if (action.__isset.change_view)
        {
            const rcsc::ViewWidth view_width = ThriftAgent::convertViewWidth(action.change_view.view_width);
            agent->doChangeView(view_width);
            continue;
        }
        if (action.__isset.say)
        {
            addSayMessage(action.say);
            continue;
        }
        if (action.__isset.point_to)
        {
            agent->doPointto(action.point_to.x, action.point_to.y);
            continue;
        }
        if (action.__isset.point_to_of)
        {
            agent->doPointtoOff();
            continue;
        }
        if (action.__isset.attention_to)
        {
            const rcsc::SideID side = ThriftAgent::convertSideID(action.attention_to.side);
            agent->doAttentionto(side, action.attention_to.unum);
            continue;
        }
        if (action.__isset.attention_to_of)
        {
            agent->doAttentiontoOff();
            continue;
        }
        if (action.__isset.log)
        {
            addDlog(action.log);
            continue;
        }
        // todo debugClient
        if (action.__isset.body_go_to_point)
        {
            const auto &bodyGoToPoint = action.body_go_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyGoToPoint.target_point);
            Body_GoToPoint(targetPoint, bodyGoToPoint.distance_threshold, bodyGoToPoint.max_dash_power).execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_smart_kick)
        {
            const auto &bodySmartKick = action.body_smart_kick;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodySmartKick.target_point);
            Body_SmartKick(targetPoint, bodySmartKick.first_speed, bodySmartKick.first_speed_threshold,
                           bodySmartKick.max_steps).execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.bhv_before_kick_off)
        {
            const auto &bhvBeforeKickOff = action.bhv_before_kick_off;
            const auto &point = ThriftAgent::convertVector2D(bhvBeforeKickOff.point);
            Bhv_BeforeKickOff(point).execute(agent);
            continue;
        }
        if (action.__isset.bhv_body_neck_to_ball)
        {
            Bhv_BodyNeckToBall().execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.bhv_body_neck_to_point)
        {
            const auto &bhvBodyNeckToPoint = action.bhv_body_neck_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bhvBodyNeckToPoint.point);
            Bhv_BodyNeckToPoint(targetPoint).execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.bhv_emergency)
        {
            Bhv_Emergency().execute(agent);
            continue;
        }
        if (action.__isset.bhv_go_to_point_look_ball)
        {
            const auto &bhvGoToPointLookBall = action.bhv_go_to_point_look_ball;
            const auto &targetPoint = ThriftAgent::convertVector2D(bhvGoToPointLookBall.target_point);
            Bhv_GoToPointLookBall(targetPoint, bhvGoToPointLookBall.distance_threshold, bhvGoToPointLookBall.max_dash_power).execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.bhv_neck_body_to_ball)
        {
            const auto &bhvNeckBodyToBall = action.bhv_neck_body_to_ball;
            Bhv_NeckBodyToBall(bhvNeckBodyToBall.angle_buf).execute(agent);
            continue;
        }
        if (action.__isset.bhv_neck_body_to_point)
        {
            const auto &bhvNeckBodyToPoint = action.bhv_neck_body_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bhvNeckBodyToPoint.point);
            Bhv_NeckBodyToPoint(targetPoint, bhvNeckBodyToPoint.angle_buf).execute(agent);
            continue;
        }
        if (action.__isset.bhv_scan_field)
        {
            Bhv_ScanField().execute(agent);
            continue;
        }
        if (action.__isset.body_advance_ball)
        {
            Body_AdvanceBall().execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_clear_ball)
        {
            Body_ClearBall().execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_dribble)
        {
            const auto &bodyDribble = action.body_dribble;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyDribble.target_point);
            Body_Dribble(
                targetPoint,
                bodyDribble.distance_threshold,
                bodyDribble.dash_power,
                bodyDribble.dash_count,
                bodyDribble.dodge)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_go_to_point_dodge)
        {
            const auto &bodyGoToPointDodge = action.body_go_to_point_dodge;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyGoToPointDodge.target_point);
            Body_GoToPointDodge(
                targetPoint,
                bodyGoToPointDodge.dash_power)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_hold_ball)
        {
            const auto &bodyHoldBall = action.body_hold_ball;
            const auto &turnTargetPoint = ThriftAgent::convertVector2D(bodyHoldBall.turn_target_point);
            const auto &kickTargetPoint = ThriftAgent::convertVector2D(bodyHoldBall.kick_target_point);
            Body_HoldBall(
                bodyHoldBall.do_turn,
                turnTargetPoint,
                kickTargetPoint)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_intercept)
        {
            const auto &bodyIntercept = action.body_intercept;
            const auto &facePoint = ThriftAgent::convertVector2D(bodyIntercept.face_point);
            Body_Intercept(
                bodyIntercept.save_recovery,
                facePoint)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_kick_one_step)
        {
            const auto &bodyKickOneStep = action.body_kick_one_step;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyKickOneStep.target_point);
            Body_KickOneStep(
                targetPoint,
                bodyKickOneStep.first_speed,
                bodyKickOneStep.force_mode)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_stop_ball)
        {
            Body_StopBall().execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_stop_dash)
        {
            const auto &bodyStopDash = action.body_stop_dash;
            Body_StopDash(
                bodyStopDash.save_recovery)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_tackle_to_point)
        {
            const auto &bodyTackleToPoint = action.body_tackle_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyTackleToPoint.target_point);
            Body_TackleToPoint(
                targetPoint,
                bodyTackleToPoint.min_probability,
                bodyTackleToPoint.min_speed)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_turn_to_angle)
        {
            const auto &bodyTurnToAngle = action.body_turn_to_angle;
            Body_TurnToAngle(
                bodyTurnToAngle.angle)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_turn_to_ball)
        {
            const auto &bodyTurnToBall = action.body_turn_to_ball;
            Body_TurnToBall(
                bodyTurnToBall.cycle)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.body_turn_to_point)
        {
            const auto &bodyTurnToPoint = action.body_turn_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(bodyTurnToPoint.target_point);
            Body_TurnToPoint(
                targetPoint,
                bodyTurnToPoint.cycle)
                .execute(agent);
            body_action_done++;
            continue;
        }
        if (action.__isset.focus_move_to_point)
        {
            const auto &focusMoveToPoint = action.focus_move_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(focusMoveToPoint.target_point);
            rcsc::Focus_MoveToPoint(
                targetPoint)
                .execute(agent);
            continue;
        }
        if (action.__isset.focus_reset)
        {
            rcsc::Focus_Reset().execute(agent);
            continue;
        }
        if (action.__isset.neck_scan_field)
        {
            Neck_ScanField().execute(agent);
            continue;
        }
        if (action.__isset.neck_scan_players)
        {
            Neck_ScanPlayers().execute(agent);
            continue;
        }
        if (action.__isset.neck_turn_to_ball_and_player)
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
                Neck_TurnToBallAndPlayer(
                    player,
                    neckTurnToBallAndPlayer.count_threshold)
                    .execute(agent);
            }
            continue;
        }
        if (action.__isset.neck_turn_to_ball_or_scan)
        {
            const auto &neckTurnToBallOrScan = action.neck_turn_to_ball_or_scan;
            Neck_TurnToBallOrScan(
                neckTurnToBallOrScan.count_threshold)
                .execute(agent);
            continue;
        }
        if (action.__isset.neck_turn_to_ball)
        {
            Neck_TurnToBall().execute(agent);
            continue;
        }
        if (action.__isset.neck_turn_to_goalie_or_scan)
        {
            const auto &neckTurnToGoalieOrScan = action.neck_turn_to_goalie_or_scan;
            Neck_TurnToGoalieOrScan(
                neckTurnToGoalieOrScan.count_threshold)
                .execute(agent);
            continue;
        }
        if (action.__isset.neck_turn_to_low_conf_teammate)
        {
            const auto &neckTurnToLowConfTeammate = action.neck_turn_to_low_conf_teammate;
            Neck_TurnToLowConfTeammate().execute(agent);
            continue;
        }
        if (action.__isset.neck_turn_to_player_or_scan)
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
                Neck_TurnToPlayerOrScan(
                    player,
                    neckTurnToPlayerOrScan.count_threshold)
                    .execute(agent);
            }
            continue;
        }
        if (action.__isset.neck_turn_to_point)
        {
            const auto &neckTurnToPoint = action.neck_turn_to_point;
            const auto &targetPoint = ThriftAgent::convertVector2D(neckTurnToPoint.target_point);
            Neck_TurnToPoint(
                targetPoint)
                .execute(agent);
            continue;
        }
        if (action.__isset.neck_turn_to_relative)
        {
            const auto &neckTurnToRelative = action.neck_turn_to_relative;
            Neck_TurnToRelative(
                neckTurnToRelative.angle)
                .execute(agent);
            continue;
        }
        if (action.__isset.view_change_width)
        {
            const auto &viewChangeWidth = action.view_change_width;
            const rcsc::ViewWidth view_width = ThriftAgent::convertViewWidth(viewChangeWidth.view_width);
            View_ChangeWidth(
                view_width)
                .execute(agent);
            continue;
        }
        if (action.__isset.view_normal)
        {
            View_Normal().execute(agent);
            continue;
        }
        if (action.__isset.view_wide)
        {
            View_Wide().execute(agent);
            continue;
        }
        if (action.__isset.view_synch)
        {
            View_Synch().execute(agent);
            continue;
        }
        if (action.__isset.helios_goalie)
        {
            RoleGoalie roleGoalie = RoleGoalie();
            roleGoalie.execute(agent);
            continue;
        }
        if (action.__isset.helios_goalie_move)
        {
            RoleGoalie roleGoalie = RoleGoalie();
            roleGoalie.doMove(agent);
            continue;
        }
        if (action.__isset.helios_goalie_kick)
        {
            RoleGoalie roleGoalie = RoleGoalie();
            roleGoalie.doKick(agent);
            continue;
        }
        if (action.__isset.helios_shoot)
        {
            const rcsc::WorldModel &wm = agent->world();

            if (wm.gameMode().type() != rcsc::GameMode::IndFreeKick_
                && wm.time().stopped() == 0 && wm.self().isKickable() && Bhv_StrictCheckShoot().execute(agent))
            {
            }
            continue;
        }
        if (action.__isset.helios_basic_move)
        {
            Bhv_BasicMove().execute(agent);
            continue;
        }
        if (action.__isset.helios_set_play)
        {
            Bhv_SetPlay().execute(agent);
            continue;
        }
        if (action.__isset.helios_penalty)
        {
            Bhv_PenaltyKick().execute(agent);
            continue;
        }
        if (action.__isset.helios_communication)
        {
            sample_communication->execute(agent);
            continue;
        }
        if (action.__isset.helios_chain_action)
        {
            FieldEvaluator::ConstPtr field_evaluator = FieldEvaluator::ConstPtr(new SampleFieldEvaluator);
            CompositeActionGenerator *g = new CompositeActionGenerator();

            if (action.helios_chain_action.lead_pass
                || action.helios_chain_action.direct_pass || action.helios_chain_action.through_pass)
                g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_StrictCheckPass(), 1));
            if (action.helios_chain_action.cross)
                g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_Cross(), 1));
            if (action.helios_chain_action.simple_pass)
                g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_DirectPass(),
                                                                        2, ActGen_RangeActionChainLengthFilter::MAX));
            if (action.helios_chain_action.short_dribble)
                g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_ShortDribble(), 1));
            if (action.helios_chain_action.long_dribble)
                g->addGenerator(new ActGen_MaxActionChainLengthFilter(new ActGen_SelfPass(), 1));
            if (action.helios_chain_action.simple_dribble)
                g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_SimpleDribble(),
                                                                        2, ActGen_RangeActionChainLengthFilter::MAX));
            if (action.helios_chain_action.simple_shoot)
                g->addGenerator(new ActGen_RangeActionChainLengthFilter(new ActGen_Shoot(),
                                                                        2, ActGen_RangeActionChainLengthFilter::MAX));
            if (g->M_generators.empty())
            {
                Body_HoldBall().execute(agent);
                agent->setNeckAction(new Neck_ScanField());
                continue;
            }
            ActionGenerator::ConstPtr action_generator = ActionGenerator::ConstPtr(g);
            ActionChainHolder::instance().setFieldEvaluator(field_evaluator);
            ActionChainHolder::instance().setActionGenerator(action_generator);
            ActionChainHolder::instance().update(agent->world());
            
            if (action.helios_chain_action.server_side_decision)
            {
                if (GetBestPlannerAction())
                {
                    agent->debugClient().addMessage("GetBestPlannerAction");
                    continue;
                }
            }
            else
            {
                if (Bhv_PlannedAction().execute(agent))
                {
                    agent->debugClient().addMessage("PlannedAction");
                    continue;
                }

            }
            Body_HoldBall().execute(agent);
            agent->setNeckAction(new Neck_ScanField());
            continue;
        }
        #ifdef DEBUG_CLIENT_PLAYER
        std::cout << "Unkown action"<<std::endl;
        #endif
        Body_HoldBall().execute(agent);
        agent->setNeckAction(new Neck_ScanField());
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
            auto map = & action_state_pairs.pairs;
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



    if (Bhv_PlannedAction().execute(agent, best_action.index))
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
soccer::State ThriftClientPlayer::generateState() const
{
    const rcsc::WorldModel &wm = M_agent->world();
    soccer::WorldModel worldModel = ThriftStateGenerator::convertWorldModel(wm);
    addHomePosition(&worldModel);
    soccer::State state;
    state.world_model = worldModel;
    return state;
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