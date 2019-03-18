//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//                   Klaus B Sautter
//

#if !defined(KRATOS_CONSTRAINT_UTILITIES)
#define KRATOS_CONSTRAINT_UTILITIES

// System includes

// External includes

// Project includes
#include "includes/model_part.h"

namespace Kratos
{
///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{
/**
 * @namespace ConstraintUtilities
 * @ingroup KratosCore
 * @brief This namespace includes several utilities necessaries for the computation of the MPC
 * @author Vicente Mataix Ferrandiz
 */
namespace ConstraintUtilities
{
    /**
     * @brief This method resets the values of the slave dofs
     * @param rModelPart The model of the problem to solve
     */
    void KRATOS_API(KRATOS_CORE) ResetSlaveDofs(ModelPart& rModelPart);

    /**
     * @brief This method resets the values of the slave dofs
     * @param rModelPart The model of the problem to solve
     */
    void KRATOS_API(KRATOS_CORE) ApplyConstraints(ModelPart& rModelPart);

    /**
     * @brief This method precomputes the contribution of the explicit MPC over nodal residual forces
     * @param rModelPart The model of the problem to solve
     * @param DofVariableName The name of the Dof variable to check
     * @param ResidualDofVariableName The name name of the corresponding residual variable
     */
    void KRATOS_API(KRATOS_CORE) PreComputeExplicitConstraintConstribution(
        ModelPart& rModelPart,
        const std::string DofVariableName = "DISPLACEMENT",
        const std::string ResidualDofVariableName = "FORCE_RESIDUAL"
        );

}; // namespace ConstraintUtilities
}  // namespace Kratos
#endif /* KRATOS_CONSTRAINT_UTILITIES defined */
