/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    LinearInequalityFactorGraph.h
 * @author  Duy-Nguyen Ta
 * @author  Krunal Chande
 * @author  Luca Carlone
 * @date    Dec 15, 2014
 */

#pragma once
#include <gtsam/linear/VectorValues.h>
#include <gtsam_unstable/linear/InequalityFactorGraph.h>
#include <gtsam_unstable/nonlinear/ConstrainedFactor.h>

namespace gtsam {

/**
 * FactorGraph container for linear inequality factors c(x)<=0 at the nonlinear level
 */
class LinearInequalityFactorGraph: public FactorGraph<NonlinearFactor> {

public:
  /// Default constructor
  LinearInequalityFactorGraph() {
  }

  /// Linearize to a InequalityFactorGraph
  InequalityFactorGraph::shared_ptr linearize(const Values& linearizationPoint) const {
    InequalityFactorGraph::shared_ptr linearGraph(new InequalityFactorGraph());
    BOOST_FOREACH(const NonlinearFactor::shared_ptr& factor, *this) {
      JacobianFactor::shared_ptr jacobian = boost::dynamic_pointer_cast<JacobianFactor>(
          factor->linearize(linearizationPoint));
      ConstrainedFactor::shared_ptr constraint
          = boost::dynamic_pointer_cast<ConstrainedFactor>(factor);
      linearGraph->add(LinearInequality(*jacobian, constraint->dualKey()));
    }
    return linearGraph;
  }

  /**
   * Return true if the all errors are <= 0.0
   */
  bool checkFeasibilityAndComplimentary(const Values& values,
      const VectorValues& dualValues, double tol) const {

    BOOST_FOREACH(const NonlinearFactor::shared_ptr& factor, *this) {
      NoiseModelFactor::shared_ptr noiseModelFactor
          = boost::dynamic_pointer_cast<NoiseModelFactor>(factor);
      Vector error = noiseModelFactor->unwhitenedError(values);

      // Primal feasibility condition: all constraints need to be <= 0.0
      if (error[0] > tol)
        return false;

      // Complimentary condition: errors of active constraints need to be 0.0
      ConstrainedFactor::shared_ptr constraint
          = boost::dynamic_pointer_cast<ConstrainedFactor>(factor);
      Key dualKey = constraint->dualKey();

      // if dualKey doesn't exist in dualValues, it must be an inactive constraint!
      if (!dualValues.exists(dualKey))
        continue;

      // for active constraint, the error should be 0.0
      if (fabs(error[0]) > tol)
        return false;
    }

    return true;
  }
};
}