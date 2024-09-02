// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sample_player.h"

#include "strategy.h"
#include "field_analyzer.h"

#include "action_chain_holder.h"
#include "sample_field_evaluator.h"

#include "soccer_role.h"

#include "sample_communication.h"
#include "keepaway_communication.h"
#include "sample_freeform_message_parser.h"

#include "bhv_penalty_kick.h"
#include "bhv_set_play.h"
#include "bhv_set_play_kick_in.h"
#include "bhv_set_play_indirect_free_kick.h"

#include "bhv_custom_before_kick_off.h"
#include "bhv_strict_check_shoot.h"

#include "view_tactical.h"

#include "intention_receive.h"

#include "basic_actions/basic_actions.h"
#include "basic_actions/bhv_emergency.h"
#include "basic_actions/body_go_to_point.h"
#include "basic_actions/body_intercept.h"
#include "basic_actions/body_kick_one_step.h"
#include "basic_actions/neck_scan_field.h"
#include "basic_actions/neck_turn_to_ball_or_scan.h"
#include "basic_actions/view_synch.h"
#include "basic_actions/kick_table.h"

#include <rcsc/formation/formation.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/say_message_builder.h>
#include <rcsc/player/audio_sensor.h>

#include <rcsc/common/abstract_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/audio_memory.h>
#include <rcsc/common/say_message_parser.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <thread>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
SamplePlayer::SamplePlayer()
    : PlayerAgent(),
      M_communication()
{
    M_field_evaluator = createFieldEvaluator();
    M_action_generator = createActionGenerator();

    std::shared_ptr< AudioMemory > audio_memory( new AudioMemory );

    M_worldmodel.setAudioMemory( audio_memory );

    //
    // set communication message parser
    //
    addSayMessageParser( new BallMessageParser( audio_memory ) );
    addSayMessageParser( new PassMessageParser( audio_memory ) );
    addSayMessageParser( new InterceptMessageParser( audio_memory ) );
    addSayMessageParser( new GoalieMessageParser( audio_memory ) );
    addSayMessageParser( new GoalieAndPlayerMessageParser( audio_memory ) );
    addSayMessageParser( new OffsideLineMessageParser( audio_memory ) );
    addSayMessageParser( new DefenseLineMessageParser( audio_memory ) );
    addSayMessageParser( new WaitRequestMessageParser( audio_memory ) );
    addSayMessageParser( new PassRequestMessageParser( audio_memory ) );
    addSayMessageParser( new DribbleMessageParser( audio_memory ) );
    addSayMessageParser( new BallGoalieMessageParser( audio_memory ) );
    addSayMessageParser( new OnePlayerMessageParser( audio_memory ) );
    addSayMessageParser( new TwoPlayerMessageParser( audio_memory ) );
    addSayMessageParser( new ThreePlayerMessageParser( audio_memory ) );
    addSayMessageParser( new SelfMessageParser( audio_memory ) );
    addSayMessageParser( new TeammateMessageParser( audio_memory ) );
    addSayMessageParser( new OpponentMessageParser( audio_memory ) );
    addSayMessageParser( new BallPlayerMessageParser( audio_memory ) );
    addSayMessageParser( new StaminaMessageParser( audio_memory ) );
    addSayMessageParser( new RecoveryMessageParser( audio_memory ) );

    // addSayMessageParser( new FreeMessageParser< 9 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 8 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 7 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 6 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 5 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 4 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 3 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 2 >( audio_memory ) );
    // addSayMessageParser( new FreeMessageParser< 1 >( audio_memory ) );

    //
    // set freeform message parser
    //
    addFreeformMessageParser( new OpponentPlayerTypeMessageParser( M_worldmodel ) );

    //
    // set communication planner
    //
    M_communication = Communication::Ptr( new SampleCommunication() );
}

/*-------------------------------------------------------------------*/
/*!

 */
