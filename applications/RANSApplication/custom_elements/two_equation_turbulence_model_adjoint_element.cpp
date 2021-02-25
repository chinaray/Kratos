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
#include <sstream>

// External includes

// Project includes
#include "includes/checks.h"
#include "includes/element.h"
#include "includes/properties.h"
#include "utilities/adjoint_extensions.h"
#include "utilities/geometrical_sensitivity_utility.h"
#include "utilities/indirect_scalar.h"

// Application includes
#include "fluid_dynamics_application_variables.h"
#include "rans_application_variables.h"

// data containers
#include "custom_elements/data_containers/k_epsilon/qsvms_rfc_adjoint_element_data.h"

// Include base h
#include "two_equation_turbulence_model_adjoint_element.h"

namespace Kratos
{
template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::ThisExtensions(Element* pElement)
    : mpElement{pElement}
{
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::GetFirstDerivativesVector(
    std::size_t NodeId,
    std::vector<IndirectScalar<double>>& rVector,
    std::size_t Step)
{
    auto& r_node = mpElement->GetGeometry()[NodeId];
    rVector.resize(TDim + 3);

    const auto& dofs_list = TAdjointElementData::GetDofVariablesList();

    for (unsigned int i = 0; i < TDim; ++i) {
        rVector[i] = MakeIndirectScalar(r_node, (*dofs_list[i]).GetTimeDerivative(), Step);
    }

    rVector[TDim] = IndirectScalar<double>{}; // pressure
    rVector[TDim + 1] = MakeIndirectScalar(r_node, (*dofs_list[TDim + 1]).GetTimeDerivative(), Step);
    rVector[TDim + 2] = MakeIndirectScalar(r_node, (*dofs_list[TDim + 2]).GetTimeDerivative(), Step);

}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::GetSecondDerivativesVector(
    std::size_t NodeId,
    std::vector<IndirectScalar<double>>& rVector,
    std::size_t Step)
{
    auto& r_node = mpElement->GetGeometry()[NodeId];
    rVector.resize(TDim + 3);

    const auto& dofs_list = TAdjointElementData::GetDofVariablesList();

    for (unsigned int i = 0; i < TDim; ++i) {
        rVector[i] = MakeIndirectScalar(r_node, (*dofs_list[i]).GetTimeDerivative().GetTimeDerivative(), Step);
    }

    rVector[TDim] = IndirectScalar<double>{}; // pressure
    rVector[TDim + 1] = MakeIndirectScalar(r_node, (*dofs_list[TDim + 1]).GetTimeDerivative().GetTimeDerivative(), Step);
    rVector[TDim + 2] = MakeIndirectScalar(r_node, (*dofs_list[TDim + 2]).GetTimeDerivative().GetTimeDerivative(), Step);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::GetAuxiliaryVector(
    std::size_t NodeId,
    std::vector<IndirectScalar<double>>& rVector,
    std::size_t Step)
{
    auto& r_node = mpElement->GetGeometry()[NodeId];
    rVector.resize(TDim + 3);

    const auto& dofs_list = TAdjointElementData::GetDofVariablesList();

    for (unsigned int i = 0; i < TDim; ++i) {
        rVector[i] = MakeIndirectScalar(r_node, (*dofs_list[i]).GetTimeDerivative().GetTimeDerivative().GetTimeDerivative(), Step);
    }

    rVector[TDim] = IndirectScalar<double>{}; // pressure
    rVector[TDim + 1] = MakeIndirectScalar(r_node, (*dofs_list[TDim + 1]).GetTimeDerivative().GetTimeDerivative().GetTimeDerivative(), Step);
    rVector[TDim + 2] = MakeIndirectScalar(r_node, (*dofs_list[TDim + 2]).GetTimeDerivative().GetTimeDerivative().GetTimeDerivative(), Step);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::GetFirstDerivativesVariables(
    std::vector<VariableData const*>& rVariables) const
{
    rVariables.resize(3);
    rVariables[0] = &ADJOINT_FLUID_VECTOR_2;
    rVariables[1] = &RANS_SCALAR_1_ADJOINT_2;
    rVariables[2] = &RANS_SCALAR_2_ADJOINT_2;
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::GetSecondDerivativesVariables(
    std::vector<VariableData const*>& rVariables) const
{
    rVariables.resize(3);
    rVariables[0] = &ADJOINT_FLUID_VECTOR_3;
    rVariables[1] = &RANS_SCALAR_1_ADJOINT_3;
    rVariables[2] = &RANS_SCALAR_2_ADJOINT_3;
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::ThisExtensions::GetAuxiliaryVariables(
    std::vector<VariableData const*>& rVariables) const
{
    rVariables.resize(3);
    rVariables[0] = &AUX_ADJOINT_FLUID_VECTOR_1;
    rVariables[1] = &RANS_AUX_ADJOINT_SCALAR_1;
    rVariables[2] = &RANS_AUX_ADJOINT_SCALAR_2;
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::TwoEquationTurbulenceModelAdjointElement(IndexType NewId)
    : BaseType(NewId)
{
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::TwoEquationTurbulenceModelAdjointElement(
    IndexType NewId,
    GeometryType::Pointer pGeometry)
    : BaseType(NewId, pGeometry)
{
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::TwoEquationTurbulenceModelAdjointElement(
    IndexType NewId,
    GeometryType::Pointer pGeometry,
    PropertiesType::Pointer pProperties)
    : BaseType(NewId, pGeometry, pProperties)
{
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::~TwoEquationTurbulenceModelAdjointElement()
{
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
Element::Pointer TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Create(
    IndexType NewId,
    NodesArrayType const& ThisNodes,
    PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>>(
        NewId, Element::GetGeometry().Create(ThisNodes), pProperties);
    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
Element::Pointer TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Create(
    IndexType NewId,
    GeometryType::Pointer pGeom,
    PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>>(NewId, pGeom, pProperties);
    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
Element::Pointer TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Clone(
    IndexType NewId,
    NodesArrayType const& ThisNodes) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>>(
        NewId, Element::GetGeometry().Create(ThisNodes), Element::pGetProperties());
    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
int TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Check(const ProcessInfo& rCurrentProcessInfo) const
{
    TAdjointElementData::Check(*this, rCurrentProcessInfo);
    return 0.0;
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::EquationIdVector(
    EquationIdVectorType& rElementalEquationIdList,
    const ProcessInfo& rCurrentProcessInfo) const
{
    if (rElementalEquationIdList.size() != TElementLocalSize) {
        rElementalEquationIdList.resize(TElementLocalSize, false);
    }

    const auto& r_variables_list = TAdjointElementData::GetDofVariablesList();

    IndexType local_index = 0;
    for (IndexType i = 0; i < TNumNodes; ++i) {
        const auto& r_node = this->GetGeometry()[i];
        for (const auto p_variable : r_variables_list) {
            rElementalEquationIdList[local_index++] = r_node.GetDof(*p_variable).EquationId();
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::GetDofList(
    DofsVectorType& rElementalDofList,
    const ProcessInfo& rCurrentProcessInfo) const
{
    if (rElementalDofList.size() != TElementLocalSize) {
        rElementalDofList.resize(TElementLocalSize);
    }

    const auto& r_variables_list = TAdjointElementData::GetDofVariablesList();

    IndexType local_index = 0;
    for (IndexType i = 0; i < TNumNodes; ++i) {
        const auto& r_node = this->GetGeometry()[i];
        for (const auto p_variable : r_variables_list) {
            rElementalDofList[local_index++] = r_node.pGetDof(*p_variable);
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::GetValuesVector(
    VectorType& rValues,
    int Step) const
{
    if (rValues.size() != TElementLocalSize) {
        rValues.resize(TElementLocalSize, false);
    }

    const auto& r_variables_list = TAdjointElementData::GetDofVariablesList();

    IndexType local_index = 0;
    for (IndexType i = 0; i < TNumNodes; ++i) {
        const auto& r_node = this->GetGeometry()[i];
        for (const auto p_variable : r_variables_list) {
            rValues[local_index++] = r_node.FastGetSolutionStepValue(*p_variable, Step);
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::GetFirstDerivativesVector(
    VectorType& rValues,
    int Step) const
{
    if (rValues.size() != TElementLocalSize) {
        rValues.resize(TElementLocalSize, false);
    }
    rValues.clear();
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::GetSecondDerivativesVector(
    VectorType& rValues,
    int Step) const
{
    if (rValues.size() != TElementLocalSize) {
        rValues.resize(TElementLocalSize, false);
    }

    const auto& r_variables_list = TAdjointElementData::GetDofVariablesList();

    IndexType local_index = 0;
    for (IndexType i = 0; i < TNumNodes; ++i) {
        const auto& r_node = this->GetGeometry()[i];
        for (IndexType j = 0; j < TDim; ++j) {
            rValues[local_index++] = r_node.FastGetSolutionStepValue(r_variables_list[j]->GetTimeDerivative().GetTimeDerivative(), Step);
        }

        rValues[local_index++] = 0.0; // pressure dof
        rValues[local_index++] = r_node.FastGetSolutionStepValue(r_variables_list[TDim+1]->GetTimeDerivative().GetTimeDerivative(), Step);
        rValues[local_index++] = r_node.FastGetSolutionStepValue(r_variables_list[TDim+2]->GetTimeDerivative().GetTimeDerivative(), Step);
    }
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Initialize(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    // If we are restarting, the constitutive law will be already defined
    if (mpConstitutiveLaw == nullptr) {
        const Properties& r_properties = this->GetProperties();

        KRATOS_ERROR_IF_NOT(r_properties.Has(CONSTITUTIVE_LAW))
            << "In initialization of Element " << this->Info()
            << ": No CONSTITUTIVE_LAW defined for property "
            << r_properties.Id() << "." << std::endl;

        // Here we can do down casting because, it should be always a FluidConstitutiveLaw
        mpConstitutiveLaw = r_properties[CONSTITUTIVE_LAW]->Clone();

        const GeometryType& r_geometry = this->GetGeometry();
        const auto& r_shape_functions =
            row(r_geometry.ShapeFunctionsValues(GeometryData::GI_GAUSS_1), 0);

        // This constitutive law should return nu + nu_t
        mpConstitutiveLaw->InitializeMaterial(r_properties, r_geometry, r_shape_functions);

        // Now we set the constitutive law to be used by the RANS equations
        // because RANS equations only need nu from constitutive law

        const auto rans_cl_name = mpConstitutiveLaw->Info();

        KRATOS_ERROR_IF(rans_cl_name.substr(0, 4) != "Rans")
            << "Incompatible constitutive law is used. Please use constitutive "
            "laws which starts with \"Rans*\" [ Constitutive law "
            "name = "
            << rans_cl_name << " ].\n";

    }

    this->SetValue(ADJOINT_EXTENSIONS, Kratos::make_shared<ThisExtensions>(this));

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateLocalSystem(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    // Resize and initialize output
    if (rLeftHandSideMatrix.size1() != 0)
        rLeftHandSideMatrix.resize(0, 0, false);

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateLeftHandSide(
    MatrixType& rLeftHandSideMatrix,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != TElementLocalSize ||
        rLeftHandSideMatrix.size2() != TElementLocalSize) {
        rLeftHandSideMatrix.resize(TElementLocalSize, TElementLocalSize, false);
    }

    rLeftHandSideMatrix.clear();
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateRightHandSide(
    VectorType& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    KRATOS_ERROR << "TwoEquationTurbulenceModelAdjointElement::"
                    "CalculateRightHandSide method is not implemented.";

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateLocalVelocityContribution(
    MatrixType &rDampMatrix,
    VectorType &rRightHandSideVector,
    const ProcessInfo &rCurrentProcessInfo)
{
    if (rRightHandSideVector.size() != TElementLocalSize)
        rRightHandSideVector.resize(TElementLocalSize, false);

    rRightHandSideVector.clear();

    AddFluidResidualsContributions(rRightHandSideVector, rCurrentProcessInfo);
    AddTurbulenceResidualsContributions(rRightHandSideVector, rCurrentProcessInfo);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateFirstDerivativesLHS(
    MatrixType& rLeftHandSideMatrix,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != TElementLocalSize ||
        rLeftHandSideMatrix.size2() != TElementLocalSize) {
        rLeftHandSideMatrix.resize(TElementLocalSize, TElementLocalSize, false);
    }

    rLeftHandSideMatrix.clear();

    AddFluidFirstDerivatives(rLeftHandSideMatrix, rCurrentProcessInfo);
    AddTurbulenceFirstDerivatives(rLeftHandSideMatrix, rCurrentProcessInfo);

}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateSecondDerivativesLHS(
    MatrixType& rLeftHandSideMatrix,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != TElementLocalSize ||
        rLeftHandSideMatrix.size2() != TElementLocalSize) {
        rLeftHandSideMatrix.resize(TElementLocalSize, TElementLocalSize, false);
    }

    rLeftHandSideMatrix.clear();

    AddFluidSecondDerivatives(rLeftHandSideMatrix, rCurrentProcessInfo);
    AddTurbulenceSecondDerivatives(rLeftHandSideMatrix, rCurrentProcessInfo);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateMassMatrix(
    MatrixType& rMassMatrix,
    const ProcessInfo& rCurrentProcessInfo)
{
    rMassMatrix.resize(0, 0);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateDampingMatrix(
    MatrixType& rDampingMatrix,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    KRATOS_ERROR << "TwoEquationTurbulenceModelAdjointElement::"
                    "CalculateDampingMatrix method is not implemented.";

    KRATOS_CATCH("")
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateSensitivityMatrix(
    const Variable<array_1d<double, 3>>& rSensitivityVariable,
    Matrix& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    if (rSensitivityVariable == SHAPE_SENSITIVITY) {
        if (rOutput.size1() != TCoordLocalSize || rOutput.size2() != TElementLocalSize) {
            rOutput.resize(TCoordLocalSize, TElementLocalSize, false);
        }

        rOutput.clear();
        AddFluidShapeDerivatives(rOutput, rCurrentProcessInfo);
        AddTurbulenceShapeDerivatives(rOutput, rCurrentProcessInfo);
    } else {
        KRATOS_ERROR << "Sensitivity variable " << rSensitivityVariable
                     << " not supported." << std::endl;
    }

    KRATOS_CATCH("")
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Calculate(
    const Variable<Vector>& rVariable,
    Vector& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    if (rVariable == PRIMAL_RELAXED_SECOND_DERIVATIVE_VALUES) {

        if (rOutput.size() != TElementLocalSize) {
            rOutput.resize(TElementLocalSize, false);
        }

        const auto& r_variables_list = TAdjointElementData::GetPrimalSecondDerivativeVariablesList();

        IndexType local_index = 0;
        for (IndexType i = 0; i < TNumNodes; ++i) {
            const auto& r_node = this->GetGeometry()[i];
            for (IndexType j = 0; j < TDim; ++j) {
                rOutput[local_index++] = r_node.GetValue(*r_variables_list[j]);
            }

            rOutput[local_index++] = 0.0; // pressure dof
            rOutput[local_index++] = r_node.FastGetSolutionStepValue(*r_variables_list[TDim+1]);
            rOutput[local_index++] = r_node.FastGetSolutionStepValue(*r_variables_list[TDim+2]);
        }
    } else {
        KRATOS_ERROR << "Unsupported variable requested for Calculate method. "
                        "[ rVariable.Name() = "
                     << rVariable.Name() << " ].\n";
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
std::string TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::Info() const
{
    std::stringstream buffer;
    buffer << "TwoEquationTurbulenceModelAdjointElement #" << Element::Id();
    return buffer.str();
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "TwoEquationTurbulenceModelAdjointElement #" << Element::Id();
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::PrintData(std::ostream& rOStream) const
{
    Element::pGetGeometry()->PrintData(rOStream);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddFluidResidualsContributions(
    Vector& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = FluidData::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Primal = typename FluidData::Primal;

    typename Primal::Data                   element_data(*this, *mpConstitutiveLaw, rCurrentProcessInfo);
    typename Primal::ResidualsContributions residual_contributions(element_data);

    VectorF residual = ZeroVector(TFluidLocalSize);

    for (IndexType g = 0; g < Ws.size(); ++g) {
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];
        const double W = Ws[g];

        element_data.CalculateGaussPointData(W, N, dNdX);
        residual_contributions.AddGaussPointResidualsContributions(residual, W, N, dNdX);
    }

    AssembleSubVectorToVector(rOutput, 0, residual);

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddFluidFirstDerivatives(
    MatrixType& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = FluidData::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Derivatives = typename FluidData::StateDerivatives::FirstDerivatives;

    typename Derivatives::Data                     element_data(*this, *mpConstitutiveLaw, rCurrentProcessInfo);
    typename Derivatives::Velocity                 velocity_derivative(element_data);
    typename Derivatives::Pressure                 pressure_derivative(element_data);
    typename Derivatives::TurbulenceModelVariable1 turbulence_equation_1_derivative(element_data);
    typename Derivatives::TurbulenceModelVariable2 turbulence_equation_2_derivative(element_data);

    VectorF residual;
    MatrixND dNdXDerivative = ZeroMatrix(TNumNodes, TDim);

    for (IndexType g = 0; g < Ws.size(); ++g) {
        const double W = Ws[g];
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];

        element_data.CalculateGaussPointData(W, N, dNdX);

        IndexType row = 0;
        for (IndexType c = 0; c < TNumNodes; ++c) {
            for (IndexType k = 0; k < TDim; ++k) {
                velocity_derivative.CalculateGaussPointResidualsDerivativeContributions(residual, c, k, W, N, dNdX, 0.0, 0.0, dNdXDerivative);
                AssembleSubVectorToMatrix(rOutput, row++, 0, residual);
            }

            pressure_derivative.CalculateGaussPointResidualsDerivativeContributions(residual, c, 0, W, N, dNdX, 0.0, 0.0, dNdXDerivative);
            AssembleSubVectorToMatrix(rOutput, row++, 0, residual);

            turbulence_equation_1_derivative.CalculateGaussPointResidualsDerivativeContributions(residual, c, 0, W, N, dNdX, 0.0, 0.0, dNdXDerivative);
            AssembleSubVectorToMatrix(rOutput, row++, 0, residual);

            turbulence_equation_2_derivative.CalculateGaussPointResidualsDerivativeContributions(residual, c, 0, W, N, dNdX, 0.0, 0.0, dNdXDerivative);
            AssembleSubVectorToMatrix(rOutput, row++, 0, residual);
        }
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddFluidSecondDerivatives(
    MatrixType& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = FluidData::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Derivatives = typename FluidData::StateDerivatives::SecondDerivatives;

    typename Derivatives::Data         element_data(*this, *mpConstitutiveLaw, rCurrentProcessInfo);
    typename Derivatives::Acceleration acceleration_derivative(element_data);

    VectorF residual;

    for (IndexType g = 0; g < Ws.size(); ++g) {
        const double W = Ws[g];
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];

        element_data.CalculateGaussPointData(W, N, dNdX);

        IndexType row = 0;
        for (IndexType c = 0; c < TNumNodes; ++c) {
            for (IndexType k = 0; k < TDim; ++k) {
                acceleration_derivative.CalculateGaussPointResidualsDerivativeContributions(residual, c, k, W, N, dNdX);
                AssembleSubVectorToMatrix(rOutput, row++, 0, residual);
            }

            // skip derivatives w.r.t. pressure time derivative, turbulence variable 1 rate, turbulence variable 2 rate
            row += 3;
        }
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddFluidShapeDerivatives(
    Matrix& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = FluidData::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Derivatives = typename FluidData::SensitivityDerivatives;

    typename Derivatives::Data  element_data(*this, *mpConstitutiveLaw, rCurrentProcessInfo);
    typename Derivatives::Shape derivative(element_data);

    VectorF residual;

    for (IndexType g = 0; g < Ws.size(); ++g) {
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];
        const double W = Ws[g];

        element_data.CalculateGaussPointData(W, N, dNdX);

        Geometry<Point>::JacobiansType J;
        this->GetGeometry().Jacobian(J, integration_method);
        const auto& DN_De = this->GetGeometry().ShapeFunctionsLocalGradients(integration_method);

        GeometricalSensitivityUtility::ShapeFunctionsGradientType dNdX_derivative;
        const Matrix& rJ = J[g];
        const Matrix& rDN_De = DN_De[g];
        const double inv_detJ = 1.0 / MathUtils<double>::DetMat(rJ);
        GeometricalSensitivityUtility geom_sensitivity(rJ, rDN_De);

        ShapeParameter deriv;
        IndexType row = 0;
        for (deriv.NodeIndex = 0; deriv.NodeIndex < TNumNodes; ++deriv.NodeIndex) {
            for (deriv.Direction = 0; deriv.Direction < TDim; ++deriv.Direction) {
                double detJ_derivative;
                geom_sensitivity.CalculateSensitivity(deriv, detJ_derivative, dNdX_derivative);
                const double W_derivative = detJ_derivative * inv_detJ * W;

                derivative.CalculateGaussPointResidualsDerivativeContributions(residual, deriv.NodeIndex, deriv.Direction, W, N, dNdX, W_derivative, detJ_derivative, dNdX_derivative);
                AssembleSubVectorToMatrix(rOutput, row++, 0, residual);
            }
        }
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddTurbulenceResidualsContributions(
    Vector& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    const auto& integration_method = TurbulenceModelEquation1Data::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Equation1Primal= typename TurbulenceModelEquation1Data::Primal;
    using Equation2Primal= typename TurbulenceModelEquation2Data::Primal;

    // create data holders for turbulence equations
    typename Equation1Primal::Data eq_1_data(*this, rCurrentProcessInfo);
    typename Equation2Primal::Data eq_2_data(*this, rCurrentProcessInfo);

    // create equation residual data holders
    typename Equation1Primal::ResidualsContributions eq_1_residuals(eq_1_data);
    typename Equation2Primal::ResidualsContributions eq_2_residuals(eq_2_data);

    VectorN residuals_1 = ZeroVector(TNumNodes);
    VectorN residuals_2 = ZeroVector(TNumNodes);

    for (IndexType g = 0; g < Ws.size(); ++g) {
        const double W = Ws[g];
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];

        eq_1_data.CalculateGaussPointData(W, N, dNdX);
        eq_2_data.CalculateGaussPointData(W, N, dNdX);

        eq_1_residuals.AddGaussPointResidualsContributions(residuals_1, W, N, dNdX);
        eq_2_residuals.AddGaussPointResidualsContributions(residuals_2, W, N, dNdX);
    }

    eq_1_data.CalculateDataAfterGaussPointPointLoop();
    eq_2_data.CalculateDataAfterGaussPointPointLoop();

    eq_1_residuals.AddResidualsContributionsAfterGaussPointLoop(residuals_1);
    eq_2_residuals.AddResidualsContributionsAfterGaussPointLoop(residuals_2);

    AssembleSubVectorToVector(rOutput, TDim + 1, residuals_1);
    AssembleSubVectorToVector(rOutput, TDim + 2, residuals_2);
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddTurbulenceFirstDerivatives(
    MatrixType& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = TurbulenceModelEquation1Data::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Equation1Derivatives = typename TurbulenceModelEquation1Data::StateDerivatives::FirstDerivatives;
    using Equation2Derivatives = typename TurbulenceModelEquation2Data::StateDerivatives::FirstDerivatives;

    // create data holders for turbulence equations
    typename Equation1Derivatives::Data eq_1_data(*this, rCurrentProcessInfo);
    typename Equation2Derivatives::Data eq_2_data(*this, rCurrentProcessInfo);

    // create equation derivative data holders
    typename Equation1Derivatives::Velocity                 eq_1_derivative_0(eq_1_data);
    typename Equation1Derivatives::TurbulenceModelVariable1 eq_1_derivative_1(eq_1_data);
    typename Equation1Derivatives::TurbulenceModelVariable2 eq_1_derivative_2(eq_1_data);

    typename Equation2Derivatives::Velocity                 eq_2_derivative_0(eq_2_data);
    typename Equation2Derivatives::TurbulenceModelVariable1 eq_2_derivative_1(eq_2_data);
    typename Equation2Derivatives::TurbulenceModelVariable2 eq_2_derivative_2(eq_2_data);

    MatrixND dNdX_derivative = ZeroMatrix(TNumNodes, TDim);
    VectorN residual_derivatives;

    IndexType row_index = 0;
    for (IndexType g = 0; g < Ws.size(); ++g) {
        const double W = Ws[g];
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];

        eq_1_data.CalculateGaussPointData(W, N, dNdX);
        eq_2_data.CalculateGaussPointData(W, N, dNdX);

        row_index = 0;
        for (IndexType c = 0; c < TNumNodes; ++c) {

            // add derivatives w.r.t velocity
            for (IndexType k = 0; k < TDim; ++k) {
                eq_1_derivative_0.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, k, W, N, dNdX, 0, 0, dNdX_derivative);
                AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
                eq_2_derivative_0.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, k, W, N, dNdX, 0, 0, dNdX_derivative);
                AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
                ++row_index;
            }

            // skip derivatives w.r.t. pressure
            ++row_index;

            // add derivatives w.r.t. turbulence variable 1
            eq_1_derivative_1.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, 0, W, N, dNdX, 0, 0, dNdX_derivative);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
            eq_2_derivative_1.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, 0, W, N, dNdX, 0, 0, dNdX_derivative);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
            ++row_index;

            // add derivative w.r.t. turbulence variable 2
            eq_1_derivative_2.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, 0, W, N, dNdX, 0, 0, dNdX_derivative);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
            eq_2_derivative_2.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, 0, W, N, dNdX, 0, 0, dNdX_derivative);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
            ++row_index;
        }
    }

    eq_1_data.CalculateDataAfterGaussPointPointLoop();
    eq_2_data.CalculateDataAfterGaussPointPointLoop();

    // finalize derivative data holders
    row_index = 0;
    for (IndexType c = 0; c < TNumNodes; ++c) {
        for (IndexType k = 0; k < TDim; ++k) {
            eq_1_derivative_0.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, k);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
            eq_2_derivative_0.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, k);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
            ++row_index;
        }

        // skip derivative w.r.t. pressure
        ++row_index;

        // initializing derivative row w.r.t. turbulence variable 1
        eq_1_derivative_1.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, 0);
        AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
        eq_2_derivative_1.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, 0);
        AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
        ++row_index;

        // initializing derivative row w.r.t. turbulence variable 2
        eq_1_derivative_2.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, 0);
        AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
        eq_2_derivative_2.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, 0);
        AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
        ++row_index;
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddTurbulenceSecondDerivatives(
    MatrixType& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = TurbulenceModelEquation1Data::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Equation1Derivatives = typename TurbulenceModelEquation1Data::StateDerivatives::SecondDerivatives;
    using Equation2Derivatives = typename TurbulenceModelEquation2Data::StateDerivatives::SecondDerivatives;

    // create data holders for turbulence equations
    typename Equation1Derivatives::Data eq_1_data(*this, rCurrentProcessInfo);
    typename Equation2Derivatives::Data eq_2_data(*this, rCurrentProcessInfo);

    // create equation derivative data holders
    typename Equation1Derivatives::TurbulenceModelVariableRate1  eq_1_derivative(eq_1_data);
    typename Equation2Derivatives::TurbulenceModelVariableRate2  eq_2_derivative(eq_2_data);

    MatrixND dNdX_derivative = ZeroMatrix(TNumNodes, TDim);
    VectorN residual_derivatives;

    IndexType row_index = 0;
    for (IndexType g = 0; g < Ws.size(); ++g) {
        const double W = Ws[g];
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];

        eq_1_data.CalculateGaussPointData(W, N, dNdX);
        eq_2_data.CalculateGaussPointData(W, N, dNdX);

        row_index = 0;
        for (IndexType c = 0; c < TNumNodes; ++c) {
            // skip derivative terms w.r.t. velocity and pressure
            row_index += TDim + 1;

            // add derivatives w.r.t. turbulence variable 1
            eq_1_derivative.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, W, N, dNdX);
            AssembleSubVectorToMatrix(rOutput, row_index++, TDim + 1, residual_derivatives);

            // add derivative w.r.t. turbulence variable 2
            eq_2_derivative.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, c, W, N, dNdX);
            AssembleSubVectorToMatrix(rOutput, row_index++, TDim + 2, residual_derivatives);
        }
    }

    eq_1_data.CalculateDataAfterGaussPointPointLoop();
    eq_2_data.CalculateDataAfterGaussPointPointLoop();

    // finalize derivative data holders
    row_index = 0;
    for (IndexType c = 0; c < TNumNodes; ++c) {
        // skip derivative terms w.r.t. velocity and pressure
        row_index += TDim + 1;

        // initializing derivative row w.r.t. turbulence variable 1
        eq_1_derivative.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c);
        AssembleSubVectorToMatrix(rOutput, row_index++, TDim + 1, residual_derivatives);

        // initializing derivative row w.r.t. turbulence variable 2
        eq_2_derivative.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c);
        AssembleSubVectorToMatrix(rOutput, row_index++, TDim + 2, residual_derivatives);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::AddTurbulenceShapeDerivatives(
    Matrix& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    const auto& integration_method = TurbulenceModelEquation1Data::TResidualsDerivatives::GetIntegrationMethod();

    Vector Ws;
    Matrix Ns;
    ShapeFunctionDerivativesArrayType dNdXs;
    this->CalculateGeometryData(Ws, Ns, dNdXs, integration_method);

    using Equation1Derivatives = typename TurbulenceModelEquation1Data::SensitivityDerivatives;
    using Equation2Derivatives = typename TurbulenceModelEquation2Data::SensitivityDerivatives;

    // create data holders for turbulence equations
    typename Equation1Derivatives::Data eq_1_data(*this, rCurrentProcessInfo);
    typename Equation2Derivatives::Data eq_2_data(*this, rCurrentProcessInfo);

    // create equation derivative data holders
    typename Equation1Derivatives::Shape eq_1_derivative(eq_1_data);
    typename Equation2Derivatives::Shape eq_2_derivative(eq_2_data);

    MatrixND dNdX_derivative = ZeroMatrix(TNumNodes, TDim);
    VectorN residual_derivatives;

    IndexType row_index = 0;
    for (IndexType g = 0; g < Ws.size(); ++g) {
        const double W = Ws[g];
        const Vector& N = row(Ns, g);
        const Matrix& dNdX = dNdXs[g];

        eq_1_data.CalculateGaussPointData(W, N, dNdX);
        eq_2_data.CalculateGaussPointData(W, N, dNdX);

        Geometry<Point>::JacobiansType J;
        this->GetGeometry().Jacobian(J, integration_method);
        const auto& DN_De = this->GetGeometry().ShapeFunctionsLocalGradients(integration_method);

        GeometricalSensitivityUtility::ShapeFunctionsGradientType dNdX_deriv;
        const Matrix& rJ = J[g];
        const Matrix& rDN_De = DN_De[g];
        const double inv_detJ = 1.0 / MathUtils<double>::DetMat(rJ);
        GeometricalSensitivityUtility geom_sensitivity(rJ, rDN_De);

        row_index = 0;
        ShapeParameter deriv;
        for (deriv.NodeIndex = 0; deriv.NodeIndex < TNumNodes; ++deriv.NodeIndex) {
            for (deriv.Direction = 0; deriv.Direction < TDim; ++deriv.Direction) {
                double detJ_deriv;
                geom_sensitivity.CalculateSensitivity(deriv, detJ_deriv, dNdX_deriv);
                const double weight_deriv = detJ_deriv * inv_detJ * W;

                eq_1_derivative.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, deriv.NodeIndex, deriv.Direction, W, N, dNdX, weight_deriv, detJ_deriv, dNdX_deriv);
                AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
                eq_2_derivative.CalculateGaussPointResidualsDerivativeContributions(residual_derivatives, deriv.NodeIndex, deriv.Direction, W, N, dNdX, weight_deriv, detJ_deriv, dNdX_deriv);
                AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
                ++row_index;
            }
        }
    }

    eq_1_data.CalculateDataAfterGaussPointPointLoop();
    eq_2_data.CalculateDataAfterGaussPointPointLoop();

    // finalize derivative data holders
    row_index = 0;
    for (IndexType c = 0; c < TNumNodes; ++c) {
        for (IndexType k = 0; k < TDim; ++k) {
            eq_1_derivative.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, k);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 1, residual_derivatives);
            eq_2_derivative.CalculateResidualsDerivativeContributionsAfterGaussPointPointLoop(residual_derivatives, c, k);
            AssembleSubVectorToMatrix(rOutput, row_index, TDim + 2, residual_derivatives);
            ++row_index;
        }
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes, class TAdjointElementData>
void TwoEquationTurbulenceModelAdjointElement<TDim, TNumNodes, TAdjointElementData>::CalculateGeometryData(
    Vector& rGaussWeights,
    Matrix& rNContainer,
    ShapeFunctionDerivativesArrayType& rDN_DX,
    const GeometryData::IntegrationMethod& rIntegrationMethod) const
{
    const auto& r_geometry = this->GetGeometry();
    const IndexType number_of_gauss_points =
        r_geometry.IntegrationPointsNumber(rIntegrationMethod);

    Vector DetJ;
    r_geometry.ShapeFunctionsIntegrationPointsGradients(rDN_DX, DetJ, rIntegrationMethod);

    if (rNContainer.size1() != number_of_gauss_points || rNContainer.size2() != TNumNodes) {
        rNContainer.resize(number_of_gauss_points, TNumNodes, false);
    }
    rNContainer = r_geometry.ShapeFunctionsValues(rIntegrationMethod);

    const auto& IntegrationPoints = r_geometry.IntegrationPoints(rIntegrationMethod);

    if (rGaussWeights.size() != number_of_gauss_points) {
        rGaussWeights.resize(number_of_gauss_points, false);
    }

    for (IndexType g = 0; g < number_of_gauss_points; ++g) {
        rGaussWeights[g] = DetJ[g] * IntegrationPoints[g].Weight();
    }
}

// template instantiations
template class TwoEquationTurbulenceModelAdjointElement<2, 3, KEpsilonElementData::QSVMSRFCAdjointElementData<2, 3>>;
template class TwoEquationTurbulenceModelAdjointElement<3, 4, KEpsilonElementData::QSVMSRFCAdjointElementData<3, 4>>;

} // namespace Kratos