//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Inigo Lopez and Riccardo Rossi
//

#include "incompressible_perturbation_potential_flow_element.h"
#include "compressible_potential_flow_application_variables.h"
#include "includes/cfd_variables.h"
#include "fluid_dynamics_application_variables.h"
#include "custom_utilities/potential_flow_utilities.h"

namespace Kratos
{
///////////////////////////////////////////////////////////////////////////////////////////////////
// Public Operations

template <int Dim, int NumNodes>
Element::Pointer IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::Create(
    IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<IncompressiblePerturbationPotentialFlowElement>(
        NewId, GetGeometry().Create(ThisNodes), pProperties);
    KRATOS_CATCH("");
}

template <int Dim, int NumNodes>
Element::Pointer IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::Create(
    IndexType NewId, GeometryType::Pointer pGeom, PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<IncompressiblePerturbationPotentialFlowElement>(
        NewId, pGeom, pProperties);
    KRATOS_CATCH("");
}

template <int Dim, int NumNodes>
Element::Pointer IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::Clone(
    IndexType NewId, NodesArrayType const& ThisNodes) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<IncompressiblePerturbationPotentialFlowElement>(
        NewId, GetGeometry().Create(ThisNodes), pGetProperties());
    KRATOS_CATCH("");
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateLocalSystem(
    MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, const ProcessInfo& rCurrentProcessInfo)
{
    CalculateRightHandSide(rRightHandSideVector,rCurrentProcessInfo);
    CalculateLeftHandSide(rLeftHandSideMatrix,rCurrentProcessInfo);
    const auto penalty_coefficient = rCurrentProcessInfo[PENALTY_COEFFICIENT];
    if (penalty_coefficient > std::numeric_limits<double>::epsilon()) {
        AddKuttaConditionPenaltyTerm(rLeftHandSideMatrix,rRightHandSideVector,rCurrentProcessInfo);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateRightHandSide(
    VectorType& rRightHandSideVector, const ProcessInfo& rCurrentProcessInfo)
{
    const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
    const int wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element (non-wake) - eventually an embedded
        CalculateRightHandSideNormalElement(rRightHandSideVector, rCurrentProcessInfo);
    else // Wake element
        CalculateRightHandSideWakeElement(rRightHandSideVector, rCurrentProcessInfo);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateLeftHandSide(
    MatrixType& rLeftHandSideMatrix, const ProcessInfo& rCurrentProcessInfo)
{
    const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
    const int wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element (non-wake) - eventually an embedded
        CalculateLeftHandSideNormalElement(rLeftHandSideMatrix, rCurrentProcessInfo);
    else // Wake element
        CalculateLeftHandSideWakeElement(rLeftHandSideMatrix, rCurrentProcessInfo);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::AddKuttaConditionPenaltyTerm(
    MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, const ProcessInfo& rCurrentProcessInfo)
{
    const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
    const int wake = r_this.GetValue(WAKE);
    const int kutta = r_this.GetValue(KUTTA);


    PotentialFlowUtilities::ElementalData<NumNodes,Dim> data;
    const double free_stream_density = rCurrentProcessInfo[FREE_STREAM_DENSITY];
    const auto penalty = rCurrentProcessInfo[PENALTY_COEFFICIENT];

    GeometryUtils::CalculateGeometryData(this->GetGeometry(), data.DN_DX, data.N, data.vol);
    data.potentials = PotentialFlowUtilities::GetPotentialOnNormalElement<Dim,NumNodes>(*this);

    Vector vector_distances=ZeroVector(NumNodes);

    double angle_in_deg = rCurrentProcessInfo[ROTATION_ANGLE];

    BoundedVector<double, Dim> n_angle;
    n_angle[0]=sin(angle_in_deg*Globals::Pi/180);
    n_angle[1]=0;
    n_angle[2]=cos(angle_in_deg*Globals::Pi/180);

    BoundedMatrix<double, NumNodes, NumNodes> lhs_kutta = ZeroMatrix(NumNodes, NumNodes);
    BoundedVector<double, NumNodes> test=prod(data.DN_DX, n_angle);


    const array_1d<double, 3> free_stream_velocity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
    array_1d<double, Dim> velocity = PotentialFlowUtilities::ComputeVelocity<Dim,NumNodes>(*this);
    for (unsigned int i = 0; i < Dim; i++){
        velocity[i] += free_stream_velocity[i];
    }

    noalias(lhs_kutta) = penalty*data.vol*free_stream_density * outer_prod(test, test);
    BoundedMatrix<double, NumNodes, NumNodes>  n_matrix = outer_prod(n_angle, n_angle);
    BoundedVector<double, NumNodes> velvector = prod(n_matrix,  velocity);
    BoundedVector<double, NumNodes> rhs_penalty = prod(data.DN_DX,  velvector);
    for (unsigned int i = 0; i < NumNodes; ++i)
    {
        if (this->GetGeometry()[i].GetValue(TRAILING_EDGE) && kutta==0)
        {
            if (wake==0)  {
                for (unsigned int j = 0; j < NumNodes; ++j)
                {
                    rLeftHandSideMatrix(i, j) += lhs_kutta(i, j);
                    rRightHandSideVector(i) += -penalty*data.vol*free_stream_density *rhs_penalty(i);
                    // rRightHandSideVector(i) += -lhs_kutta(i, j)*data.potentials(j);
                }
            } else {
                // data.distances = this->GetValue(WAKE_ELEMENTAL_DISTANCES);
                // BoundedVector<double, 2*NumNodes> split_element_values;
                // split_element_values = PotentialFlowUtilities::GetPotentialOnWakeElement<Dim, NumNodes>(*this, data.distances);
                // for (unsigned int j = 0; j < NumNodes; ++j)
                // {
                //     rLeftHandSideMatrix(i, j) += lhs_kutta(i, j);
                //     rLeftHandSideMatrix(i+NumNodes, j+NumNodes) += lhs_kutta(i, j);
                //     rRightHandSideVector(i) += -lhs_kutta(i, j)*split_element_values(j);
                //     rRightHandSideVector(i+NumNodes) += -lhs_kutta(i, j)*split_element_values(j+NumNodes);
                // }
            }
        }
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::EquationIdVector(
    EquationIdVectorType& rResult, const ProcessInfo& CurrentProcessInfo) const
{
    const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
    const int wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element
    {
        if (rResult.size() != NumNodes)
            rResult.resize(NumNodes, false);

        const int kutta = r_this.GetValue(KUTTA);

        if (kutta == 0)
            GetEquationIdVectorNormalElement(rResult);
        else
            GetEquationIdVectorKuttaElement(rResult);
    }
    else // Wake element
    {
        // if(this->GetValue(WING_TIP_ELEMENT)){
        //     if (rResult.size() != 2 * NumNodes + 1)
        //         rResult.resize(2 * NumNodes + 1, false);
        // }
        // else{
        //     if (rResult.size() != 2 * NumNodes)
        //         rResult.resize(2 * NumNodes, false);
        // }
        if (rResult.size() != 3 * NumNodes)
            rResult.resize(3 * NumNodes, false);

        GetEquationIdVectorWakeElement(rResult);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetDofList(DofsVectorType& rElementalDofList,
                                                                   const ProcessInfo& CurrentProcessInfo) const
{
    const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
    const int wake = r_this.GetValue(WAKE);

    if (wake == 0) // Normal element
    {
        if (rElementalDofList.size() != NumNodes)
            rElementalDofList.resize(NumNodes);

        const int kutta = r_this.GetValue(KUTTA);

        if (kutta == 0)
            GetDofListNormalElement(rElementalDofList);
        else
            GetDofListKuttaElement(rElementalDofList);
    }
    else // wake element
    {
        // if(this->GetValue(WING_TIP_ELEMENT)){
        //     if (rElementalDofList.size() != 2 * NumNodes + 1)
        //         rElementalDofList.resize(2 * NumNodes + 1);
        // }
        // else{
        //     if (rElementalDofList.size() != 2 * NumNodes)
        //         rElementalDofList.resize(2 * NumNodes);
        // }
        if (rElementalDofList.size() != 3 * NumNodes)
            rElementalDofList.resize(3 * NumNodes);

        GetDofListWakeElement(rElementalDofList);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::FinalizeSolutionStep(const ProcessInfo& rCurrentProcessInfo)
{
    bool active = true;
    if ((this)->IsDefined(ACTIVE))
        active = (this)->Is(ACTIVE);

    const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
    const int wake = r_this.GetValue(WAKE);

    if (wake != 0 && active == true)
    {
        ComputePotentialJump(rCurrentProcessInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Inquiry

template <int Dim, int NumNodes>
int IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    // Generic geometry check
    int out = Element::Check(rCurrentProcessInfo);
    if (out != 0)
    {
        return out;
    }

    KRATOS_ERROR_IF(GetGeometry().Area() <= 0.0)
        << this->Id() << "Area cannot be less than or equal to 0" << std::endl;

    for (unsigned int i = 0; i < this->GetGeometry().size(); i++)
    {
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY_POTENTIAL,this->GetGeometry()[i]);
    }

    return out;

    KRATOS_CATCH("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateOnIntegrationPoints(
    const Variable<double>& rVariable, std::vector<double>& rValues, const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);

    if (rVariable == PRESSURE_COEFFICIENT)
    {
        rValues[0] = PotentialFlowUtilities::ComputePerturbationIncompressiblePressureCoefficient<Dim,NumNodes>(*this,rCurrentProcessInfo);
    }
    else if (rVariable == DENSITY)
    {
        rValues[0] = rCurrentProcessInfo[FREE_STREAM_DENSITY];
    }
    else if (rVariable == MACH)
    {
        const array_1d<double, 3> free_stream_velocity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
        array_1d<double, Dim> velocity = PotentialFlowUtilities::ComputeVelocity<Dim,NumNodes>(*this);
        for (unsigned int i = 0; i < Dim; i++){
            velocity[i] += free_stream_velocity[i];
        }
        const double velocity_module = sqrt(inner_prod(velocity, velocity));
        rValues[0] = velocity_module / rCurrentProcessInfo[SOUND_VELOCITY];
    }
    else if (rVariable == SOUND_VELOCITY)
    {
        rValues[0] = rCurrentProcessInfo[SOUND_VELOCITY];
    }
    else if (rVariable == WAKE)
    {
        const IncompressiblePerturbationPotentialFlowElement& r_this = *this;
        rValues[0] = r_this.GetValue(WAKE);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateOnIntegrationPoints(
    const Variable<int>& rVariable, std::vector<int>& rValues, const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);
    if (rVariable == TRAILING_EDGE)
        rValues[0] = this->GetValue(TRAILING_EDGE);
    else if (rVariable == WAKE)
        rValues[0] = this->GetValue(WAKE);
    else if (rVariable == KUTTA)
        rValues[0] = this->GetValue(KUTTA);
    else if (rVariable == NORMAL_ELEMENT)
        rValues[0] = this->GetValue(NORMAL_ELEMENT);
    else if (rVariable == WING_TIP_ELEMENT)
        rValues[0] = this->GetValue(WING_TIP_ELEMENT);
    else if (rVariable == TRAILING_EDGE_ELEMENT)
        rValues[0] = this->GetValue(TRAILING_EDGE_ELEMENT);
    else if (rVariable == DECOUPLED_TRAILING_EDGE_ELEMENT)
        rValues[0] = this->GetValue(DECOUPLED_TRAILING_EDGE_ELEMENT);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateOnIntegrationPoints(
    const Variable<array_1d<double, 3>>& rVariable,
    std::vector<array_1d<double, 3>>& rValues,
    const ProcessInfo& rCurrentProcessInfo)
{
    if (rValues.size() != 1)
        rValues.resize(1);
    if (rVariable == VELOCITY){
        const array_1d<double, 3> v_inf = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
        array_1d<double, 3> v(3, 0.0);
        array_1d<double, Dim> vaux = PotentialFlowUtilities::ComputeVelocity<Dim,NumNodes>(*this);
        for (unsigned int k = 0; k < Dim; k++)
            v[k] = vaux[k] + v_inf[k];
        rValues[0] = v;
    }
    else if (rVariable == PERTURBATION_VELOCITY){
        array_1d<double, 3> v(3, 0.0);
        array_1d<double, Dim> vaux = PotentialFlowUtilities::ComputeVelocity<Dim,NumNodes>(*this);
        for (unsigned int k = 0; k < Dim; k++)
            v[k] = vaux[k];
        rValues[0] = v;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input and output

template <int Dim, int NumNodes>
std::string IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::Info() const
{
    std::stringstream buffer;
    buffer << "IncompressiblePerturbationPotentialFlowElement #" << Id();
    return buffer.str();
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "IncompressiblePerturbationPotentialFlowElement #" << Id();
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::PrintData(std::ostream& rOStream) const
{
    pGetGeometry()->PrintData(rOStream);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////////////////////////

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetWakeDistances(array_1d<double, NumNodes>& distances) const
{
    noalias(distances) = GetValue(WAKE_ELEMENTAL_DISTANCES);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetEquationIdVectorNormalElement(EquationIdVectorType& rResult) const
{
    for (unsigned int i = 0; i < NumNodes; i++)
        rResult[i] = GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetEquationIdVectorKuttaElement(EquationIdVectorType& rResult) const
{
    const auto& r_geometry = this->GetGeometry();
    // Kutta elements have only negative part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (!r_geometry[i].GetValue(TRAILING_EDGE))
            rResult[i] = r_geometry[i].GetDof(VELOCITY_POTENTIAL).EquationId();
        else
            rResult[i] = r_geometry[i].GetDof(AUXILIARY_VELOCITY_POTENTIAL).EquationId();
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetEquationIdVectorWakeElement(EquationIdVectorType& rResult) const
{
    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    // Positive part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] > 0.0)
            rResult[i] = GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
        else
            rResult[i] =
                GetGeometry()[i].GetDof(AUXILIARY_VELOCITY_POTENTIAL, 0).EquationId();
    }

    // Negative part - sign is opposite to the previous case
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] < 0.0)
            rResult[NumNodes + i] =
                GetGeometry()[i].GetDof(VELOCITY_POTENTIAL).EquationId();
        else
            rResult[NumNodes + i] =
                GetGeometry()[i].GetDof(AUXILIARY_VELOCITY_POTENTIAL).EquationId();
    }

    for (unsigned int i = 0; i < NumNodes; i++){
        rResult[2*NumNodes + i] = GetGeometry()[i].GetDof(LAGRANGE_MULTIPLIER_0).EquationId();
    }

    // if(this->GetValue(WING_TIP_ELEMENT)){
    //     for (unsigned int i = 0; i < NumNodes; i++){
    //         if(GetGeometry()[i].GetValue(WING_TIP)){
    //             rResult[2*NumNodes] = GetGeometry()[i].GetDof(LAGRANGE_MULTIPLIER_0).EquationId();
    //         }
    //     }
    // }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetDofListNormalElement(DofsVectorType& rElementalDofList) const
{
    for (unsigned int i = 0; i < NumNodes; i++)
        rElementalDofList[i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetDofListKuttaElement(DofsVectorType& rElementalDofList) const
{
    const auto& r_geometry = this->GetGeometry();
    // Kutta elements have only negative part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (!GetGeometry()[i].GetValue(TRAILING_EDGE))
            rElementalDofList[i] = r_geometry[i].pGetDof(VELOCITY_POTENTIAL);
        else
            rElementalDofList[i] = r_geometry[i].pGetDof(AUXILIARY_VELOCITY_POTENTIAL);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::GetDofListWakeElement(DofsVectorType& rElementalDofList) const
{
    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    // Positive part
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] > 0)
            rElementalDofList[i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
        else
            rElementalDofList[i] = GetGeometry()[i].pGetDof(AUXILIARY_VELOCITY_POTENTIAL);
    }

    // Negative part - sign is opposite to the previous case
    for (unsigned int i = 0; i < NumNodes; i++)
    {
        if (distances[i] < 0)
            rElementalDofList[NumNodes + i] = GetGeometry()[i].pGetDof(VELOCITY_POTENTIAL);
        else
            rElementalDofList[NumNodes + i] =
                GetGeometry()[i].pGetDof(AUXILIARY_VELOCITY_POTENTIAL);
    }

    for (unsigned int i = 0; i < NumNodes; i++){
        rElementalDofList[2*NumNodes + i] = GetGeometry()[i].pGetDof(LAGRANGE_MULTIPLIER_0);
    }

    // if(this->GetValue(WING_TIP_ELEMENT)){
    //     for (unsigned int i = 0; i < NumNodes; i++){
    //         if(GetGeometry()[i].GetValue(WING_TIP)){
    //             rElementalDofList[2*NumNodes] = GetGeometry()[i].pGetDof(LAGRANGE_MULTIPLIER_0);
    //         }
    //     }
    // }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateLeftHandSideNormalElement(
    MatrixType& rLeftHandSideMatrix, const ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != NumNodes || rLeftHandSideMatrix.size2() != NumNodes)
        rLeftHandSideMatrix.resize(NumNodes, NumNodes, false);
    rLeftHandSideMatrix.clear();

    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    const double free_stream_density = rCurrentProcessInfo[FREE_STREAM_DENSITY];

    noalias(rLeftHandSideMatrix) =
        data.vol * free_stream_density * prod(data.DN_DX, trans(data.DN_DX));
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateRightHandSideNormalElement(
    VectorType& rRightHandSideVector, const ProcessInfo& rCurrentProcessInfo)
{
    if (rRightHandSideVector.size() != NumNodes)
        rRightHandSideVector.resize(NumNodes, false);
    rRightHandSideVector.clear();

    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    const double free_stream_density = rCurrentProcessInfo[FREE_STREAM_DENSITY];

    const array_1d<double, 3> free_stream_velocity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
    array_1d<double, Dim> velocity = PotentialFlowUtilities::ComputeVelocity<Dim,NumNodes>(*this);
    for (unsigned int i = 0; i < Dim; i++){
        velocity[i] += free_stream_velocity[i];
    }
    noalias(rRightHandSideVector) = - data.vol * free_stream_density * prod(data.DN_DX, velocity);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateLeftHandSideWakeElement(
    MatrixType& rLeftHandSideMatrix, const ProcessInfo& rCurrentProcessInfo)
{
    // Note that the lhs and rhs have double the size
    // if(this->GetValue(WING_TIP_ELEMENT)){
    //     if (rLeftHandSideMatrix.size1() != 2 * NumNodes + 1 ||
    //         rLeftHandSideMatrix.size2() != 2 * NumNodes + 1)
    //         rLeftHandSideMatrix.resize(2 * NumNodes + 1, 2 * NumNodes + 1, false);
    // }
    // else{
    //     if (rLeftHandSideMatrix.size1() != 2 * NumNodes ||
    //         rLeftHandSideMatrix.size2() != 2 * NumNodes)
    //         rLeftHandSideMatrix.resize(2 * NumNodes, 2 * NumNodes, false);
    // }
    if (rLeftHandSideMatrix.size1() != 2 * NumNodes ||
        rLeftHandSideMatrix.size2() != 2 * NumNodes)
        rLeftHandSideMatrix.resize(2 * NumNodes, 2 * NumNodes, false);
    rLeftHandSideMatrix.clear();

    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    const double free_stream_density = rCurrentProcessInfo[FREE_STREAM_DENSITY];

    GetWakeDistances(data.distances);

    BoundedMatrix<double, NumNodes, NumNodes> lhs_total = ZeroMatrix(NumNodes, NumNodes);

    ComputeLHSGaussPointContribution(data.vol*free_stream_density, lhs_total, data);

    // Wake condition matrix
    BoundedMatrix<double, Dim, Dim> condition_matrix = IdentityMatrix(Dim,Dim);
    condition_matrix(0,0) = 1.0;
    condition_matrix(1,1) = 0.0;
    condition_matrix(2,2) = 0.0;

    auto xzfilter = prod(condition_matrix, trans(data.DN_DX));

    BoundedMatrix<double, NumNodes, NumNodes> lhs_wake_condition = ZeroMatrix(NumNodes, NumNodes);

    for (unsigned int i = 0; i < NumNodes; i++){
        for(unsigned int j = 0; j < NumNodes; j++){
            for(unsigned int k = 0; k < Dim; k++){
                lhs_wake_condition(i,j) += data.vol*free_stream_density*data.DN_DX(i,k)*xzfilter(k,j);
            }
        }
    }

    BoundedMatrix<double, Dim, Dim> lagrange_condition_matrix = IdentityMatrix(Dim,Dim);
    lagrange_condition_matrix(0,0) = 1.0;
    lagrange_condition_matrix(1,1) = 0.0;
    lagrange_condition_matrix(2,2) = 0.0;

    auto xzfilter_lagrange = prod(lagrange_condition_matrix, trans(data.DN_DX));

    BoundedMatrix<double, NumNodes, NumNodes> lhs_wake_condition_lagrange = ZeroMatrix(NumNodes, NumNodes);

    for (unsigned int i = 0; i < NumNodes; i++){
        for(unsigned int j = 0; j < NumNodes; j++){
            for(unsigned int k = 0; k < Dim; k++){
                lhs_wake_condition_lagrange(i,j) += data.vol*free_stream_density*data.DN_DX(i,k)*xzfilter_lagrange(k,j);
            }
        }
    }

    if (this->Is(STRUCTURE)){
        BoundedMatrix<double, NumNodes, NumNodes> lhs_positive = ZeroMatrix(NumNodes, NumNodes);
        BoundedMatrix<double, NumNodes, NumNodes> lhs_negative = ZeroMatrix(NumNodes, NumNodes);

        CalculateLeftHandSideSubdividedElement(lhs_positive, lhs_negative, rCurrentProcessInfo);
        const auto& r_geometry = this->GetGeometry();
        for (unsigned int row = 0; row < NumNodes; ++row){
            // The TE node takes the contribution of the subdivided element and
            // we do not apply the wake condition on the TE node
            if (r_geometry[row].GetValue(TRAILING_EDGE)){
                for (unsigned int column = 0; column < NumNodes; ++column){
                    // Conservation of mass
                    rLeftHandSideMatrix(row, column) = lhs_positive(row, column);
                    rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_negative(row, column);

                    // // Wake condition below
                    // rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_wake_condition(row, column); // Diagonal
                    // rLeftHandSideMatrix(row + NumNodes, column) = -lhs_wake_condition(row, column); // Off diagonal
                }
            }
            else{
                // Applying wake condition on the AUXILIARY_VELOCITY_POTENTIAL dofs
                if (data.distances[row] < 0.0){
                    for (unsigned int column = 0; column < NumNodes; ++column){
                        // Conservation of mass
                        rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_total(row, column);
                        // Wake condition
                        rLeftHandSideMatrix(row, column) = lhs_wake_condition(row, column); // Diagonal
                        rLeftHandSideMatrix(row, column + NumNodes) = -lhs_wake_condition(row, column); // Off diagonal
                    }
                }
                else{ // else if (data.distances[row] > 0.0)
                    for (unsigned int column = 0; column < NumNodes; ++column){
                        // Conservation of mass
                        rLeftHandSideMatrix(row, column) = lhs_total(row, column);
                        // Wake condition
                        rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_wake_condition(row, column); // Diagonal
                        rLeftHandSideMatrix(row + NumNodes, column) = -lhs_wake_condition(row, column); // Off diagonal
                    }
                }
            }
        }
    }
    else{
        for (unsigned int row = 0; row < NumNodes; ++row){
            // Applying wake condition on the AUXILIARY_VELOCITY_POTENTIAL dofs
            if (data.distances[row] < 0.0){
                for (unsigned int column = 0; column < NumNodes; ++column){
                    // Conservation of mass
                    rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_total(row, column);
                    // Wake condition
                    rLeftHandSideMatrix(row, column) = lhs_wake_condition(row, column); // Diagonal
                    rLeftHandSideMatrix(row, column + NumNodes) = -lhs_wake_condition(row, column); // Off diagonal
                }
            }
            else{ // else if (data.distances[row] > 0.0)
                for (unsigned int column = 0; column < NumNodes; ++column){
                    // Conservation of mass
                    rLeftHandSideMatrix(row, column) = lhs_total(row, column);
                    // Wake condition
                    rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_wake_condition(row, column); // Diagonal
                    rLeftHandSideMatrix(row + NumNodes, column) = -lhs_wake_condition(row, column); // Off diagonal
                }
            }
        }

        // // Apply wake condition with lagrange multipliers
        // for (unsigned int row = 0; row < NumNodes; ++row){
        //     if(!GetGeometry()[row].GetValue(WING_TIP)){
        //         for (unsigned int column = 0; column < NumNodes; ++column){
        //             rLeftHandSideMatrix(2*NumNodes + row, column) = -lhs_wake_condition_lagrange(row, row);
        //             rLeftHandSideMatrix(2*NumNodes + row, column + NumNodes) = lhs_wake_condition_lagrange(row, row);

        //             rLeftHandSideMatrix(column, 2*NumNodes + row) = -lhs_wake_condition_lagrange(row, column);
        //             rLeftHandSideMatrix(column + NumNodes, 2*NumNodes + row) = lhs_wake_condition_lagrange(row, column);
        //         }
        //     }
        // }
    }

    // if(this->GetValue(WING_TIP_ELEMENT)){
    //     // KRATOS_WATCH(this->Id())
    //     for (unsigned int row = 0; row < NumNodes; ++row){
    //         if(GetGeometry()[row].GetValue(WING_TIP)){
    //             rLeftHandSideMatrix(2*NumNodes + row, row) = -lhs_total(row, row);
    //             rLeftHandSideMatrix(2*NumNodes + row, row + NumNodes) = lhs_total(row, row);

    //             rLeftHandSideMatrix(row, 2*NumNodes + row) = -lhs_total(row, row);
    //             rLeftHandSideMatrix(row + NumNodes, 2*NumNodes + row) = lhs_total(row, row);
    //         }
    //     }
    // }

    // Print lhs
    std::cout.precision(5);
    std::cout << std::scientific;
    std::cout << std::showpos;
    // if(this->GetValue(WING_TIP_ELEMENT)){
    //     KRATOS_WATCH(this->Id())
    // }
    // if(this->Id()==12750 || this->Id()==26346 || this->Id()==265451 || this->Id()==87225){
    if(this->Id()==161405){//} || this->Id()==79624){
    //if(this->GetValue(WING_TIP_ELEMENT) || this->Id()==94253 || this->Id()==1){
        std::cout << std::endl;
        for(unsigned int row = 0; row < rLeftHandSideMatrix.size1(); ++row){
            for(unsigned int column = 0; column < rLeftHandSideMatrix.size2(); column++){
                if(column == 3 || column == 7){
                    std::cout << " " << rLeftHandSideMatrix(row, column) << " |";
                }
                else{
                    std::cout << " " << rLeftHandSideMatrix(row, column) << " ";
                }
            }

            std::cout << std::endl;

            if(row ==3|| row == 7){
                for(unsigned int j = 0; j < 14*rLeftHandSideMatrix.size1(); j++){
                std::cout << "_" ;
                }
                std::cout << " " << std::endl;
            }
            else{
                for(unsigned int i = 0; i < 3; i++){
                    for(unsigned int j = 0; j < 14*4; j++){
                        std::cout << " " ;
                    }
                    if(i != 2){
                        std::cout << "|" ;
                    }
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateRightHandSideWakeElement(
    VectorType& rRightHandSideVector, const ProcessInfo& rCurrentProcessInfo)
{
    // Note that the rhs has double the size
    // if(this->GetValue(WING_TIP_ELEMENT)){
    //     if (rRightHandSideVector.size() != 2 * NumNodes + 1)
    //         rRightHandSideVector.resize(2 * NumNodes + 1, false);
    // }
    // else{
    //     if (rRightHandSideVector.size() != 2 * NumNodes)
    //         rRightHandSideVector.resize(2 * NumNodes, false);
    // }
    if (rRightHandSideVector.size() != 3 * NumNodes)
            rRightHandSideVector.resize(3 * NumNodes, false);

    rRightHandSideVector.clear();

    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    const auto& r_geometry = this->GetGeometry();
    GeometryUtils::CalculateGeometryData(r_geometry, data.DN_DX, data.N, data.vol);

    const double free_stream_density = rCurrentProcessInfo[FREE_STREAM_DENSITY];

    GetWakeDistances(data.distances);

    const array_1d<double, 3> free_stream_velocity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
    array_1d<double, Dim> upper_velocity = PotentialFlowUtilities::ComputeVelocityUpperWakeElement<Dim,NumNodes>(*this);
    array_1d<double, Dim> lower_velocity = PotentialFlowUtilities::ComputeVelocityLowerWakeElement<Dim,NumNodes>(*this);
    for (unsigned int i = 0; i < Dim; i++){
        upper_velocity[i] += free_stream_velocity[i];
        lower_velocity[i] += free_stream_velocity[i];
    }
    const array_1d<double, Dim> diff_velocity = upper_velocity - lower_velocity;

    const BoundedVector<double, NumNodes> upper_rhs = - data.vol * free_stream_density * prod(data.DN_DX, upper_velocity);
    const BoundedVector<double, NumNodes> lower_rhs = - data.vol * free_stream_density * prod(data.DN_DX, lower_velocity);
    const BoundedVector<double, NumNodes> wake_rhs = - data.vol * free_stream_density * prod(data.DN_DX, diff_velocity);

    if (this->Is(STRUCTURE))
    {
        double upper_vol = 0.0;
        double lower_vol = 0.0;

        CalculateVolumesSubdividedElement(upper_vol, lower_vol, rCurrentProcessInfo);
        for (unsigned int i = 0; i < NumNodes; ++i){
            if (r_geometry[i].GetValue(TRAILING_EDGE)){
                rRightHandSideVector[i] = upper_rhs(i)*upper_vol/data.vol;
                rRightHandSideVector[i + NumNodes] = lower_rhs(i)*lower_vol/data.vol;
            }
            else{
                AssignRightHandSideWakeNode(rRightHandSideVector, upper_rhs, lower_rhs, wake_rhs, data, i);
            }
        }
    }
    else{
        for (unsigned int i = 0; i < NumNodes; ++i){
            AssignRightHandSideWakeNode(rRightHandSideVector, upper_rhs, lower_rhs, wake_rhs, data, i);
        }
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateLeftHandSideSubdividedElement(
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_positive,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_negative,
    const ProcessInfo& rCurrentProcessInfo)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    const double free_stream_density = rCurrentProcessInfo[FREE_STREAM_DENSITY];

    GetWakeDistances(data.distances);

    // Subdivide the element
    constexpr unsigned int nvolumes = 3 * (Dim - 1);
    BoundedMatrix<double, NumNodes, Dim> Points;
    array_1d<double, nvolumes> PartitionsSign;
    BoundedMatrix<double, nvolumes, NumNodes> GPShapeFunctionValues;
    array_1d<double, nvolumes> Volumes;
    std::vector<Matrix> GradientsValue(nvolumes);
    BoundedMatrix<double, nvolumes, 2> NEnriched;
    for (unsigned int i = 0; i < GradientsValue.size(); ++i)
        GradientsValue[i].resize(2, Dim, false);
    for (unsigned int i = 0; i < NumNodes; ++i)
    {
        const array_1d<double, 3>& coords = GetGeometry()[i].Coordinates();
        for (unsigned int k = 0; k < Dim; ++k)
        {
            Points(i, k) = coords[k];
        }
    }

    const unsigned int nsubdivisions = EnrichmentUtilities::CalculateEnrichedShapeFuncions(
        Points, data.DN_DX, data.distances, Volumes, GPShapeFunctionValues,
        PartitionsSign, GradientsValue, NEnriched);

    // Compute the lhs and rhs that would correspond to it being divided
    for (unsigned int i = 0; i < nsubdivisions; ++i)
    {
        if (PartitionsSign[i] > 0){
            ComputeLHSGaussPointContribution(Volumes[i]*free_stream_density, lhs_positive, data);
        }
        else{
            ComputeLHSGaussPointContribution(Volumes[i]*free_stream_density, lhs_negative, data);
        }
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::CalculateVolumesSubdividedElement(
    double& rUpper_vol,
    double& rLower_vol,
    const ProcessInfo& rCurrentProcessInfo)
{
    ElementalData<NumNodes, Dim> data;

    // Calculate shape functions
    GeometryUtils::CalculateGeometryData(GetGeometry(), data.DN_DX, data.N, data.vol);

    GetWakeDistances(data.distances);

    // Subdivide the element
    constexpr unsigned int nvolumes = 3 * (Dim - 1);
    BoundedMatrix<double, NumNodes, Dim> Points;
    array_1d<double, nvolumes> PartitionsSign;
    BoundedMatrix<double, nvolumes, NumNodes> GPShapeFunctionValues;
    array_1d<double, nvolumes> Volumes;
    std::vector<Matrix> GradientsValue(nvolumes);
    BoundedMatrix<double, nvolumes, 2> NEnriched;
    for (unsigned int i = 0; i < GradientsValue.size(); ++i)
        GradientsValue[i].resize(2, Dim, false);
    for (unsigned int i = 0; i < NumNodes; ++i)
    {
        const array_1d<double, 3>& coords = GetGeometry()[i].Coordinates();
        for (unsigned int k = 0; k < Dim; ++k)
        {
            Points(i, k) = coords[k];
        }
    }

    const unsigned int nsubdivisions = EnrichmentUtilities::CalculateEnrichedShapeFuncions(
        Points, data.DN_DX, data.distances, Volumes, GPShapeFunctionValues,
        PartitionsSign, GradientsValue, NEnriched);

    // Compute the volumes that would correspond to it being divided
    for (unsigned int i = 0; i < nsubdivisions; ++i)
    {
        if (PartitionsSign[i] > 0){
            rUpper_vol += Volumes[i];
        }
        else{
            rLower_vol += Volumes[i];
        }
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::ComputeLHSGaussPointContribution(
    const double weight,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs,
    const ElementalData<NumNodes, Dim>& data) const
{
    noalias(lhs) += weight * prod(data.DN_DX, trans(data.DN_DX));
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::AssignLeftHandSideSubdividedElement(
    MatrixType& rLeftHandSideMatrix,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_positive,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_negative,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_total,
    const ElementalData<NumNodes, Dim>& data) const
{
    const auto& r_geometry = this->GetGeometry();
    for (unsigned int i = 0; i < NumNodes; ++i)
    {
        // The TE node takes the contribution of the subdivided element and
        // we do not apply the wake condition on the TE node
        if (r_geometry[i].GetValue(TRAILING_EDGE))
        {
            for (unsigned int j = 0; j < NumNodes; ++j)
            {
                rLeftHandSideMatrix(i, j) = lhs_positive(i, j);
                rLeftHandSideMatrix(i + NumNodes, j + NumNodes) = lhs_negative(i, j);
            }
        }
        else
            AssignLeftHandSideWakeNode(rLeftHandSideMatrix, lhs_total, data, i);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::AssignLeftHandSideWakeElement(
    MatrixType& rLeftHandSideMatrix,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_total,
    const ElementalData<NumNodes, Dim>& data) const
{
    for (unsigned int row = 0; row < NumNodes; ++row)
        AssignLeftHandSideWakeNode(rLeftHandSideMatrix, lhs_total, data, row);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::AssignLeftHandSideWakeNode(
    MatrixType& rLeftHandSideMatrix,
    BoundedMatrix<double, NumNodes, NumNodes>& lhs_total,
    const ElementalData<NumNodes, Dim>& data,
    unsigned int& row) const
{
    // Filling the diagonal blocks (i.e. decoupling upper and lower dofs)
    for (unsigned int column = 0; column < NumNodes; ++column)
    {
        rLeftHandSideMatrix(row, column) = lhs_total(row, column);
        rLeftHandSideMatrix(row + NumNodes, column + NumNodes) = lhs_total(row, column);
    }

    // Applying wake condition on the AUXILIARY_VELOCITY_POTENTIAL dofs
    if (data.distances[row] < 0.0)
        for (unsigned int column = 0; column < NumNodes; ++column)
            rLeftHandSideMatrix(row, column + NumNodes) = -lhs_total(row, column); // Side 1
    else if (data.distances[row] > 0.0)
        for (unsigned int column = 0; column < NumNodes; ++column)
            rLeftHandSideMatrix(row + NumNodes, column) = -lhs_total(row, column); // Side 2
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::AssignRightHandSideWakeNode(
    VectorType& rRightHandSideVector,
    const BoundedVector<double, NumNodes>& rUpper_rhs,
    const BoundedVector<double, NumNodes>& rLower_rhs,
    const BoundedVector<double, NumNodes>& rWake_rhs,
    const ElementalData<NumNodes, Dim>& rData,
    unsigned int& rRow) const
{
    if (rData.distances[rRow] > 0.0){
        rRightHandSideVector[rRow] = rUpper_rhs(rRow);
        rRightHandSideVector[rRow + NumNodes] = 0.0; //rWake_rhs(rRow);
    }
    else{
        rRightHandSideVector[rRow] = 0.0; //rWake_rhs(rRow);
        rRightHandSideVector[rRow + NumNodes] = rLower_rhs(rRow);
    }
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::ComputePotentialJump(const ProcessInfo& rCurrentProcessInfo)
{
    const array_1d<double, 3> free_stream_velocity = rCurrentProcessInfo[FREE_STREAM_VELOCITY];
    const double free_stream_velocity_norm = sqrt(inner_prod(free_stream_velocity, free_stream_velocity));
    const double reference_chord = rCurrentProcessInfo[REFERENCE_CHORD];

    array_1d<double, NumNodes> distances;
    GetWakeDistances(distances);

    auto r_geometry = GetGeometry();
    for (unsigned int i = 0; i < NumNodes; i++){
        double aux_potential = r_geometry[i].FastGetSolutionStepValue(AUXILIARY_VELOCITY_POTENTIAL);
        double potential = r_geometry[i].FastGetSolutionStepValue(VELOCITY_POTENTIAL);
        double potential_jump = aux_potential - potential;

        if (distances[i] > 0){
            r_geometry[i].SetLock();
            r_geometry[i].SetValue(POTENTIAL_JUMP, - (2.0 * potential_jump) / (free_stream_velocity_norm * reference_chord));
            r_geometry[i].UnSetLock();
        }
        else{
            r_geometry[i].SetLock();
            r_geometry[i].SetValue(POTENTIAL_JUMP, (2.0 * potential_jump) / (free_stream_velocity_norm * reference_chord));
            r_geometry[i].UnSetLock();
        }
    }
}

// serializer

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::save(Serializer& rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);
}

template <int Dim, int NumNodes>
void IncompressiblePerturbationPotentialFlowElement<Dim, NumNodes>::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Template class instantiation

template class IncompressiblePerturbationPotentialFlowElement<2, 3>;
template class IncompressiblePerturbationPotentialFlowElement<3, 4>;

} // namespace Kratos