SamplePlayer::~SamplePlayer()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SamplePlayer::initImpl( CmdLineParser & cmd_parser )
{


    bool result = PlayerAgent::initImpl( cmd_parser );

    // read additional options
    result &= Strategy::instance().init( cmd_parser );

    rcsc::ParamMap my_params( "Additional options" );
#if 0
    std::string param_file_path = "params";
    param_map.add()
        ( "param-file", "", &param_file_path, "specified parameter file" );
#endif

    cmd_parser.parse( my_params );

    if ( cmd_parser.count( "help" ) > 0 )
    {
        my_params.printHelp( std::cout );
        return false;
    }

    if ( cmd_parser.failed() )
    {
        std::cerr << "player: ***WARNING*** detected unsuppprted options: ";
        cmd_parser.print( std::cerr );
        std::cerr << std::endl;
    }

    if ( ! result )
    {
        return false;
    }

    if ( ! Strategy::instance().read( config().configDir() ) )
    {
        std::cerr << "***ERROR*** Failed to read team strategy." << std::endl;
        return false;
    }

    if ( KickTable::instance().read( config().configDir() + "/kick-table" ) )
    {
        std::cerr << "Loaded the kick table: ["
                  << config().configDir() << "/kick-table]"
                  << std::endl;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!
  main decision
  virtual method in super class
*/
void
SamplePlayer::actionImpl()
{
    if ( this->audioSensor().trainerMessageTime() == world().time() )
    {
        std::cerr << world().ourTeamName() << ' ' << world().self().unum()
                  << ' ' << world().time()
                  << " receive trainer message["
                  << this->audioSensor().trainerMessage() << ']'
                  << std::endl;
    }

    // connect to thrift-client server
    bool connectedToGrpcServer = false;
    if (!M_rpc_client->isConnected()){
        if (M_use_thrift){
#ifdef USE_THRIFT
            dynamic_cast<ThriftClientPlayer*>(M_rpc_client)->init(
                    this,
                    M_rpc_server_address,
                    M_first_rpc_port,
                    M_use_same_rpc_port,
                    M_add_20_to_rpc_port_if_right_side);
#endif
        }
        else
        {
#ifdef USE_GRPC
            dynamic_cast<GrpcClientPlayer*>(M_rpc_client)->init(
                    this,
                    M_rpc_server_address,
                    M_first_rpc_port,
                    M_use_same_rpc_port,
                    M_add_20_to_rpc_port_if_right_side);
#endif
        }
    }
    while (M_rpc_client->isConnected() == false)
    {
        std::cout<<"Connecting to GRPC server..."<<std::endl;
        try{
            connectedToGrpcServer = M_rpc_client->connectToGrpcServer();
            if (connectedToGrpcServer == false) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        catch (const std::exception& e){
            std::cerr << "Error: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    //
    // update strategy and analyzer
    //
    Strategy::instance().update( world() );
    FieldAnalyzer::instance().update( world() );

        //
    // prepare action chain
    //
    M_field_evaluator = createFieldEvaluator();
    M_action_generator = createActionGenerator();

    ActionChainHolder::instance().setFieldEvaluator( M_field_evaluator );
    ActionChainHolder::instance().setActionGenerator( M_action_generator );

    M_rpc_client->sendParams(this->config().offlineLogging());
    M_rpc_client->getActions();
        
    return;
    //
    // update action chain
    //
    ActionChainHolder::instance().update( world() );


    //
    // create current role
    //
    SoccerRole::Ptr role_ptr;
    {
        role_ptr = Strategy::i().createRole( world().self().unum(), world() );

        if ( ! role_ptr )
        {
            std::cerr << config().teamName() << ": "
                      << world().self().unum()
                      << " Error. Role is not registerd.\nExit ..."
                      << std::endl;
            M_client->setServerAlive( false );
            return;
        }
    }


    //
    // override execute if role accept
    //
    if ( role_ptr->acceptExecution( world() ) )
    {
        role_ptr->execute( this );
        return;
    }


    //
    // play_on mode
    //
    if ( world().gameMode().type() == GameMode::PlayOn )
    {
        role_ptr->execute( this );
        return;
    }


    //
    // penalty kick mode
    //
    if ( world().gameMode().isPenaltyKickMode() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": penalty kick" );
        Bhv_PenaltyKick().execute( this );
        return;
    }

    //
    // other set play mode
    //
    Bhv_SetPlay().execute( this );
}

void
SamplePlayer::handleExit()
{
    std::cout << "BYE BYE BYE BYE BYE" << std::endl;
    M_rpc_client->sendByeCommand();
    // call super class method
    PlayerAgent::handleExit();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleActionStart()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleActionEnd()
{
    if ( world().self().posValid() )
    {
#if 0
        const ServerParam & SP = ServerParam::i();
        //
        // inside of pitch
        //

        // top,lower
        debugClient().addLine( Vector2D( world().ourOffenseLineX(),
                                         -SP.pitchHalfWidth() ),
                               Vector2D( world().ourOffenseLineX(),
                                         -SP.pitchHalfWidth() + 3.0 ) );
        // top,lower
        debugClient().addLine( Vector2D( world().ourDefenseLineX(),
                                         -SP.pitchHalfWidth() ),
                               Vector2D( world().ourDefenseLineX(),
                                         -SP.pitchHalfWidth() + 3.0 ) );

        // bottom,upper
        debugClient().addLine( Vector2D( world().theirOffenseLineX(),
                                         +SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().theirOffenseLineX(),
                                         +SP.pitchHalfWidth() ) );
        //
        debugClient().addLine( Vector2D( world().offsideLineX(),
                                         world().self().pos().y - 15.0 ),
                               Vector2D( world().offsideLineX(),
                                         world().self().pos().y + 15.0 ) );

        // outside of pitch

        // top,upper
        debugClient().addLine( Vector2D( world().ourOffensePlayerLineX(),
                                         -SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().ourOffensePlayerLineX(),
                                         -SP.pitchHalfWidth() ) );
        // top,upper
        debugClient().addLine( Vector2D( world().ourDefensePlayerLineX(),
                                         -SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().ourDefensePlayerLineX(),
                                         -SP.pitchHalfWidth() ) );
        // bottom,lower
        debugClient().addLine( Vector2D( world().theirOffensePlayerLineX(),
                                         +SP.pitchHalfWidth() ),
                               Vector2D( world().theirOffensePlayerLineX(),
                                         +SP.pitchHalfWidth() + 3.0 ) );
        // bottom,lower
        debugClient().addLine( Vector2D( world().theirDefensePlayerLineX(),
                                         +SP.pitchHalfWidth() ),
                               Vector2D( world().theirDefensePlayerLineX(),
                                         +SP.pitchHalfWidth() + 3.0 ) );
#else
        // top,lower
        debugClient().addLine( Vector2D( world().ourDefenseLineX(),
                                         world().self().pos().y - 2.0 ),
                               Vector2D( world().ourDefenseLineX(),
                                         world().self().pos().y + 2.0 ) );

        //
        debugClient().addLine( Vector2D( world().offsideLineX(),
                                         world().self().pos().y - 15.0 ),
                               Vector2D( world().offsideLineX(),
                                         world().self().pos().y + 15.0 ) );
#endif
    }

    //
    // ball position & velocity
    //
    dlog.addText( Logger::WORLD,
                  "WM: BALL pos=(%lf, %lf), vel=(%lf, %lf, r=%lf, ang=%lf)",
                  world().ball().pos().x,
                  world().ball().pos().y,
                  world().ball().vel().x,
                  world().ball().vel().y,
                  world().ball().vel().r(),
                  world().ball().vel().th().degree() );


    dlog.addText( Logger::WORLD,
                  "WM: SELF move=(%lf, %lf, r=%lf, th=%lf)",
                  world().self().lastMove().x,
                  world().self().lastMove().y,
                  world().self().lastMove().r(),
                  world().self().lastMove().th().degree() );

    if ( world().prevBall().rpos().isValid() )
    {
        Vector2D diff = world().ball().rpos() - world().prevBall().rpos();
        dlog.addText( Logger::WORLD,
                      "WM: BALL rpos=(%lf %lf) prev_rpos=(%lf %lf) diff=(%lf %lf)",
                  world().ball().rpos().x,
                      world().ball().rpos().y,
                      world().prevBall().rpos().x,
                      world().prevBall().rpos().y,
                      diff.x,
                      diff.y );

        Vector2D ball_move = diff + world().self().lastMove();
        Vector2D diff_vel = ball_move * ServerParam::i().ballDecay();
        dlog.addText( Logger::WORLD,
                      "---> ball_move=(%lf %lf) vel=(%lf, %lf, r=%lf, th=%lf)",
                      ball_move.x,
                      ball_move.y,
                      diff_vel.x,
                      diff_vel.y,
                      diff_vel.r(),
                      diff_vel.th().degree() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleInitMessage()
{
    {
        // Initializing the order of penalty kickers
        std::vector< int > unum_order_pk_kickers = { 10, 9, 2, 11, 3, 4, 1, 5, 6, 7, 8 };
        M_worldmodel.setPenaltyKickTakerOrder( unum_order_pk_kickers );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleServerParam()
{
    if ( ServerParam::i().keepawayMode() )
    {
        std::cerr << "set Keepaway mode communication." << std::endl;
        M_communication = Communication::Ptr( new KeepawayCommunication() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handlePlayerParam()
{
    if ( KickTable::instance().createTables() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << " KickTable created."
                  << std::endl;
    }
    else
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << " KickTable failed..."
                  << std::endl;
        M_client->setServerAlive( false );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handlePlayerType()
{

}

/*-------------------------------------------------------------------*/
/*!
  communication decision.
  virtual method in super class
*/
void
SamplePlayer::communicationImpl()
{
    if ( M_communication )
    {
        M_communication->execute( this );
    }
}



/*-------------------------------------------------------------------*/
/*!

*/
FieldEvaluator::ConstPtr
SamplePlayer::getFieldEvaluator() const
{
    return M_field_evaluator;
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldEvaluator::ConstPtr
SamplePlayer::createFieldEvaluator() const
{
    return FieldEvaluator::ConstPtr( new SampleFieldEvaluator );
}


/*-------------------------------------------------------------------*/
/*!
*/
#include "actgen_cross.h"
#include "actgen_direct_pass.h"
#include "actgen_self_pass.h"
#include "actgen_strict_check_pass.h"
#include "actgen_short_dribble.h"
#include "actgen_simple_dribble.h"
#include "actgen_shoot.h"
#include "actgen_action_chain_length_filter.h"

ActionGenerator::ConstPtr
SamplePlayer::createActionGenerator() const
{
    CompositeActionGenerator * g = new CompositeActionGenerator();

    //
    // shoot
    //
    g->addGenerator( new ActGen_RangeActionChainLengthFilter
                     ( new ActGen_Shoot(),
                       2, ActGen_RangeActionChainLengthFilter::MAX ) );

    //
    // strict check pass
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_StrictCheckPass(), 1 ) );

    //
    // cross
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_Cross(), 1 ) );

    //
    // direct pass
    //
    // g->addGenerator( new ActGen_RangeActionChainLengthFilter
    //                  ( new ActGen_DirectPass(),
    //                    2, ActGen_RangeActionChainLengthFilter::MAX ) );

    //
    // short dribble
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_ShortDribble(), 1 ) );

    //
    // self pass (long dribble)
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_SelfPass(), 1 ) );

    //
    // simple dribble
    //
    // g->addGenerator( new ActGen_RangeActionChainLengthFilter
    //                  ( new ActGen_SimpleDribble(),
    //                    2, ActGen_RangeActionChainLengthFilter::MAX ) );

    return ActionGenerator::ConstPtr( g );
}