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

/////////////////////////////////////////////////////////////////////

#ifndef SAMPLE_FIELD_EVALUATOR_H
#define SAMPLE_FIELD_EVALUATOR_H

#include "field_evaluator.h"
#include "predict_state.h"

#include <vector>
#include <map>

#ifdef USE_GRPC
#include "../grpc-generated/service.pb.h"
using protos::PlannerEvalution;
#endif

namespace rcsc {
class AbstractPlayerObject;
class Vector2D;
class WorldModel;
}

class ActionStatePair;

class SampleFieldEvaluator
    : public FieldEvaluator {
private:
    std::vector< double > m_opponent_negetive_effect_by_distance;
    std::vector< double > m_opponent_negetive_effect_by_reach_steps;

    bool m_use_action_term = false;
    double m_direct_pass_term = 1.0;
    double m_lead_pass_term = 1.0;
    double m_through_pass_term = 1.0;
    double m_short_dribble_term = 1.0;
    double m_long_dribble_term = 1.0;
    double m_cross_term = 1.0;
    double m_hold_term = 1.0;

    std::map< int, double > m_teammate_positive_effect;

    // evaluator methods
    bool m_use_heleos = true;
    double m_helios_x_term = 1.0;
    double m_helios_ball_dist_to_goal_term = 1.0;
    double m_helios_effective_max_ball_dist_to_goal = 40.0;

    bool m_use_field_evaluator_2d = false;
    std::vector < std::vector < double > > m_field_evaluator_2d; // [i_x][i_y] 
public:
    SampleFieldEvaluator();

#ifdef USE_GRPC
    void set_grpc_evalution_method( const PlannerEvalution & evalution );
#endif

    virtual
    ~SampleFieldEvaluator();

    virtual
    double operator()( const PredictState & state,
                       const std::vector< ActionStatePair > & path,
                       const rcsc::WorldModel & wm ) const;

    double effected_by_action_term( const PredictState & state,
                                    const std::vector< ActionStatePair > & path,
                                    const double & eval ) const;

    double effected_by_opponent_distance( const PredictState & state,
                                          const double & eval,
                                          const rcsc::WorldModel & wm ) const;
                                 
    double effected_by_opponent_reach_step( const PredictState & state,
                                            const double & eval,
                                            const rcsc::WorldModel & wm ) const;

    double effected_by_teammate( const PredictState & state,
                                 const double & eval ) const;

};

#endif
