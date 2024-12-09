#include "rpc-client/rpc-player-client.h"
#include <rcsc/common/logger.h>
#include "view_tactical.h"
#include "rcsc/player/world_model.h"
#include "bhv_custom_before_kick_off.h"
#include "basic_actions/neck_turn_to_ball_or_scan.h"
#include "strategy.h"
#include "basic_actions/basic_actions.h"
#include "basic_actions/bhv_emergency.h"
#include <rcsc/common/audio_memory.h>
#include "basic_actions/body_kick_one_step.h"
#include "basic_actions/neck_scan_field.h"
#include "basic_actions/body_go_to_point.h"
#include "basic_actions/body_intercept.h"
#include "intention_receive.h"

using namespace rcsc;

bool
RpcPlayerClient::checkPreprocess(PlayerAgent * agent) {
    // check tackle expires
    // check self position accuracy
    // ball search
    // check queued intention
    // check simultaneous kick
    
    const WorldModel & wm = agent->world();

    dlog.addText( Logger::TEAM,
                  __FILE__": (doPreProcess)" );

    //
    // freezed by tackle effect
    //
    if ( wm.self().isFrozen() )
    {
                dlog.addText( Logger::TEAM,
                      __FILE__": tackle wait. expires= %d",
                      wm.self().tackleExpires() );
        return true;
    }

    //
    // BeforeKickOff or AfterGoal. jump to the initial position
    //
    if ( wm.gameMode().type() == GameMode::BeforeKickOff
         || wm.gameMode().type() == GameMode::AfterGoal_ )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": before_kick_off" );
        return true;
    }

    //
    // self localization error
    //
    if ( ! wm.self().posValid() )
    {
        return true;
    }

    //
    // ball localization error
    //
    const int count_thr = ( wm.self().goalie()
                            ? 10
                            : 5 );
    if ( wm.ball().posCount() > count_thr
         || ( wm.gameMode().type() != GameMode::PlayOn
              && wm.ball().seenPosCount() > count_thr + 10 ) )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": search ball" );
        return true;
    }

    //
    // set default change view
    //

    agent->setViewAction( new View_Tactical() );

    //
    // check queued action
    //
    // if ( agent->doIntention() )
    // {
    //     std::cout<<"doIntention"<<std::endl;
    //     dlog.addText( Logger::TEAM,
    //                   __FILE__": do queued intention" );
    //     return true;
    // }


    return false;
}

bool RpcPlayerClient::checkdoForceKick(PlayerAgent * agent) {
    //
    // check simultaneous kick
    //
    const WorldModel & wm = agent->world();
    if ( wm.gameMode().type() == GameMode::PlayOn
         && ! wm.self().goalie()
         && wm.self().isKickable()
         && wm.kickableOpponent() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": simultaneous kick" );
        agent->debugClient().addMessage( "SimultaneousKick" );
        Vector2D goal_pos( ServerParam::i().pitchHalfLength(), 0.0 );

        if ( wm.self().pos().x > 36.0
             && wm.self().pos().absY() > 10.0 )
        {
            goal_pos.x = 45.0;
            dlog.addText( Logger::TEAM,
                          __FILE__": simultaneous kick cross type" );
        }
        return true;
    }
    return false;
}

bool RpcPlayerClient::checkdoHeardPassReceive(PlayerAgent * agent) {
    const WorldModel & wm = agent->world();

    //
    // check pass message
    //
    if ( !(wm.audioMemory().passTime() != wm.time()
           || wm.audioMemory().pass().empty()
           || wm.audioMemory().pass().front().receiver_ != wm.self().unum()) )
    {

        return true;
    }

    return false;
}

