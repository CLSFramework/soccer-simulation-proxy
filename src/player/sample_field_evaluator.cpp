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
                              const double & helios_x_coefficient = 1.0,
                              const double & helios_ball_dist_to_goal_coefficient = 1.0,
                              const double & helios_effective_max_ball_dist_to_goal = 40.0 );

static double evaluate_state_2d( const PredictState & state,
                                 const std::vector < std::vector < double > > & matrix_field_evaluator);


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
using protos::PlannerEvaluationEffector;
using protos::PlannerFieldEvaluator;
void 
SampleFieldEvaluator::set_grpc_evalution_method( const PlannerEvaluation & evalution )
{
    dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method" );
    auto effectors = evalution.effectors();
    auto field_evaluators = evalution.field_evaluators();

    if ( effectors.has_opponent_effector() )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector" );
        if (effectors.opponent_effector().negetive_effect_by_distance().size() > 0)
        {
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_distance" );
            m_use_opponent_effector_by_distance = true;
            for (auto & value : effectors.opponent_effector().negetive_effect_by_distance()) {
                if (value > 0) {
                    dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_distance to 0: %f", 0.0 );
                    m_opponent_negetive_effect_by_distance.push_back(0);
                }
                else{
                    dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_distance: %f", value );
                    m_opponent_negetive_effect_by_distance.push_back(value);
                }
            }
            m_opponent_negetive_effect_by_distance_based_on_first_layer = effectors.opponent_effector().negetive_effect_by_distance_based_on_first_layer();
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_distance_based_on_first_layer: %d", 
                            m_opponent_negetive_effect_by_distance_based_on_first_layer );
        }
        if (effectors.opponent_effector().negetive_effect_by_reach_steps().size() > 0)
        {
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_reach_steps" );
            m_use_opponent_effector_by_reach_steps = true;
            for (auto & value : effectors.opponent_effector().negetive_effect_by_reach_steps()) {
                if (value > 0) {
                    dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_reach_steps to 0: %f", 0.0 );
                    m_opponent_negetive_effect_by_reach_steps.push_back(0);
                }
                else{
                    dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_reach_steps: %f", value );
                    m_opponent_negetive_effect_by_reach_steps.push_back(value);
                }
            }
            m_opponent_negetive_effect_by_reach_steps_based_on_first_layer = effectors.opponent_effector().negetive_effect_by_reach_steps_based_on_first_layer();
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: opponent effector: negetive_effect_by_reach_steps_based_on_first_layer: %d", 
                            m_opponent_negetive_effect_by_reach_steps_based_on_first_layer );
        }
            
    }
    if ( effectors.has_action_type_effector() )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector" );
        m_use_action_coefficients = true;
        m_direct_pass_coefficient = (effectors.action_type_effector().direct_pass() < 0.0 ? 0.0 : effectors.action_type_effector().direct_pass());
        m_lead_pass_coefficient = (effectors.action_type_effector().lead_pass() < 0.0 ? 0.0 : effectors.action_type_effector().lead_pass());
        m_through_pass_coefficient = (effectors.action_type_effector().through_pass() < 0.0 ? 0.0 : effectors.action_type_effector().through_pass());
        m_short_dribble_coefficient = (effectors.action_type_effector().short_dribble() < 0.0 ? 0.0 : effectors.action_type_effector().short_dribble());
        m_long_dribble_coefficient = (effectors.action_type_effector().long_dribble() < 0.0 ? 0.0 : effectors.action_type_effector().long_dribble());
        m_cross_coefficient = (effectors.action_type_effector().cross() < 0.0 ? 0.0 : effectors.action_type_effector().cross());
        m_hold_coefficient = (effectors.action_type_effector().hold() < 0.0 ? 0.0 : effectors.action_type_effector().hold());
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: direct_pass: %f", m_direct_pass_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: lead_pass: %f", m_lead_pass_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: through_pass: %f", m_through_pass_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: short_dribble: %f", m_short_dribble_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: long_dribble: %f", m_long_dribble_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: cross: %f", m_cross_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: action type effector: hold: %f", m_hold_coefficient );
    }

    if ( effectors.has_teammate_effector() )
        {
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: teammate effector" );
            m_use_teammate_effector = true;
            for ( auto & effect : effectors.teammate_effector().coefficients() )
            {
                m_teammate_positive_coefficients[effect.first] = (effect.second < 0.0 ? 0.0 : effect.second);
                dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: teammate effector: %d: %f", effect.first, effect.second );
            }
            m_teammate_positive_coefficients_based_on_first_layer = effectors.teammate_effector().apply_based_on_first_layer();
        }

    m_use_heleos_field_evaluator = false;
    if ( field_evaluators.has_helios_field_evaluator() )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator" );
        m_use_heleos_field_evaluator = true;
        m_helios_x_coefficient = field_evaluators.helios_field_evaluator().x_coefficient();
        m_helios_x_coefficient = ( m_helios_x_coefficient < 0.0 ? 0.0 : m_helios_x_coefficient );
        m_helios_ball_dist_to_goal_coefficient = field_evaluators.helios_field_evaluator().ball_dist_to_goal_coefficient();
        m_helios_ball_dist_to_goal_coefficient = ( m_helios_ball_dist_to_goal_coefficient < 0.0 ? 0.0 : m_helios_ball_dist_to_goal_coefficient );
        m_helios_effective_max_ball_dist_to_goal = field_evaluators.helios_field_evaluator().effective_max_ball_dist_to_goal();

        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator: x_coefficient: %f", m_helios_x_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator: ball_dist_to_goal_coefficient: %f", m_helios_ball_dist_to_goal_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator: effective_max_ball_dist_to_goal: %f", m_helios_effective_max_ball_dist_to_goal );
    }

    if ( field_evaluators.has_matrix_field_evaluator() )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: matrix field evaluator" );
        m_use_matrix_field_evaluator = true;
        m_matrix_field_evaluator.clear();
        double min_value = std::numeric_limits<double>::max();
        for (const auto& x_row : field_evaluators.matrix_field_evaluator().evals()) {
            for (const auto& y_row : x_row.evals()) {
                min_value = std::min(min_value, static_cast<double>(y_row));
            }
        }
        for ( auto & x_row : field_evaluators.matrix_field_evaluator().evals() )
        {
            std::vector< double > row;
            std::string row_str;
            for ( auto & y_row : x_row.evals() )
            {
                row.push_back( y_row - min_value ); // make all values positive
                row_str += std::to_string( y_row ) + " ";
            }
            m_matrix_field_evaluator.push_back( row );
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: matrix field evaluator: %s", row_str.c_str() );
        }
    }

    if ( !m_use_matrix_field_evaluator)
        m_use_heleos_field_evaluator = true;
}
#endif

