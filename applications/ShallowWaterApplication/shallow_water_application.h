//   
//   Project Name:        Kratos       
//   Last Modified by:    Miguel Masó Sotomayor
//   Date:                April 26th 2017
//   Revision:            1.5
//
//


#if !defined(KRATOS_SHALLOW_WATER_APPLICATION_H_INCLUDED )
#define  KRATOS_SHALLOW_WATER_APPLICATION_H_INCLUDED



// System includes
#include <string>
#include <iostream> 


// External includes 


// Project includes
#include "includes/define.h"
#include "includes/kratos_application.h"
#include "includes/variables.h"
#include "includes/condition.h"
#include "includes/ublas_interface.h"

// Shallow water includes
#include "shallow_water_application_variables.h"
#include "custom_elements/primitive_var_element.hpp"
#include "custom_elements/conserved_var_element.hpp"
#include "custom_elements/euler_prim_var_element.hpp"
#include "custom_elements/euler_cons_var_element.hpp"
#include "custom_conditions/rain_condition.hpp"
#include "custom_conditions/nothing_condition.hpp"


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

    /// Short class definition.
    /** Detail class definition.
    */
    class KratosShallowWaterApplication : public KratosApplication
    {
    public:
        ///@name Type Definitions
        ///@{
        

        /// Pointer definition of KratosShallowWaterApplication
        KRATOS_CLASS_POINTER_DEFINITION(KratosShallowWaterApplication);

        ///@}
        ///@name Life Cycle 
        ///@{ 

        /// Default constructor.
        KratosShallowWaterApplication();

        /// Destructor.
        virtual ~KratosShallowWaterApplication(){}


        ///@}
        ///@name Operators 
        ///@{


        ///@}
        ///@name Operations
        ///@{

        virtual void Register();



        ///@}
        ///@name Access
        ///@{ 


        ///@}
        ///@name Inquiry
        ///@{


        ///@}      
        ///@name Input and output
        ///@{

        /// Turn back information as a string.
        virtual std::string Info() const
        {
            return "KratosShallowWaterApplication";
        }

        /// Print information about this object.
        virtual void PrintInfo(std::ostream& rOStream) const
        {
            rOStream << Info();
            PrintData(rOStream);
        }

        ///// Print object's data.
        virtual void PrintData(std::ostream& rOStream) const
        {
            KRATOS_WATCH("in my application");
            KRATOS_WATCH(KratosComponents<VariableData>::GetComponents().size() );
            rOStream << "Variables:" << std::endl;
            KratosComponents<VariableData>().PrintData(rOStream);
            rOStream << std::endl;
            rOStream << "Elements:" << std::endl;
            KratosComponents<Element>().PrintData(rOStream);
            rOStream << std::endl;
            rOStream << "Conditions:" << std::endl;
            KratosComponents<Condition>().PrintData(rOStream);
        }


        ///@}      
        ///@name Friends
        ///@{


        ///@}

    protected:
        ///@name Protected static Member Variables 
        ///@{ 


        ///@} 
        ///@name Protected member Variables 
        ///@{ 


        ///@} 
        ///@name Protected Operators
        ///@{ 


        ///@} 
        ///@name Protected Operations
        ///@{ 


        ///@} 
        ///@name Protected  Access 
        ///@{ 


        ///@}      
        ///@name Protected Inquiry 
        ///@{ 


        ///@}    
        ///@name Protected LifeCycle 
        ///@{ 


        ///@}

    private:
        ///@name Static Member Variables 
        ///@{ 


        ///@} 
        ///@name Member Variables 
        ///@{
        // Lagrangian elements 
        const PrimitiveVarElement<3> mPrimitiveVarElement2D3N;
        const PrimitiveVarElement<4> mPrimitiveVarElement2D4N;
        const ConservedVarElement<3> mConservedVarElement2D3N;
        const ConservedVarElement<4> mConservedVarElement2D4N;
        // Eulerian elements
        const EulerPrimVarElement<3> mEulerPrimVarElement2D3N;
        const EulerPrimVarElement<4> mEulerPrimVarElement2D4N;
        const EulerConsVarElement<3> mEulerConsVarElement2D3N;
        const EulerConsVarElement<4> mEulerConsVarElement2D4N;
        // Conditions
        const RainCondition<3> mRainCondition2D3N;
        const RainCondition<4> mRainCondition2D4N;
        //
        const NothingCondition<2> mNothingCondition2D2N;


        ///@} 
        ///@name Private Operators
        ///@{ 


        ///@} 
        ///@name Private Operations
        ///@{ 


        ///@} 
        ///@name Private  Access 
        ///@{ 


        ///@}    
        ///@name Private Inquiry 
        ///@{ 


        ///@}    
        ///@name Un accessible methods 
        ///@{ 

        /// Assignment operator.
        KratosShallowWaterApplication& operator=(KratosShallowWaterApplication const& rOther);

        /// Copy constructor.
        KratosShallowWaterApplication(KratosShallowWaterApplication const& rOther);


        ///@}    

    }; // Class KratosShallowWaterApplication 

    ///@} 


    ///@name Type Definitions       
    ///@{ 


    ///@} 
    ///@name Input and output 
    ///@{ 

    ///@} 


}  // namespace Kratos.

#endif // KRATOS_SHALLOW_WATER_APPLICATION_H_INCLUDED  defined 
