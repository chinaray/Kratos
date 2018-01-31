//
//   Project Name:        Kratos
//   Last Modified by:    $Author: ajarauta $
//   Date:                $Date: 2007-11-06 12:34:26 $
//   Revision:            $Revision: 1.4 $
//
//  this process save structural elements in a separate list

#if !defined(FIND_TRIPLE_POINT_CONDITION_INCLUDED )
#define  FIND_TRIPLE_POINT_CONDITION_INCLUDED



// System includes
#include <string>
#include <iostream>
#include <algorithm>

// External includes


// Project includes
#include "includes/define.h"
#include "includes/model_part.h"
#include "includes/node.h"
#include "utilities/math_utils.h"
#include "utilities/geometry_utilities.h"
#include "processes/process.h"
#include "includes/condition.h"
#include "includes/element.h"
#include "ULF_application.h"
//#include "custom_conditions/Surface_Tension2D.h"



namespace Kratos
{
  class FindTriplePoint
  : public Process
  {
  public:
    void FindTriplePoint2D(ModelPart& ThisModelPart)
    {
      KRATOS_TRY
      
      double mean_ystr = 0.0;
      double xmin = 0.0;
      double xmax = 0.0;
      int num_istruct = 0;
      //FIRST STEP: find IS_STRUCTURE nodes and their mean y coordinate, min and max x coordinate
      for(ModelPart::NodesContainerType::iterator im = ThisModelPart.NodesBegin() ; im != ThisModelPart.NodesEnd() ; im++)
      {
	  if (im->FastGetSolutionStepValue(IS_STRUCTURE) != 0.0)
	  {
	      mean_ystr += im->Y();
	      num_istruct++;
	  }
      }
      mean_ystr /= num_istruct;
      
      double dist = 0.0;
      double eps_tol = 1.0E-6;
      //SECON STEP: correct the flags
      for(ModelPart::NodesContainerType::iterator im = ThisModelPart.NodesBegin() ; im != ThisModelPart.NodesEnd() ; im++)
      {
	  if (im->FastGetSolutionStepValue(IS_BOUNDARY) != 0.0 && im->FastGetSolutionStepValue(IS_LAGRANGIAN_INLET) == 0.0)
	  {
	      dist = sqrt((im->Y() - mean_ystr)*(im->Y() - mean_ystr));
	      if (dist < eps_tol)
	      {
		  im->FastGetSolutionStepValue(IS_FREE_SURFACE) = 0.0;
		  im->FastGetSolutionStepValue(IS_INTERFACE) = 0.0;
		  im->FastGetSolutionStepValue(IS_STRUCTURE) = 1.0;
	      }
	      else
	      {
		  im->FastGetSolutionStepValue(IS_FREE_SURFACE) = 1.0;
		  im->FastGetSolutionStepValue(IS_INTERFACE) = 1.0;
		  im->FastGetSolutionStepValue(TRIPLE_POINT) = 0.0;
		  im->FastGetSolutionStepValue(IS_STRUCTURE) = 0.0;		  
	      }
	  }
      }
      
      
      //THIRD STEP: apply TRIPLE_POINT flag
      double x_mean = 0.0;
      double num_tp = 0.0;
      for(ModelPart::NodesContainerType::iterator im = ThisModelPart.NodesBegin() ; im != ThisModelPart.NodesEnd() ; im++)
      {
	  if (im->FastGetSolutionStepValue(IS_BOUNDARY) != 0.0)
	  {
	      WeakPointerVector< Node<3> >& neighb = im->GetValue(NEIGHBOUR_NODES);
	      double is_free = 0.0;
	      double is_struct = 0.0;
	      double is_lagin = 0.0;
	      int neighnum = 0;
	      
	      for (unsigned int i = 0; i < neighb.size(); i++)
	      {
		  if (neighb[i].FastGetSolutionStepValue(IS_BOUNDARY) != 0.0)
		  {
		    is_free += neighb[i].FastGetSolutionStepValue(IS_FREE_SURFACE);
		    is_struct += neighb[i].FastGetSolutionStepValue(IS_STRUCTURE);
		    is_lagin += neighb[i].FastGetSolutionStepValue(IS_LAGRANGIAN_INLET);
		  }
		  neighnum++;
	      }
	      
	      if (is_free < 1.1 && is_struct < 1.1 && is_lagin == 0.0 && (im->FastGetSolutionStepValue(IS_FREE_SURFACE) < 0.5))
	      {
		  im->FastGetSolutionStepValue(TRIPLE_POINT) = 1.0;
		  x_mean += im->X();
		  num_tp++;
	      }
	      
	      if (is_struct > 0.5 && is_lagin > 0.5 && (im->FastGetSolutionStepValue(IS_STRUCTURE) != 0.0))
	      {
		  im->FastGetSolutionStepValue(SOLID_FRACTION_GRADIENT_X) = 1.0;
		  im->FastGetSolutionStepValue(TRIPLE_POINT) = 0.0;
	      }
	      if (is_free > 0.5 && is_lagin > 0.5 && (im->FastGetSolutionStepValue(IS_STRUCTURE) != 0.0))
	      {
		  im->FastGetSolutionStepValue(SOLID_FRACTION_GRADIENT_X) = 0.0;
		  im->FastGetSolutionStepValue(TRIPLE_POINT) = 1.0;
		  x_mean += im->X();
		  num_tp++;
	      }
	      
	  }
      }
      
      if (num_tp != 0.0)
	  x_mean /= num_tp;
      else
	  x_mean = 0.0;
      
      int num_trip = 0;
      //FOURTH STEP: correct TRIPLE_POINT flag and count how many are there
      for(ModelPart::NodesContainerType::iterator im = ThisModelPart.NodesBegin() ; im != ThisModelPart.NodesEnd() ; im++)
      {
	  if (im->FastGetSolutionStepValue(TRIPLE_POINT) != 0.0)
	  {
	    if (num_tp != 1.0)
	      im->FastGetSolutionStepValue(TRIPLE_POINT) = 1.0;
	    else
	      im->FastGetSolutionStepValue(TRIPLE_POINT) = 1.0;
	    num_trip++;
	  }
      }
      
     
      
      KRATOS_CATCH("")
    }
    
    void FindTriplePoint3D(ModelPart& ThisModelPart)
    {
      KRATOS_TRY
      
      unsigned int num_tp = 0;
      unsigned int num_fs = 0;
      array_1d<double,3> An = ZeroVector(3);
      
      for(ModelPart::NodesContainerType::iterator im = ThisModelPart.NodesBegin() ; im != ThisModelPart.NodesEnd() ; im++)
      {
	  num_fs = num_tp = 0;
	  if ((im->FastGetSolutionStepValue(TRIPLE_POINT) != 0.0) && (im->FastGetSolutionStepValue(CONTACT_ANGLE) == 0.0) && (im->FastGetSolutionStepValue(VELOCITY_X) != 0.0))
	      im->FastGetSolutionStepValue(TRIPLE_POINT) = 0.0;
	  
	  An = im->FastGetSolutionStepValue(NORMAL);
	  NormalizeVec3D(An);
	  if (im->FastGetSolutionStepValue(TRIPLE_POINT) != 0.0 && An[2] < -0.99)
	  {
	      im->FastGetSolutionStepValue(TRIPLE_POINT) = 0.0;
	      im->FastGetSolutionStepValue(CONTACT_ANGLE) = 0.0;
	      im->FastGetSolutionStepValue(FORCE) = ZeroVector(3);
	  }
	  
	  if (im->FastGetSolutionStepValue(IS_STRUCTURE) != 0.0)
	  {
	    if( An[0] > 0.01 || An[0] < -0.01 || An[1] > 0.01 || An[1] < -0.01)
	    {
      im->FastGetSolutionStepValue(TRIPLE_POINT) = 1.0;
	    }
	    else
		im->FastGetSolutionStepValue(TRIPLE_POINT) = 0.0;
	  }
      }


      KRATOS_CATCH("")
    }    

    double Norm3D(const array_1d<double,3>& a)
    {
      return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    }

    double DotProduct3D(const array_1d<double,3>& a, const array_1d<double,3>& b)
    {
      return (a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
    }   
    
    void NormalizeVec3D(array_1d<double,3>& input)
    {
      double norm = Norm3D(input);
      if (norm != 0.0)
      {
	input[0] /= norm;
	input[1] /= norm;
	input[2] /= norm;
      }
    } 
    
    private:
    
  }; // Class FindTriplePoint


}  // namespace Kratos.

#endif // KRATOS_CREATE_INTERFACE_CONDITIONS_PROCESS_INCLUDED  defined 


