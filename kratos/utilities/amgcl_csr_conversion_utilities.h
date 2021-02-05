
//    |  /           | 
//    ' /   __| _` | __|  _ \   __| 
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/ 
//                   Multi-Physics  
//
//  License:		 BSD License 
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Denis Demidov
//                   Riccardo Rossi
//                   
//

#if !defined(KRATOS_CSR_CONVERSION_UTILITIES_H_INCLUDED)
#define  KRATOS_CSR_CONVERSION_UTILITIES_H_INCLUDED

#include "containers/csr_matrix.h"

#include <amgcl/backend/builtin.hpp>
#include <amgcl/adapter/zero_copy.hpp>

namespace Kratos
{

/**
Utilities to convert the distributed_csr matrix to other libraries
 */
class AmgclCSRConversionUtilities
{

public:

    /**
     This function returns a shared pointer to an Amgcl distributed_matrix
     */
	template< class TDataType, class TIndexType >
	static Kratos::shared_ptr<typename amgcl::backend::builtin<TDataType>::matrix > ConvertToAmgcl(
			const CsrMatrix<TDataType, TIndexType>& rA)
	{
    	Kratos::shared_ptr<typename amgcl::backend::builtin<TDataType>::matrix > pAmgcl = amgcl::adapter::zero_copy(
                rA.size1(),
                rA.index1_data().data().begin(),
                rA.index2_data().data().begin(),
                rA.value_data().data().begin()
            );
        return pAmgcl;
	}

};

}

#endif // KRATOS_CSR_CONVERSION_UTILITIES_H_INCLUDED