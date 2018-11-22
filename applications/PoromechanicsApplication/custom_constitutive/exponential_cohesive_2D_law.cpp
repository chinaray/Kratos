//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Ignasi de Pouplana
//

// Application includes
#include "custom_constitutive/exponential_cohesive_2D_law.hpp"

namespace Kratos
{

void ExponentialCohesive2DLaw::InitializeConstitutiveLawVariables(ConstitutiveLawVariables& rVariables,
                                                                    Parameters& rValues)

{
    const Vector& StrainVector = rValues.GetStrainVector();
    const Properties& MaterialProperties = rValues.GetMaterialProperties();

    rVariables.YieldStress = MaterialProperties[YIELD_STRESS];
    this->ComputeCriticalDisplacement(rVariables,rValues);
    rVariables.PenaltyStiffness = std::exp(1.0)*rVariables.YieldStress/rVariables.CriticalDisplacement;

    const double MinusNormalStrain = -1.0*StrainVector[1];
    rVariables.CompressionMatrix.resize(2,2);
    noalias(rVariables.CompressionMatrix) = ZeroMatrix(2,2);
    if(std::abs(MinusNormalStrain) > 1.0e-15)
        rVariables.CompressionMatrix(1,1) = this->MacaulayBrackets(MinusNormalStrain)/MinusNormalStrain;
    else
        rVariables.CompressionMatrix(1,1) = 0.0;

    const double WeightingParameter = 1.0; // TODO ?
    rVariables.WeightMatrix.resize(2,2);
    noalias(rVariables.WeightMatrix) = ZeroMatrix(2,2);
    rVariables.WeightMatrix(0,0) = WeightingParameter*WeightingParameter;
    if(std::abs(StrainVector[1]) > 1.0e-15)
        rVariables.WeightMatrix(1,1) = this->MacaulayBrackets(StrainVector[1])/StrainVector[1];
    else
        rVariables.WeightMatrix(1,1) = 0.0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void ExponentialCohesive2DLaw::ComputeEquivalentStrain(ConstitutiveLawVariables& rVariables,
                                                        Parameters& rValues)
{
    const Vector& StrainVector = rValues.GetStrainVector();
    const array_1d<double,2> Aux = prod(rVariables.WeightMatrix,StrainVector);
    const double EquivalentStrain2 = inner_prod(StrainVector,Aux);
    if(EquivalentStrain2 > 0.0)
        rVariables.EquivalentStrain = std::sqrt(EquivalentStrain2);
    else
        rVariables.EquivalentStrain = 0.0;
}

//----------------------------------------------------------------------------------------

void ExponentialCohesive2DLaw::ComputeCriticalDisplacement(ConstitutiveLawVariables& rVariables,
                                                            Parameters& rValues)
{
    const Vector& StrainVector = rValues.GetStrainVector();
    const Properties& MaterialProperties = rValues.GetMaterialProperties();

    const double FractureEnergy = MaterialProperties[FRACTURE_ENERGY];
    const double ShearStrain2 = StrainVector[0]*StrainVector[0];
    const double PositiveNormalStrain = this->MacaulayBrackets(StrainVector[1]);
    const double TotalStrain2 = PositiveNormalStrain*PositiveNormalStrain+ShearStrain2;
    double ModeMixingRatio;
    if(TotalStrain2 > 1.0e-15)
        ModeMixingRatio = ShearStrain2 / TotalStrain2;
    else
        ModeMixingRatio = 1.0;
    const double CurveFittingParameter = 1.0; // TODO ?
    const double FractureThoughness = FractureEnergy+(MaterialProperties[SHEAR_FRACTURE_ENERGY]-FractureEnergy)*std::pow(ModeMixingRatio,CurveFittingParameter);

    // TODO Should CriticalDisplacement be calculated with FractureEnergy ?
    rVariables.CriticalDisplacement = FractureThoughness / (std::exp(1.0) * MaterialProperties[YIELD_STRESS]);
}

//----------------------------------------------------------------------------------------

void ExponentialCohesive2DLaw::ComputeConstitutiveMatrix(Matrix& rConstitutiveMatrix,
                                                        ConstitutiveLawVariables& rVariables,
                                                        Parameters& rValues)
{
    const Vector& StrainVector = rValues.GetStrainVector();

    array_1d<double,2> WeightedStrain = prod(rVariables.WeightMatrix,StrainVector);

    noalias(rConstitutiveMatrix) = rVariables.LoadingFunction*std::exp(1.0)*rVariables.YieldStress/(rVariables.CriticalDisplacement*rVariables.CriticalDisplacement)
                                    * 1.0/mStateVariable*std::exp(-mStateVariable/rVariables.CriticalDisplacement)*outer_prod(WeightedStrain,WeightedStrain)
                                    + std::exp(1.0)*rVariables.YieldStress/rVariables.CriticalDisplacement*std::exp(-mStateVariable/rVariables.CriticalDisplacement)*rVariables.WeightMatrix
                                    + rVariables.PenaltyStiffness*rVariables.CompressionMatrix;
}

} // Namespace Kratos
