//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics FemDem Application
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Alejandro Cornejo Velazquez
//

// System includes

// External includes

// Project includes
#include "generic_small_strain_femdem_element.hpp"
#include "fem_to_dem_application_variables.h"
#include "solid_mechanics_application_variables.h"

namespace Kratos
{

//***********************DEFAULT CONSTRUCTOR******************************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
GenericSmallStrainFemDemElement<TDim, TyieldSurf>::GenericSmallStrainFemDemElement(IndexType NewId, GeometryType::Pointer pGeometry)
	: SmallDisplacementElement(NewId, pGeometry)
{
	//DO NOT ADD DOFS HERE!!!
}
//******************************CONSTRUCTOR*******************************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
GenericSmallStrainFemDemElement<TDim, TyieldSurf>::GenericSmallStrainFemDemElement(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
	: SmallDisplacementElement(NewId, pGeometry, pProperties)
{
	// BY DEFAULT, THE GEOMETRY WILL DEFINE THE INTEGRATION METHOD
	mThisIntegrationMethod = GetGeometry().GetDefaultIntegrationMethod();

	// Each component == Each edge
	mNonConvergedThresholds = ZeroVector(NumberOfEdges);   // Equivalent stress
	mThresholds = ZeroVector(NumberOfEdges); // Stress mThreshold on edge
	mDamages = ZeroVector(NumberOfEdges); // Converged mDamage on each edge
	mNonConvergedDamages = ZeroVector(NumberOfEdges); // mDamages on edges of "i" iteration
}

//******************************COPY CONSTRUCTOR**************************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
GenericSmallStrainFemDemElement<TDim,TyieldSurf>::GenericSmallStrainFemDemElement(GenericSmallStrainFemDemElement const &rOther)
	: SmallDisplacementElement(rOther)
{
}

//*******************************ASSIGMENT OPERATOR***********************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
GenericSmallStrainFemDemElement<TDim, TyieldSurf> &GenericSmallStrainFemDemElement<TDim,TyieldSurf>::operator=(GenericSmallStrainFemDemElement const &rOther)
{
	SmallDisplacementElement::operator=(rOther);
	return *this;
}

//*********************************OPERATIONS*****************************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
Element::Pointer GenericSmallStrainFemDemElement<TDim,TyieldSurf>::Create(IndexType NewId, NodesArrayType const &rThisNodes, PropertiesType::Pointer pProperties) const
{
	return Element::Pointer(new GenericSmallStrainFemDemElement(NewId, GetGeometry().Create(rThisNodes), pProperties));
}

//************************************CLONE*******************************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
Element::Pointer GenericSmallStrainFemDemElement<TDim,TyieldSurf>::Clone(IndexType NewId, NodesArrayType const &rThisNodes) const
{
	GenericSmallStrainFemDemElement NewElement(NewId, GetGeometry().Create(rThisNodes), pGetProperties());
	return Element::Pointer(new GenericSmallStrainFemDemElement(NewElement));
}

//*******************************DESTRUCTOR*******************************************
//************************************************************************************
template<unsigned int TDim, unsigned int TyieldSurf>
GenericSmallStrainFemDemElement<TDim,TyieldSurf>::~GenericSmallStrainFemDemElement()
{
}

template class GenericSmallStrainFemDemElement<2, 0>;
template class GenericSmallStrainFemDemElement<2, 1>;
template class GenericSmallStrainFemDemElement<2, 2>;
template class GenericSmallStrainFemDemElement<2, 3>;
template class GenericSmallStrainFemDemElement<2, 4>;
template class GenericSmallStrainFemDemElement<2, 5>;
template class GenericSmallStrainFemDemElement<2, 6>;
template class GenericSmallStrainFemDemElement<2, 7>;
template class GenericSmallStrainFemDemElement<3, 0>;
template class GenericSmallStrainFemDemElement<3, 1>;
template class GenericSmallStrainFemDemElement<3, 2>;
template class GenericSmallStrainFemDemElement<3, 3>;
template class GenericSmallStrainFemDemElement<3, 4>;
template class GenericSmallStrainFemDemElement<3, 5>;
template class GenericSmallStrainFemDemElement<3, 6>;
template class GenericSmallStrainFemDemElement<3, 7>;
} // namespace Kratos