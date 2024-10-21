// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sample_field_evaluator.h"

#include "field_analyzer.h"
#include "simple_pass_checker.h"

#include <rcsc/player/player_evaluator.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/math_util.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cfloat>

// #define DEBUG_PRINT

using namespace rcsc;

static const int VALID_PLAYER_THRESHOLD = 8;


/*-------------------------------------------------------------------*/
/*!

 */
static double evaluate_state( const PredictState & state,
                              const double & helios_x_term = 1.0,
                              const double & helios_ball_dist_to_goal_term = 1.0,
                              const double & helios_effective_max_ball_dist_to_goal = 40.0 );

static double evaluate_state_2d( const PredictState & state,
                                 const std::vector < std::vector < double > > & field_evaluator_2d);


/*-------------------------------------------------------------------*/
/*!

 */
SampleFieldEvaluator::SampleFieldEvaluator()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
SampleFieldEvaluator::~SampleFieldEvaluator()
{

}

#ifdef USE_GRPC
using protos::PlannerEvalutionEfector;
using protos::PlannerEvalutionFieldEvaluator;
void 
SampleFieldEvaluator::set_grpc_evalution_method( const PlannerEvalution & evalution )
{
    auto effectors = evalution.effectors();
    auto field_evaluators = evalution.field_evaluators();

    for ( auto & effector : effectors )
    {
        if ( effector.efector_case() == PlannerEvalutionEfector::kOpponentEffect )
        {
            if (effector.opponent_effect().negetive_effect_by_distance().size() > 0)
                m_opponent_negetive_effect_by_distance.assign(
                    effector.opponent_effect().negetive_effect_by_distance().begin(),
                    effector.opponent_effect().negetive_effect_by_distance().end()
                );
            if (effector.opponent_effect().negetive_effect_by_reach_steps().size() > 0)
                m_opponent_negetive_effect_by_reach_steps.assign(
                    effector.opponent_effect().negetive_effect_by_reach_steps().begin(),
                    effector.opponent_effect().negetive_effect_by_reach_steps().end()
                );
        }
        if ( effector.efector_case() == PlannerEvalutionEfector::kActionTypeEffect )
        {
            m_use_action_term = true;
            m_direct_pass_term = effector.action_type_effect().direct_pass();
            m_lead_pass_term = effector.action_type_effect().lead_pass();
            m_through_pass_term = effector.action_type_effect().through_pass();
            m_short_dribble_term = effector.action_type_effect().short_dribble();
            m_long_dribble_term = effector.action_type_effect().long_dribble();
            m_cross_term = effector.action_type_effect().cross();
            m_hold_term = effector.action_type_effect().hold();
        }

        if ( effector.efector_case() == PlannerEvalutionEfector::kTeammateEffect )
        {
            for ( auto & effect : effector.teammate_effect().positive_effect() )
            {
                m_teammate_positive_effect[effect.first] = effect.second;
            }
        }
    }

    for ( auto & field_evaluator : field_evaluators )
    {
        if ( field_evaluator.field_evaluator_case() == PlannerEvalutionFieldEvaluator::kFieldEvaluatorHelios )
        {
            m_use_heleos = true;
            m_helios_x_term = field_evaluator.field_evaluator_helios().x_term();
            m_helios_ball_dist_to_goal_term = field_evaluator.field_evaluator_helios().ball_dist_to_goal_term();
            m_helios_effective_max_ball_dist_to_goal = field_evaluator.field_evaluator_helios().effective_max_ball_dist_to_goal();
        }

        if ( field_evaluator.field_evaluator_case() == PlannerEvalutionFieldEvaluator::kFieldEvaluator2D )
        {
            m_use_field_evaluator_2d = true;
            m_field_evaluator_2d.clear();
            for ( auto & x_row : field_evaluator.field_evaluator_2d().evals() )
            {
                std::vector< double > row;
                for ( auto & y_row : x_row.evals() )
                {
                    row.push_back( y_row );
                }
                m_field_evaluator_2d.push_back( row );
            }
        }
    }
}
#endif