#ifdef USE_THRIFT
void 
SampleFieldEvaluator::set_thrift_evalution_method( const soccer::PlannerEvaluation & evalution )
{
    dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method" );
    auto effectors = evalution.effectors;
    auto field_evaluators = evalution.field_evaluators;

    if ( effectors.__isset.opponent_effector )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: opponent effector" );
        if (effectors.opponent_effector.negetive_effect_by_distance.size() > 0)
        {
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: opponent effector: negetive_effect_by_distance" );
            m_use_opponent_effector_by_distance = true;
            m_opponent_negetive_effect_by_distance.clear();
            for (auto & value : effectors.opponent_effector.negetive_effect_by_distance) {
                if (value > 0) {
                    m_opponent_negetive_effect_by_distance.push_back(0);
                }
                else{
                    m_opponent_negetive_effect_by_distance.push_back(value);
                }
                dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: opponent effector: negetive_effect_by_distance: %f", value );
            }
            m_opponent_negetive_effect_by_distance_based_on_first_layer = effectors.opponent_effector.negetive_effect_by_distance_based_on_first_layer;
        }
        if (effectors.opponent_effector.negetive_effect_by_reach_steps.size() > 0)
        {
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: opponent effector: negetive_effect_by_reach_steps" );
            m_use_opponent_effector_by_reach_steps = true;
            m_opponent_negetive_effect_by_reach_steps.clear();
            for (auto & value : effectors.opponent_effector.negetive_effect_by_reach_steps) {
                if (value > 0) {
                    m_opponent_negetive_effect_by_reach_steps.push_back(0);
                }
                else{
                    m_opponent_negetive_effect_by_reach_steps.push_back(value);
                }
                dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: opponent effector: negetive_effect_by_reach_steps: %f", value );
            }
            m_opponent_negetive_effect_by_reach_steps_based_on_first_layer = effectors.opponent_effector.negetive_effect_by_reach_steps_based_on_first_layer;
        }
    }
    if ( effectors.__isset.action_type_effector )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector" );
        m_use_action_coefficients = true;
        m_direct_pass_coefficient = (effectors.action_type_effector.direct_pass < 0.0 ? 0.0 : effectors.action_type_effector.direct_pass);
        m_lead_pass_coefficient = (effectors.action_type_effector.lead_pass < 0.0 ? 0.0 : effectors.action_type_effector.lead_pass);
        m_through_pass_coefficient = (effectors.action_type_effector.through_pass < 0.0 ? 0.0 : effectors.action_type_effector.through_pass);
        m_short_dribble_coefficient = (effectors.action_type_effector.short_dribble < 0.0 ? 0.0 : effectors.action_type_effector.short_dribble);
        m_long_dribble_coefficient = (effectors.action_type_effector.long_dribble < 0.0 ? 0.0 : effectors.action_type_effector.long_dribble);
        m_cross_coefficient = (effectors.action_type_effector.cross < 0.0 ? 0.0 : effectors.action_type_effector.cross);
        m_hold_coefficient = (effectors.action_type_effector.hold < 0.0 ? 0.0 : effectors.action_type_effector.hold);
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: direct_pass: %f", m_direct_pass_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: lead_pass: %f", m_lead_pass_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: through_pass: %f", m_through_pass_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: short_dribble: %f", m_short_dribble_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: long_dribble: %f", m_long_dribble_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: cross: %f", m_cross_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: action type effector: hold: %f", m_hold_coefficient );
    }

    if ( effectors.__isset.teammate_effector )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: teammate effector" );
        m_use_teammate_effector = true;
        for ( auto & effect : effectors.teammate_effector.coefficients )
        {
            m_teammate_positive_coefficients[effect.first] = (effect.second < 0.0 ? 0.0 : effect.second);
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_thrift_evalution_method: teammate effector: %d: %f", effect.first, effect.second );
        }
        m_teammate_positive_coefficients_based_on_first_layer = effectors.teammate_effector.apply_based_on_first_layer;
    }

    m_use_heleos_field_evaluator = false;
    if ( field_evaluators.__isset.helios_field_evaluator )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator" );
        m_use_heleos_field_evaluator = true;
        m_helios_x_coefficient = field_evaluators.helios_field_evaluator.x_coefficient;
        m_helios_x_coefficient = ( m_helios_x_coefficient < 0.0 ? 0.0 : m_helios_x_coefficient );
        m_helios_ball_dist_to_goal_coefficient = field_evaluators.helios_field_evaluator.ball_dist_to_goal_coefficient;
        m_helios_ball_dist_to_goal_coefficient = ( m_helios_ball_dist_to_goal_coefficient < 0.0 ? 0.0 : m_helios_ball_dist_to_goal_coefficient );
        m_helios_effective_max_ball_dist_to_goal = field_evaluators.helios_field_evaluator.effective_max_ball_dist_to_goal;

        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator: x_coefficient: %f", m_helios_x_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator: ball_dist_to_goal_coefficient: %f", m_helios_ball_dist_to_goal_coefficient );
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: helios field evaluator: effective_max_ball_dist_to_goal: %f", m_helios_effective_max_ball_dist_to_goal );
    }

    if ( field_evaluators.__isset.matrix_field_evaluator )
    {
        dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: matrix field evaluator" );
        m_use_matrix_field_evaluator = true;
        m_matrix_field_evaluator.clear();
        double min_value = std::numeric_limits<double>::max();
        for (const auto& x_row : field_evaluators.matrix_field_evaluator.evals) {
            for (const auto& y_row : x_row.evals) {
                min_value = std::min(min_value, static_cast<double>(y_row));
            }
        }
        for ( auto & x_row : field_evaluators.matrix_field_evaluator
        .evals )
        {
            std::vector< double > row;
            std::string row_str;
            for ( auto & y_row : x_row.evals )
            {
                row.push_back( y_row - min_value ); // make all values positive
                row_str += std::to_string( y_row ) + " ";
            }
            m_matrix_field_evaluator.push_back( row );
            dlog.addText( Logger::ANALYZER, "SampleFieldEvaluator::set_grpc_evalution_method: matrix field evaluator: %s", row_str.c_str() );
        }
    }

    if ( !m_use_matrix_field_evaluator)
        m_use_heleos_field_evaluator = true;
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
    
    if ( m_use_heleos_field_evaluator ){
        state_evaluation += evaluate_state( state,
                                            m_helios_x_coefficient,
                                            m_helios_ball_dist_to_goal_coefficient,
                                            m_helios_effective_max_ball_dist_to_goal );
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "eval after helios: %f", state_evaluation );
        #endif
    }
        

    if ( m_use_matrix_field_evaluator )
    {
        state_evaluation += evaluate_state_2d( state, m_matrix_field_evaluator );
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "eval after matrix: %f", state_evaluation );
        #endif
    }

    if ( m_use_action_coefficients )
    {
        state_evaluation = effected_by_action_term( state, path, state_evaluation );
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "eval after action effector: %f", state_evaluation );
        #endif
    }
    
    if ( m_use_opponent_effector_by_distance )
    {
        state_evaluation = effected_by_opponent_distance( state, path, state_evaluation, wm );
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "eval after opponent distance effector: %f", state_evaluation );
        #endif
    }
    
    if ( m_use_opponent_effector_by_reach_steps )
    {
        state_evaluation = effected_by_opponent_reach_step( state, path, state_evaluation, wm );
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "eval after opponent reach step effector: %f", state_evaluation );
        #endif
    }

    if ( m_use_teammate_effector )
    {
        state_evaluation = effected_by_teammate( state, path, state_evaluation );
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "eval after teammate effector: %f", state_evaluation );
        #endif
    }
    
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
        return eval * m_hold_coefficient;
    const ActionStatePair asp = path.at(0);
    if ( asp.action().description() == "strictDirect" )
        return eval * m_direct_pass_coefficient;
    else if ( asp.action().description() == "strictLead" )
        return eval * m_lead_pass_coefficient;
    else if ( asp.action().description() == "strictThrough" )
        return eval * m_through_pass_coefficient;
    else if ( asp.action().description() == "shortDribble" )
        return eval * m_short_dribble_coefficient;
    else if ( asp.action().description() == "SelfPass" )
        return eval * m_long_dribble_coefficient;
    else if ( asp.action().description() == "cross" )
        return eval * m_cross_coefficient;
    return eval;
}