bool
RpcPlayerClient::doPreprocess(PlayerAgent * agent) {
    // check tackle expires
    // check self position accuracy
    // ball search
    // check queued intention
    // check simultaneous kick
    
    const WorldModel & wm = agent->world();

    dlog.addText( Logger::TEAM,
                  __FILE__": (doPreProcess)" );

    //
    // freezed by tackle effect
    //
    if ( wm.self().isFrozen() )
    {
                dlog.addText( Logger::TEAM,
                      __FILE__": tackle wait. expires= %d",
                      wm.self().tackleExpires() );
        // face neck to ball
        agent->setViewAction( new View_Tactical() );
        agent->setNeckAction( new Neck_TurnToBallOrScan( 0 ) );
        return true;
    }

    //
    // BeforeKickOff or AfterGoal. jump to the initial position
    //
    if ( wm.gameMode().type() == GameMode::BeforeKickOff
         || wm.gameMode().type() == GameMode::AfterGoal_ )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": before_kick_off" );
        Vector2D move_point =  Strategy::i().getPosition( wm.self().unum() );
        Bhv_CustomBeforeKickOff( move_point ).execute( agent );
        agent->setViewAction( new View_Tactical() );
        return true;
    }

    //
    // self localization error
    //
    if ( ! wm.self().posValid() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": invalid my pos" );
        Bhv_Emergency().execute( agent ); // includes change view
        return true;
    }

    //
    // ball localization error
    //
    const int count_thr = ( wm.self().goalie()
                            ? 10
                            : 5 );
    if ( wm.ball().posCount() > count_thr
         || ( wm.gameMode().type() != GameMode::PlayOn
              && wm.ball().seenPosCount() > count_thr + 10 ) )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": search ball" );
        agent->setViewAction( new View_Tactical() );
        Bhv_NeckBodyToBall().execute( agent );
        return true;
    }

    //
    // set default change view
    //

    agent->setViewAction( new View_Tactical() );

    return false;
}

bool
RpcPlayerClient::doForceKick(PlayerAgent * agent)
{
    const WorldModel & wm = agent->world();

    if ( wm.gameMode().type() == GameMode::PlayOn
         && ! wm.self().goalie()
         && wm.self().isKickable()
         && wm.kickableOpponent() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": simultaneous kick" );
        agent->debugClient().addMessage( "SimultaneousKick" );
        Vector2D goal_pos( ServerParam::i().pitchHalfLength(), 0.0 );

        if ( wm.self().pos().x > 36.0
             && wm.self().pos().absY() > 10.0 )
        {
            goal_pos.x = 45.0;
            dlog.addText( Logger::TEAM,
                          __FILE__": simultaneous kick cross type" );
        }
        Body_KickOneStep( goal_pos,
                          ServerParam::i().ballSpeedMax()
                          ).execute( agent );
        agent->setNeckAction( new Neck_ScanField() );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
RpcPlayerClient::doHeardPassReceive(PlayerAgent * agent)
{
    const WorldModel & wm = agent->world();

    if ( wm.audioMemory().passTime() != wm.time()
         || wm.audioMemory().pass().empty()
         || wm.audioMemory().pass().front().receiver_ != wm.self().unum() )
    {

        return false;
    }

    int self_min = wm.interceptTable().selfStep();
    Vector2D intercept_pos = wm.ball().inertiaPoint( self_min );
    Vector2D heard_pos = wm.audioMemory().pass().front().receive_pos_;

    dlog.addText( Logger::TEAM,
                  __FILE__":  (doHeardPassReceive) heard_pos(%.2f %.2f) intercept_pos(%.2f %.2f)",
                  heard_pos.x, heard_pos.y,
                  intercept_pos.x, intercept_pos.y );

    if ( ! wm.kickableTeammate()
         && wm.ball().posCount() <= 1
         && wm.ball().velCount() <= 1
         && self_min < 20
         //&& intercept_pos.dist( heard_pos ) < 3.0 ) //5.0 )
         )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": (doHeardPassReceive) intercept cycle=%d. intercept",
                      self_min );
        agent->debugClient().addMessage( "Comm:Receive:Intercept" );
        Body_Intercept().execute( agent );
        agent->setNeckAction( new Neck_TurnToBall() );
    }
    else
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": (doHeardPassReceive) intercept cycle=%d. go to receive point",
                      self_min );
        agent->debugClient().setTarget( heard_pos );
        agent->debugClient().addMessage( "Comm:Receive:GoTo" );
        Body_GoToPoint( heard_pos,
                    0.5,
                        ServerParam::i().maxDashPower()
                        ).execute( agent );
        agent->setNeckAction( new Neck_TurnToBall() );
    }

    agent->setIntention( new IntentionReceive( heard_pos,
                                              ServerParam::i().maxDashPower(),
                                              0.9,
                                              5,
                                              wm.time() ) );

    return true;
}