/*-------------------------------------------------------------------*/
/*!

 */
double
SampleFieldEvaluator::operator()( const PredictState & state,
                                  const std::vector< ActionStatePair > & path,
                                  const rcsc::WorldModel & wm ) const
{
    double state_evaluation = 0;
    
    if ( m_use_heleos )
        state_evaluation += evaluate_state( state,
                                                  m_helios_x_term,
                                                  m_helios_ball_dist_to_goal_term,
                                                  m_helios_effective_max_ball_dist_to_goal );

    if ( m_use_field_evaluator_2d )
        state_evaluation += evaluate_state_2d( state, m_field_evaluator_2d );

    if ( m_use_action_term )
        state_evaluation = effected_by_action_term( state, path, state_evaluation );
    
    state_evaluation = effected_by_opponent_distance( state, state_evaluation, wm );

    state_evaluation = effected_by_opponent_reach_step( state, state_evaluation, wm );

    state_evaluation = effected_by_teammate( state, state_evaluation );
    //
    // ???
    //

    double result = state_evaluation;

    return result;
}


double
SampleFieldEvaluator::effected_by_action_term( const PredictState & state,
                                               const std::vector< ActionStatePair > & path,
                                               const double & eval ) const
{
    if ( path.size() == 0 )
        return eval * m_hold_term;
    const ActionStatePair asp = path.at(0);
    if ( asp.action().description() == "strictDirect" )
        return eval * m_direct_pass_term;
    else if ( asp.action().description() == "strictLead" )
        return eval * m_lead_pass_term;
    else if ( asp.action().description() == "strictThrough" )
        return eval * m_through_pass_term;
    else if ( asp.action().description() == "shortDribble" )
        return eval * m_short_dribble_term;
    else if ( asp.action().description() == "SelfPass" )
        return eval * m_long_dribble_term;
    else if ( asp.action().description() == "cross" )
        return eval * m_cross_term;
    return eval;
}

double 
SampleFieldEvaluator::effected_by_opponent_distance( const PredictState & state,
                                                     const double & eval,
                                                     const WorldModel & wm ) const
{
    if ( m_opponent_negetive_effect_by_distance.size() == 0 )
        return eval;
    auto ball_pos = state.ball().pos();

    double min_dist = 1000.0;

    for ( auto & opp : wm.theirPlayers() )
    {
        if ( opp->unum() <= 0 )
            continue;

        double dist = ball_pos.dist( opp->pos() );

        if ( dist < min_dist )
            min_dist = dist;
    }

    int min_dist_int = (int) min_dist;

    if ( min_dist_int >= m_opponent_negetive_effect_by_distance.size() )
        return eval;
    
    return eval - m_opponent_negetive_effect_by_distance[min_dist_int];
}

double 
SampleFieldEvaluator::effected_by_opponent_reach_step( const PredictState & state,
                                                       const double & eval,
                                                       const WorldModel & wm ) const
{
    if ( m_opponent_negetive_effect_by_reach_steps.size() == 0 )
        return eval;
    auto ball_pos = state.ball().pos();

    int min_reach = 1000;

    for ( auto & opp : wm.theirPlayers() )
    {
        if ( opp->unum() <= 0 )
            continue;

        double reach = opp->playerTypePtr()->cyclesToReachDistance( ball_pos.dist( opp->pos() ) );

        if ( reach < min_reach )
            min_reach = reach;
    }

    if ( min_reach >= m_opponent_negetive_effect_by_reach_steps.size() )
        return eval;
    
    return eval - m_opponent_negetive_effect_by_reach_steps[min_reach];
}

double 
SampleFieldEvaluator::effected_by_teammate( const PredictState & state,
                                            const double & eval ) const
{
    if ( m_teammate_positive_effect.size() == 0 )
        return eval;
    const ServerParam & SP = ServerParam::i();

    const AbstractPlayerObject * holder = state.ballHolder();

    int unum = holder->unum();

    if ( m_teammate_positive_effect.find( unum ) == m_teammate_positive_effect.end() )
        return eval;
    
    return eval * m_teammate_positive_effect.at( unum );
}
/*-------------------------------------------------------------------*/
/*!

 */