double 
SampleFieldEvaluator::effected_by_opponent_distance( const PredictState & state,
                                                     const std::vector< ActionStatePair > & path,
                                                     const double & eval,
                                                     const WorldModel & wm ) const
{
    if ( m_opponent_negetive_effect_by_distance.size() == 0 )
        return eval;
    auto ball_pos = state.ball().pos();
    if ( path.size() > 0 && m_opponent_negetive_effect_by_distance_based_on_first_layer )
    {
        ball_pos = path.at(0).state().ball().pos();
    }

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

    double min_effect = *std::min_element(m_opponent_negetive_effect_by_distance.begin(), m_opponent_negetive_effect_by_distance.end());
    double new_eval = eval - min_effect;
    if ( min_dist_int >= m_opponent_negetive_effect_by_distance.size() )
    {
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "opp dist effect -> min_dist_int: %d > m_opponent_negetive_effect_by_distance.size(): %d", min_dist_int, m_opponent_negetive_effect_by_distance.size() );
        #endif
        return new_eval;
    }
        
    #ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN, "opp dist effect -> min_dist_int: %d, min_effect: %f, eval: %f, effect: %f", min_dist_int, min_effect, new_eval, m_opponent_negetive_effect_by_distance[min_dist_int] );
    #endif
    
    return new_eval + m_opponent_negetive_effect_by_distance[min_dist_int];
}

