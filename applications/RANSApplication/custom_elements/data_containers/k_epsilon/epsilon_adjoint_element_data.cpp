//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//

// System includes

// Project includes
#include "geometries/geometry_data.h"
#include "includes/ublas_interface.h"
#include "includes/variables.h"
#include "includes/checks.h"
#include "includes/cfd_variables.h"

// Application includes
#include "custom_elements/data_containers/k_epsilon/adjoint_element_data_utilities.h"
#include "custom_elements/data_containers/k_epsilon/element_data_utilities.h"
#include "custom_utilities/rans_calculation_utilities.h"
#include "rans_application_variables.h"

// Include base h
#include "epsilon_adjoint_element_data.h"

namespace Kratos
{
namespace KEpsilonAdjointElementData
{

//***********************************************************************************//
//************************** Adjoint Element Data ***********************************//

template <unsigned int TDim, unsigned int TNumNodes>
EpsilonAdjointStateDerivatives<TDim, TNumNodes>::Data::Data(
    const GeometryType& rGeometry)
    : BaseType(rGeometry)
{
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::Data::CalculateGaussPointData(
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const int Step)
{
    BaseType::CalculateGaussPointData(rShapeFunctions, rShapeFunctionDerivatives, Step);

    mProductionTerm = KEpsilonElementData::CalculateSourceTerm<TDim>(this->mVelocityGradient, this->mTurbulentKinematicViscosity);
    mEffectiveKinematicViscosity = this->CalculateEffectiveKinematicViscosity(rShapeFunctions, rShapeFunctionDerivatives);
    mReactionTerm = this->CalculateReactionTerm(rShapeFunctions, rShapeFunctionDerivatives);

    BoundedVector<double, TNumNodes> nodal_tke;
    BoundedVector<double, TNumNodes> nodal_epsilon;

    for (std::size_t i_node = 0; i_node < TNumNodes; ++i_node) {
        const auto& r_node = this->GetGeometry()[i_node];
        const array_1d<double, 3>& rVelocity =
            r_node.FastGetSolutionStepValue(VELOCITY, Step);
        for (unsigned int i_dim = 0; i_dim < TDim; ++i_dim) {
            mNodalVelocity(i_node, i_dim) = rVelocity[i_dim];
        }
        nodal_tke[i_node] = r_node.FastGetSolutionStepValue(TURBULENT_KINETIC_ENERGY, Step);
        nodal_epsilon[i_node] = r_node.FastGetSolutionStepValue(TURBULENT_ENERGY_DISSIPATION_RATE, Step);
    }

    KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateNodalNutTKESensitivity(
        this->mGaussTurbulentKinematicViscositySensitivitiesK, this->mCmu,
        nodal_tke, nodal_epsilon);
    KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateNodalNutEpsilonSensitivity(
        this->mGaussTurbulentKinematicViscositySensitivitiesEpsilon, this->mCmu,
        nodal_tke, nodal_epsilon);

    RansCalculationUtilities::CalculateGaussSensitivities<TNumNodes>(
        this->mGaussTurbulentKinematicViscositySensitivitiesK,
        this->mGaussTurbulentKinematicViscositySensitivitiesK, rShapeFunctions);
    RansCalculationUtilities::CalculateGaussSensitivities<TNumNodes>(
        this->mGaussTurbulentKinematicViscositySensitivitiesEpsilon,
        this->mGaussTurbulentKinematicViscositySensitivitiesEpsilon, rShapeFunctions);
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::Data::Check(
    const GeometryType& rGeometry,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY
    const int number_of_nodes = rGeometry.PointsNumber();

    for (int i_node = 0; i_node < number_of_nodes; ++i_node) {
        const auto& r_node = rGeometry[i_node];
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(KINEMATIC_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE_2, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_AUXILIARY_VARIABLE_2, r_node);

        KRATOS_CHECK_DOF_IN_NODE(RANS_SCALAR_2_ADJOINT_1, r_node);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& EpsilonAdjointStateDerivatives<TDim, TNumNodes>::Data::GetAdjointScalarVariable()
{
    return RANS_SCALAR_2_ADJOINT_1;
}

//************************** Adjoint Element Data ***********************************//
//***********************************************************************************//
//************************** Velocity Derivatives ***********************************//

template <unsigned int TDim, unsigned int TNumNodes>
EpsilonAdjointStateDerivatives<TDim, TNumNodes>::VelocityDerivatives::VelocityDerivatives(
    const Data& rElementData)
    : BaseType(rElementData)
{
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<array_1d<double, 3>>& EpsilonAdjointStateDerivatives<TDim, TNumNodes>::VelocityDerivatives::GetDerivativeVariable()
{
    return VELOCITY;
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::VelocityDerivatives::CalculateEffectiveVelocityDerivatives(
    BoundedMatrix<double, TDerivativesSize, 3>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    rOutput.clear();

    for (IndexType c = 0; c < TNumNodes; ++c) {
        const auto block_row = c * TDim;
        for (IndexType k = 0; k < TDim; ++k) {
            rOutput(block_row + k, k) = rShapeFunctions[c];
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::VelocityDerivatives::CalculateEffectiveKinematicViscosityDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    rOutput.clear();
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::VelocityDerivatives::CalculateReactionTermDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();
    if (data.mReactionTerm > 0.0) {
        for (IndexType c = 0; c < TNumNodes; ++c) {
            const auto block_row = c * TDim;
            for (IndexType k = 0; k < TDim; ++k) {
                rOutput[block_row + k] = data.mC1 * rShapeFunctionDerivatives(c, k) * (2.0 / 3.0);
            }
        }
    } else {
        rOutput.clear();
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::VelocityDerivatives::CalculateSourceTermDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();

    KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateProductionVelocitySensitivities(
        rOutput, data.mTurbulentKinematicViscosity, data.mProductionTerm,
        data.mVelocityGradient, rShapeFunctionDerivatives);
    noalias(rOutput) = rOutput * (data.mC1 * data.mGamma);
}

//************************** Velocity Derivatives ***********************************//
//***********************************************************************************//
//***************************** K Derivatives ***************************************//

template <unsigned int TDim, unsigned int TNumNodes>
EpsilonAdjointStateDerivatives<TDim, TNumNodes>::KDerivatives::KDerivatives(
    const Data& rElementData)
    : BaseType(rElementData)
{
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& EpsilonAdjointStateDerivatives<TDim, TNumNodes>::KDerivatives::GetDerivativeVariable()
{
    return TURBULENT_KINETIC_ENERGY;
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::KDerivatives::CalculateEffectiveVelocityDerivatives(
    BoundedMatrix<double, TDerivativesSize, 3>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    rOutput.clear();
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::KDerivatives::CalculateEffectiveKinematicViscosityDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();
    noalias(rOutput) =
        data.mGaussTurbulentKinematicViscositySensitivitiesK * data.mInvEpsilonSigma;
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::KDerivatives::CalculateReactionTermDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();

    if (data.mReactionTerm > 0.0) {
        KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateGaussGammaTkeSensitivity(
            rOutput, data.mCmu, data.mGamma, data.mTurbulentKinematicViscosity,
            data.mGaussTurbulentKinematicViscositySensitivitiesK, rShapeFunctions);
        noalias(rOutput) = rOutput * data.mC2;
    } else {
        rOutput.clear();
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::KDerivatives::CalculateSourceTermDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();

    BoundedVector<double, TDerivativesSize> production_sensitivities;

    KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateProductionScalarSensitivities(
        production_sensitivities, data.mTurbulentKinematicViscosity, data.mProductionTerm,
        data.mGaussTurbulentKinematicViscositySensitivitiesK);

    KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateGaussGammaTkeSensitivity(
        rOutput, data.mCmu, data.mGamma, data.mTurbulentKinematicViscosity,
        data.mGaussTurbulentKinematicViscositySensitivitiesK, rShapeFunctions);

    noalias(rOutput) = rOutput * data.mProductionTerm + production_sensitivities * data.mGamma;
    noalias(rOutput) = rOutput * data.mC1;
}

//***************************** K Derivatives ***************************************//
//***********************************************************************************//
//************************** Epsilon Derivatives ************************************//

template <unsigned int TDim, unsigned int TNumNodes>
EpsilonAdjointStateDerivatives<TDim, TNumNodes>::EpsilonDerivatives::EpsilonDerivatives(
    const Data& rElementData)
    : BaseType(rElementData)
{
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& EpsilonAdjointStateDerivatives<TDim, TNumNodes>::EpsilonDerivatives::GetDerivativeVariable()
{
    return TURBULENT_ENERGY_DISSIPATION_RATE;
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::EpsilonDerivatives::CalculateEffectiveVelocityDerivatives(
    BoundedMatrix<double, TDerivativesSize, 3>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    rOutput.clear();
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::EpsilonDerivatives::CalculateEffectiveKinematicViscosityDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();
    noalias(rOutput) =
        data.mGaussTurbulentKinematicViscositySensitivitiesEpsilon * data.mInvEpsilonSigma;
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::EpsilonDerivatives::CalculateReactionTermDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    const auto& data = this->GetElementData();

    if (data.mReactionTerm > 0.0) {
        KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateGaussGammaEpsilonSensitivity(
            rOutput, data.mGamma, data.mTurbulentKinematicViscosity,
            data.mGaussTurbulentKinematicViscositySensitivitiesEpsilon);
        noalias(rOutput) = rOutput * data.mC2;
    } else {
        rOutput.clear();
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointStateDerivatives<TDim, TNumNodes>::EpsilonDerivatives::CalculateSourceTermDerivatives(
    BoundedVector<double, TDerivativesSize>& rOutput,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives) const
{
    rOutput.clear();
}

//************************** Epsilon Derivatives ************************************//
//***********************************************************************************//
//**************************** Shape Derivatives ************************************//

template <unsigned int TDim, unsigned int TNumNodes>
EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::ShapeDerivatives(
    const GeometryType& rGeometry)
    : BaseType(rGeometry)
{
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::CalculateGaussPointData(
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const int Step)
{
    BaseType::CalculateGaussPointData(rShapeFunctions, rShapeFunctionDerivatives, Step);

    mProductionTerm = this->CalculateSourceTerm(rShapeFunctions, rShapeFunctionDerivatives);
    mReactionTerm = this->CalculateReactionTerm(rShapeFunctions, rShapeFunctionDerivatives);

    for (std::size_t i_node = 0; i_node < TNumNodes; ++i_node) {
        const auto& r_node = this->GetGeometry()[i_node];
        const array_1d<double, 3>& rVelocity =
            r_node.FastGetSolutionStepValue(VELOCITY, Step);
        for (unsigned int i_dim = 0; i_dim < TDim; ++i_dim) {
            mNodalVelocity(i_node, i_dim) = rVelocity[i_dim];
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::Check(
    const GeometryType& rGeometry,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY
    const int number_of_nodes = rGeometry.PointsNumber();

    for (int i_node = 0; i_node < number_of_nodes; ++i_node) {
        const auto& r_node = rGeometry[i_node];
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(KINEMATIC_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_ENERGY_DISSIPATION_RATE_2, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(RANS_AUXILIARY_VARIABLE_2, r_node);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<double>& EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::GetAdjointScalarVariable()
{
    return RANS_SCALAR_2_ADJOINT_1;
}

template <unsigned int TDim, unsigned int TNumNodes>
const Variable<array_1d<double, 3>>& EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::GetDerivativeVariable()
{
    return SHAPE_SENSITIVITY;
}

template <unsigned int TDim, unsigned int TNumNodes>
array_1d<double, 3> EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::CalculateEffectiveVelocityDerivatives(
    const ShapeParameter& rShapeParameters,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv) const
{
    return ZeroVector(3);
}

template <unsigned int TDim, unsigned int TNumNodes>
double EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::CalculateEffectiveKinematicViscosityDerivatives(
    const ShapeParameter& rShapeParameters,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv) const
{
    return 0.0;
}

template <unsigned int TDim, unsigned int TNumNodes>
double EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::CalculateReactionTermDerivatives(
    const ShapeParameter& rShapeParameters,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv) const
{
    if (this->mReactionTerm > 0.0) {
        return (this->mC1 * 2.0 / 3.0) * RansCalculationUtilities::GetDivergence(
                                             this->GetGeometry(), VELOCITY, rDN_Dx_deriv);
    } else {
        return 0.0;
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
double EpsilonAdjointSensitivityDerivatives<TDim, TNumNodes>::ShapeDerivatives::CalculateSourceTermDerivatives(
    const ShapeParameter& rShapeParameters,
    const Vector& rShapeFunctions,
    const Matrix& rShapeFunctionDerivatives,
    const double detJ_deriv,
    const GeometricalSensitivityUtility::ShapeFunctionsGradientType& rDN_Dx_deriv) const
{
    return KEpsilonElementData::AdjointUtilities<TDim, TNumNodes>::CalculateProductionShapeSensitivities(
               this->mTurbulentKinematicViscosity, 0.0, this->mProductionTerm,
               this->mNodalVelocity, rShapeFunctionDerivatives, rDN_Dx_deriv) *
           this->mC1 * this->mGamma;
}

//*************************** Shape Sensitivity *************************************//
//***********************************************************************************//

// template instantiations
template class EpsilonAdjointStateDerivatives<2, 3>;
template class EpsilonAdjointStateDerivatives<3, 4>;

template class EpsilonAdjointSensitivityDerivatives<2, 3>;
template class EpsilonAdjointSensitivityDerivatives<3, 4>;

} // namespace KEpsilonAdjointElementData
} // namespace Kratos