static
double
evaluate_state( const PredictState & state,
                const double & helios_x_term,
                const double & helios_ball_dist_to_goal_term,
                const double & helios_effective_max_ball_dist_to_goal)
{
    const ServerParam & SP = ServerParam::i();

    const AbstractPlayerObject * holder = state.ballHolder();

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "========= (evaluate_state) ==========" );
#endif

    //
    // if holder is invalid, return bad evaluation
    //
    if ( ! holder )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX null holder" );
#endif
        return - DBL_MAX / 2.0;
    }

    const int holder_unum = holder->unum();


    //
    // ball is in opponent goal
    //
    if ( state.ball().pos().x > + ( SP.pitchHalfLength() - 0.1 )
         && state.ball().pos().absY() < SP.goalHalfWidth() + 2.0 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) *** in opponent goal" );
#endif
        return +1.0e+7;
    }

    //
    // ball is in our goal
    //
    if ( state.ball().pos().x < - ( SP.pitchHalfLength() - 0.1 )
         && state.ball().pos().absY() < SP.goalHalfWidth() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX in our goal" );
#endif

        return -1.0e+7;
    }


    //
    // out of pitch
    //
    if ( state.ball().pos().absX() > SP.pitchHalfLength()
         || state.ball().pos().absY() > SP.pitchHalfWidth() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX out of pitch" );
#endif

        return - DBL_MAX / 2.0;
    }


    //
    // set basic evaluation
    //
    double point = helios_x_term * state.ball().pos().x;

    point += helios_ball_dist_to_goal_term * 
             std::max( 0.0,
                       helios_effective_max_ball_dist_to_goal 
                       - ServerParam::i().theirTeamGoalPos().dist( state.ball().pos() ) );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) ball pos (%f, %f)",
                  state.ball().pos().x, state.ball().pos().y );

    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) initial value (%f)", point );
#endif

    //
    // add bonus for goal, free situation near offside line
    //
    if ( FieldAnalyzer::can_shoot_from( holder->unum() == state.self().unum(),
                                        holder->pos(),
                                        state.getPlayers( new OpponentOrUnknownPlayerPredicate( state.ourSide() ) ),
                                        VALID_PLAYER_THRESHOLD ) )
    {
        point += 1.0e+6;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) bonus for goal %f (%f)", 1.0e+6, point );
#endif

        if ( holder_unum == state.self().unum() )
        {
            point += 5.0e+5;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN,
                          "(eval) bonus for goal self %f (%f)", 5.0e+5, point );
#endif
        }
    }

    return point;
}

/*-------------------------------------------------------------------*/
/*!

 */
static double evaluate_state_2d( const PredictState & state,
                                 const std::vector < std::vector < double > > & field_evaluator_2d)
{
    const ServerParam & SP = ServerParam::i();

    const AbstractPlayerObject * holder = state.ballHolder();

    auto ball_pos = state.ball().pos();
    ball_pos.x += SP.pitchHalfLength();
    ball_pos.y += SP.pitchHalfWidth();

    int x_size = field_evaluator_2d.size();

    if (x_size == 0)
        return 0;

    int y_size = field_evaluator_2d[0].size();

    if (y_size == 0)
        return 0;

    double x_step = 2.0 * SP.pitchHalfLength() / x_size;
    double y_step = 2.0 * SP.pitchHalfWidth() / y_size;

    int x_index = (int) (ball_pos.x / x_step);
    int y_index = (int) (ball_pos.y / y_step);

    if (x_index < 0)
        x_index = 0;
    if (x_index >= x_size)
        x_index = x_size - 1;
    
    if (y_index < 0)
        y_index = 0;
    if (y_index >= y_size)
        y_index = y_size - 1;

    return field_evaluator_2d[x_index][y_index];
}