double 
SampleFieldEvaluator::effected_by_opponent_reach_step( const PredictState & state,
                                                       const std::vector< ActionStatePair > & path,
                                                       const double & eval,
                                                       const WorldModel & wm ) const
{
    if ( m_opponent_negetive_effect_by_reach_steps.size() == 0 )
        return eval;
    auto ball_pos = state.ball().pos();
    if ( path.size() > 0 && m_opponent_negetive_effect_by_reach_steps_based_on_first_layer )
    {
        ball_pos = path.at(0).state().ball().pos();
    }

    int min_reach = 1000;

    for ( auto & opp : wm.theirPlayers() )
    {
        if ( opp->unum() <= 0 )
            continue;

        double reach = opp->playerTypePtr()->cyclesToReachDistance( ball_pos.dist( opp->pos() ) );

        if ( reach < min_reach )
            min_reach = reach;
    }

    double min_effect = *std::min_element(m_opponent_negetive_effect_by_reach_steps.begin(), m_opponent_negetive_effect_by_reach_steps.end());
    double new_eval = eval - min_effect;

    if ( min_reach >= m_opponent_negetive_effect_by_reach_steps.size() )
    {
        #ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN, "opp reach effect -> min_reach: %d > m_opponent_negetive_effect_by_reach_steps.size(): %d", min_reach, m_opponent_negetive_effect_by_reach_steps.size() );
        #endif
        return new_eval;
    }
        
    #ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN, "opp reach effect -> min_reach: %d, min_effect: %f, eval: %f, effect: %f", min_reach, min_effect, new_eval, m_opponent_negetive_effect_by_reach_steps[min_reach] );
    #endif
    
    return new_eval + m_opponent_negetive_effect_by_reach_steps[min_reach];
}

double 
SampleFieldEvaluator::effected_by_teammate( const PredictState & state,
                                            const std::vector< ActionStatePair > & path,
                                            const double & eval ) const
{
    if ( m_teammate_positive_coefficients.size() == 0 )
        return eval;
    const ServerParam & SP = ServerParam::i();

    int unum = 0;
    if (m_teammate_positive_coefficients_based_on_first_layer && path.size() > 0)
    {
        const ActionStatePair asp = path.at(0);
        const AbstractPlayerObject * holder = state.ballHolder();
        if ( ! holder)
            unum = holder->unum();
    }
    else
    {
        const AbstractPlayerObject * holder = state.ballHolder();
        if ( ! holder)
            unum = holder->unum();
    }
    
    if ( m_teammate_positive_coefficients.find( unum ) == m_teammate_positive_coefficients.end() )
        return eval;
    
    return eval * m_teammate_positive_coefficients.at( unum );
}
/*-------------------------------------------------------------------*/
/*!

 */
static
double
evaluate_state( const PredictState & state,
                const double & helios_x_coefficient,
                const double & helios_ball_dist_to_goal_coefficient,
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
    double point = helios_x_coefficient * (state.ball().pos().x + SP.pitchHalfLength());

    point += helios_ball_dist_to_goal_coefficient * 
             std::max( 0.0,
                       helios_effective_max_ball_dist_to_goal 
                       - ServerParam::i().theirTeamGoalPos().dist( state.ball().pos() ) );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) ball pos (%f, %f)",
                  state.ball().pos().x, state.ball().pos().y );
    dlog.addText( Logger::ACTION_CHAIN,
                  "helios_x_coefficient: %f, helios_ball_dist_to_goal_coefficient: %f, helios_effective_max_ball_dist_to_goal: %f",
                  helios_x_coefficient, helios_ball_dist_to_goal_coefficient, helios_effective_max_ball_dist_to_goal );
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
                                 const std::vector < std::vector < double > > & matrix_field_evaluator)
{
    const ServerParam & SP = ServerParam::i();

    const AbstractPlayerObject * holder = state.ballHolder();

    auto ball_pos = state.ball().pos();
    ball_pos.x += SP.pitchHalfLength();
    ball_pos.y += SP.pitchHalfWidth();

    int x_size = matrix_field_evaluator.size();

    if (x_size == 0)
        return 0;

    int y_size = matrix_field_evaluator[0].size();

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

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) ball pos (%f, %f)",
                  state.ball().pos().x, state.ball().pos().y );
    dlog.addText( Logger::ACTION_CHAIN,
                  "matrix_field_evaluator size: %d, %d", x_size, y_size );
    dlog.addText( Logger::ACTION_CHAIN,
                  "matrix_field_evaluator step: %f, %f", x_step, y_step );
    dlog.addText( Logger::ACTION_CHAIN,
                  "matrix_field_evaluator index: %d, %d", x_index, y_index );
    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) initial value (%f)", matrix_field_evaluator[x_index][y_index] );
#endif
    return matrix_field_evaluator[x_index][y_index];
}

