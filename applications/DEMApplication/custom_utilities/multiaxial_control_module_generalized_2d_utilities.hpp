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
//                   Guillermo Casas
//


#ifndef KRATOS_MULTIAXIAL_CONTROL_MODULE_GENERALIZED_2D_UTILITIES
#define KRATOS_MULTIAXIAL_CONTROL_MODULE_GENERALIZED_2D_UTILITIES

// /* External includes */

// System includes

// Project includes
#include "includes/variables.h"

/* System includes */
#include <limits>
#include <iostream>
#include <iomanip>
#include <map>

/* External includes */
#ifdef _OPENMP
#include <omp.h>
#endif

/* Project includes */
#include "geometries/geometry.h"
#include "includes/define.h"
#include "includes/model_part.h"

#include "includes/table.h"
#include "includes/kratos_parameters.h"
#include "utilities/math_utils.h"

// Application includes
#include "custom_elements/spheric_continuum_particle.h"

#include "DEM_application_variables.h"


namespace Kratos
{
class MultiaxialControlModuleGeneralized2DUtilities
{
public:

KRATOS_CLASS_POINTER_DEFINITION(MultiaxialControlModuleGeneralized2DUtilities);

/// Defining a table with double argument and result type as table type.
typedef Table<double,double> TableType;

/// Default constructor.

MultiaxialControlModuleGeneralized2DUtilities(ModelPart& rDemModelPart,
                                ModelPart& rFemModelPart,
                                Parameters& rParameters
                                ) :
                                mrDemModelPart(rDemModelPart),
                                mrFemModelPart(rFemModelPart)
{
    KRATOS_TRY

    Parameters default_parameters( R"(
        {
            "Parameters"    : {
                "control_module_delta_time": 1.0e-5,
                "velocity_factor" : 1.0,
                "stress_increment_tolerance": 1.0e-3,
                "update_stiffness": true,
                "stiffness_alpha": 1.0,
                "start_time" : 0.0,
                "stress_averaging_time": 1.0e-5,
                "axisymmetry": false
            },
            "list_of_actuators" : []
        }  )" );

    // Now validate agains defaults -- this also ensures no type mismatch
    rParameters.ValidateAndAssignDefaults(default_parameters);

    mVectorOfLastStresses.resize(0);
    mVelocityFactor = rParameters["Parameters"]["velocity_factor"].GetDouble();
    mStartTime = rParameters["Parameters"]["start_time"].GetDouble();
    mStressIncrementTolerance = rParameters["Parameters"]["stress_increment_tolerance"].GetDouble();
    mUpdateStiffness = rParameters["Parameters"]["update_stiffness"].GetBool();
    mStressAveragingTime = rParameters["Parameters"]["stress_averaging_time"].GetDouble();
    mStiffnessAlpha = rParameters["Parameters"]["stiffness_alpha"].GetDouble();
    mCMDeltaTime = rParameters["Parameters"]["control_module_delta_time"].GetDouble();
    mAxisymmetry = rParameters["Parameters"]["axisymmetry"].GetBool();
    mStep = 0;
    mCMTime = 0.0;

    const unsigned int number_of_actuators = rParameters["list_of_actuators"].size();
    // TODO: For the moment we assume that for axisymmetry cases 2 actuators are used
    //       and for the rest of cases 3 actuators are used (not fully general)
    if (number_of_actuators < 2 || number_of_actuators > 3) {
        KRATOS_ERROR << "The number of actuators is neither 2 (axisymmetric) nor 3 (triaxial)" << std::endl;
    }
    mVelocity.resize(number_of_actuators, false);
    mLimitVelocities.resize(number_of_actuators, false);
    mReactionStress.resize(number_of_actuators, false);
    mStiffness.resize(number_of_actuators,number_of_actuators,false);
    noalias(mStiffness) = ZeroMatrix(number_of_actuators,number_of_actuators);
    mDeltaDisplacement.resize(number_of_actuators,number_of_actuators,false);
    noalias(mDeltaDisplacement) = ZeroMatrix(number_of_actuators,number_of_actuators);
    mDeltaReactionStress.resize(number_of_actuators,number_of_actuators,false);
    noalias(mDeltaReactionStress) = ZeroMatrix(number_of_actuators,number_of_actuators);
    for (unsigned int i = 0; i < number_of_actuators; i++) {
        const unsigned int number_of_dem_boundaries = rParameters["list_of_actuators"][i]["list_of_dem_boundaries"].size();
        const unsigned int number_of_fem_boundaries = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"].size();
        std::vector<ModelPart*> list_of_dem_submodelparts(number_of_dem_boundaries);
        std::vector<ModelPart*> list_of_fem_submodelparts(number_of_fem_boundaries);
        std::vector<array_1d<double,3>> list_of_outer_normals(number_of_fem_boundaries);
        for (unsigned int j = 0; j < number_of_dem_boundaries; j++) {
            list_of_dem_submodelparts[j] = 
                &(mrDemModelPart.GetSubModelPart(
                rParameters["list_of_actuators"][i]["list_of_dem_boundaries"][j]["model_part_name"].GetString()
                ));
        }
        for (unsigned int j = 0; j < number_of_fem_boundaries; j++) {
            list_of_fem_submodelparts[j] = 
                &(rFemModelPart.GetSubModelPart(
                rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["model_part_name"].GetString()
                ));
            list_of_outer_normals[j][0] = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["outer_normal"][0].GetDouble();
            list_of_outer_normals[j][1] = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["outer_normal"][1].GetDouble();
            list_of_outer_normals[j][2] = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["outer_normal"][2].GetDouble();
        }
        const std::string actuator_name = rParameters["list_of_actuators"][i]["Parameters"]["actuator_name"].GetString();
        mDEMBoundariesSubModelParts[actuator_name] = list_of_dem_submodelparts;
        mFEMBoundariesSubModelParts[actuator_name] = list_of_fem_submodelparts;
        mOuterNormals[actuator_name] = list_of_outer_normals;
        mTargetStressTableIds[actuator_name] = rParameters["list_of_actuators"][i]["Parameters"]["target_stress_table_id"].GetInt();
        const double compression_length = rParameters["list_of_actuators"][i]["Parameters"]["compression_length"].GetDouble();
        const double initial_velocity = rParameters["list_of_actuators"][i]["Parameters"]["initial_velocity"].GetDouble();
        const double limit_velocity = rParameters["list_of_actuators"][i]["Parameters"]["limit_velocity"].GetDouble();
        const double stiffness = rParameters["list_of_actuators"][i]["Parameters"]["young_modulus"].GetDouble()/compression_length; // mStiffness is actually a stiffness over an area
        if (actuator_name == "X") {
            mVelocity[0] = initial_velocity;
            mLimitVelocities[0] = limit_velocity;
            mStiffness(0,0) = stiffness;
        } else if (actuator_name == "Y") {
            mVelocity[1] = initial_velocity;
            mLimitVelocities[1] = limit_velocity;
            mStiffness(1,1) = stiffness;
        } else if (actuator_name == "Z") {
            if (mAxisymmetry == false) {
                mVelocity[2] = initial_velocity;
                mLimitVelocities[2] = limit_velocity;
                mStiffness(2,2) = stiffness;
            } else {
                mVelocity[1] = initial_velocity;
                mLimitVelocities[1] = limit_velocity;
                mStiffness(1,1) = stiffness;
            }
        } else if (actuator_name == "Radial") {
            mVelocity[0] = initial_velocity;
            mLimitVelocities[0] = limit_velocity;
            mStiffness(0,0) = stiffness;
        } else {
            KRATOS_ERROR << actuator_name << " is not a valid actuator name" << std::endl;
        }
        mReactionStress[i] = 0.0;
    }

    mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = 0.0;

    KRATOS_CATCH("");
}

/// Destructor.

virtual ~MultiaxialControlModuleGeneralized2DUtilities(){}

//***************************************************************************************************************
//***************************************************************************************************************

// Before FEM and DEM solution
void ExecuteInitialize()
{
    KRATOS_TRY;

    // Iterate through all actuators
    std::map<std::string, std::vector<ModelPart*>>::iterator map_it = mFEMBoundariesSubModelParts.begin();
    for (; map_it!=mFEMBoundariesSubModelParts.end(); map_it++) {
        const std::string actuator_name = map_it->first;
        std::vector<ModelPart*> SubModelPartList = map_it->second;
        if (actuator_name == "Radial") {
            // In axisymmetric cases we assume there is only 1 actuator in the FEM boundary
            ModelPart& rSubModelPart = *(SubModelPartList[0]);
            // Iterate through nodes of Fem boundary
            const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
            ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
            #pragma omp parallel for
            for(int i = 0; i<NNodes; i++) {
                ModelPart::NodesContainerType::iterator it = it_begin + i;
                const double external_radius = std::sqrt(it->X()*it->X() + it->Y()*it->Y());
                const double cos_theta = it->X()/external_radius;
                const double sin_theta = it->Y()/external_radius;
                array_1d<double,3>& r_displacement = it->FastGetSolutionStepValue(DISPLACEMENT);
                array_1d<double,3>& r_delta_displacement = it->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
                array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                noalias(r_displacement) = ZeroVector(3);
                noalias(r_delta_displacement) = ZeroVector(3);
                r_velocity[0] = mVelocity[0] * cos_theta;
                r_velocity[1] = mVelocity[0] * sin_theta;
                r_velocity[2] = 0.0;
            }
        } else if (actuator_name == "Z") {
            mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] = 0.0;
        } else {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < SubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(SubModelPartList[i]);
                // Unit normal vector pointing outwards
                array_1d<double,3> n;
                n[0] = mOuterNormals[actuator_name][i][0];
                n[1] = mOuterNormals[actuator_name][i][1];
                n[2] = 0.0;
                double inv_norm = 1.0/norm_2(n);
                n[0] *= inv_norm;
                n[1] *= inv_norm;
                // Initial velocity * normal
                double norm_vel = inner_prod(mVelocity,n)
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_displacement = it->FastGetSolutionStepValue(DISPLACEMENT);
                    array_1d<double,3>& r_delta_displacement = it->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
                    array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                    noalias(r_displacement) = ZeroVector(3);
                    noalias(r_delta_displacement) = ZeroVector(3);
                    r_velocity[0] = norm_vel * n[0];
                    r_velocity[1] = norm_vel * n[1];
                    r_velocity[2] = 0.0;
                }
            }
        }
    }

    KRATOS_CATCH("");
}

// Before FEM and DEM solution
void ExecuteInitializeSolutionStep()
{
    KRATOS_TRY;

    const double current_time = mrDemModelPart.GetProcessInfo()[TIME];

    // Update velocities
    if (mCMTime < current_time) {
        
        mCMTime += mCMDeltaTime;
        mStep += 1;

        const unsigned int number_of_actuators = mFEMBoundariesSubModelParts.size();
        Vector next_target_stress(number_of_actuators);
        noalias(next_target_stress) = ZeroVector(number_of_actuators);
        Vector target_stress_perturbation(number_of_actuators);
        noalias(target_stress_perturbation) = ZeroVector(number_of_actuators); // TODO
        noalias(next_target_stress) += target_stress_perturbation;
        Matrix k_inverse(number_of_actuators,number_of_actuators);
        double k_det = 0.0;
        MathUtils<double>::InvertMatrix(mStiffness, k_inverse, k_det);

        // Iterate through all actuators
        std::map<std::string, std::vector<ModelPart*>>::iterator map_it = mFEMBoundariesSubModelParts.begin();
        for (; map_it!=mFEMBoundariesSubModelParts.end(); map_it++) {
            const std::string actuator_name = map_it->first;
            std::vector<ModelPart*> FEMSubModelPartList = map_it->second;
            std::vector<ModelPart*> DEMSubModelPartList = mDEMBoundariesSubModelParts[actuator_name];
            unsigned int target_stress_table_id = mTargetStressTableIds[actuator_name];
            if (actuator_name == "X") {
                TableType::Pointer pFEMTargetStressTable = *(FEMSubModelPartList[0]).pGetTable(target_stress_table_id);
                next_target_stress[0] = pFEMTargetStressTable->GetValue(mCMTime);
            } else if (actuator_name == "Y") {
                TableType::Pointer pFEMTargetStressTable = *(FEMSubModelPartList[0]).pGetTable(target_stress_table_id);
                next_target_stress[1] = pFEMTargetStressTable->GetValue(mCMTime);
            } else if (actuator_name == "Z") {
                TableType::Pointer pDEMTargetStressTable = *(DEMSubModelPartList[0]).pGetTable(target_stress_table_id);
                if (mAxisymmetry == false) {
                    next_target_stress[2] = pDEMTargetStressTable->GetValue(mCMTime);
                } else {
                    next_target_stress[1] = pTargetStressTable->GetValue(mCMTime);
                }
            } else if (actuator_name == "Radial") {
                TableType::Pointer pFEMTargetStressTable = *(FEMSubModelPartList[0]).pGetTable(target_stress_table_id);
                next_target_stress[0] = pFEMTargetStressTable->GetValue(mCMTime);
            }
        }

        Vector delta_velocity(number_of_actuators);
        Vector delta_stress_target(number_of_actuators);
        noalias(delta_stress_target) = next_target_stress-mReactionStress;
        const double delta_stress_target_norm = norm_2(delta_stress_target);
        // TODO: is this right ?
        if (delta_stress_target_norm < mStressIncrementTolerance) {
            noalias(delta_velocity) = -mVelocity;
        } else {
            noalias(delta_velocity) = prod(k_inverse,delta_stress_target)/mCMDeltaTime - mVelocity;
        }

        noalias(mVelocity) += mVelocityFactor * delta_velocity;

        // TODO: is this right ?
        for (unsigned int i = 0; i < mVelocity.size(); i++) {
            if (std::abs(mVelocity[i]) > std::abs(mLimitVelocities[i])) {
                if (mVelocity[i] > 0.0) {
                    mVelocity[i] = std::abs(mLimitVelocities[i]);
                } else {
                    mVelocity[i] = - std::abs(mLimitVelocities[i]);
                }
            }
        }
    }

    // Move Actuators
    const double delta_time = mrDemModelPart.GetProcessInfo()[DELTA_TIME];
    // Iterate through all actuators
    std::map<std::string, std::vector<ModelPart*>>::iterator map_it = mFEMBoundariesSubModelParts.begin();
    for (; map_it!=mFEMBoundariesSubModelParts.end(); map_it++) {
        const std::string actuator_name = map_it->first;
        std::vector<ModelPart*> SubModelPartList = map_it->second;
        if (actuator_name == "Radial") {
            // In axisymmetric cases we assume there is only 1 actuator in the FEM boundary
            ModelPart& rSubModelPart = *(SubModelPartList[0]);
            // Iterate through nodes of Fem boundary
            const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
            ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
            #pragma omp parallel for
            for(int i = 0; i<NNodes; i++) {
                ModelPart::NodesContainerType::iterator it = it_begin + i;
                const double external_radius = std::sqrt(it->X()*it->X() + it->Y()*it->Y());
                const double cos_theta = it->X()/external_radius;
                const double sin_theta = it->Y()/external_radius;
                it->FastGetSolutionStepValue(VELOCITY_X) = mVelocity[0] * cos_theta;
                it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_X) = it->FastGetSolutionStepValue(VELOCITY_X) * delta_time;
                it->FastGetSolutionStepValue(DISPLACEMENT_X) += it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_X);
                it->X() = it->X0() + it->FastGetSolutionStepValue(DISPLACEMENT_X);
                it->FastGetSolutionStepValue(VELOCITY_Y) = mVelocity[0] * sin_theta;
                it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_Y) = it->FastGetSolutionStepValue(VELOCITY_Y) * delta_time;
                it->FastGetSolutionStepValue(DISPLACEMENT_Y) += it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_Y);
                it->Y() = it->Y0() + it->FastGetSolutionStepValue(DISPLACEMENT_Y);
            }
        } else if (actuator_name == "Z") {
            if (mAxisymmetry == false) {
                mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] += mVelocity[2]*delta_time/1.0;
            } else {
                mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] += mVelocity[1]*delta_time/1.0;
            }
        } else {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < SubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(SubModelPartList[i]);
                // Unit normal vector pointing outwards
                array_1d<double,3> n;
                n[0] = mOuterNormals[actuator_name][i][0];
                n[1] = mOuterNormals[actuator_name][i][1];
                n[2] = 0.0;
                double inv_norm = 1.0/norm_2(n);
                n[0] *= inv_norm;
                n[1] *= inv_norm;
                // Initial velocity * normal
                double norm_vel = inner_prod(mVelocity,n)
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    it->FastGetSolutionStepValue(VELOCITY_X) = norm_vel * n[0];
                    it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_X) = it->FastGetSolutionStepValue(VELOCITY_X) * delta_time;
                    it->FastGetSolutionStepValue(DISPLACEMENT_X) += it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_X);
                    it->X() = it->X0() + it->FastGetSolutionStepValue(DISPLACEMENT_X);
                    it->FastGetSolutionStepValue(VELOCITY_Y) = norm_vel * n[1];
                    it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_Y) = it->FastGetSolutionStepValue(VELOCITY_Y) * delta_time;
                    it->FastGetSolutionStepValue(DISPLACEMENT_Y) += it->FastGetSolutionStepValue(DELTA_DISPLACEMENT_Y);
                    it->Y() = it->Y0() + it->FastGetSolutionStepValue(DISPLACEMENT_Y);
                }
            }
        }
    }

    KRATOS_CATCH("");
}

// After FEM and DEM solution
void ExecuteFinalizeSolutionStep()
{
    // TODO: here
    // Update ReactionStresses, Stiffness matrix and print results

    // Update K with latest ReactionStress after the axis has been loaded
    // if (mApplyCM == true) {
    //     if (mAlternateAxisLoading == true) {
    //         const double delta_time = mrDemModelPart.GetProcessInfo()[DELTA_TIME];
    //         double ReactionStress = CalculateReactionStress();
    //         if(mUpdateStiffness == true) {
    //             mStiffness = EstimateStiffness(ReactionStress,delta_time);
    //         }
    //     }
    // }
}

//***************************************************************************************************************
//***************************************************************************************************************

///@}
///@name Inquiry
///@{


///@}
///@name Input and output
///@{

/// Turn back information as a stemplate<class T, std::size_t dim> tring.

virtual std::string Info() const
{
    return "";
}

/// Print information about this object.

virtual void PrintInfo(std::ostream& rOStream) const
{
}

/// Print object's data.

virtual void PrintData(std::ostream& rOStream) const
{
}


///@}
///@name Friends
///@{

///@}

protected:
///@name Protected static Member r_variables
///@{

    ModelPart& mrDemModelPart;
    ModelPart& mrFemModelPart;
    double mStartTime;
    double mStressAveragingTime;
    double mVelocityFactor;
    double mStressIncrementTolerance;
    bool mUpdateStiffness;
    double mCMDeltaTime;
    double mStiffnessAlpha;
    bool mAxisymmetry;
    unsigned int mStep; // TODO ?
    double mCMTime;
    std::map<std::string, std::vector<ModelPart*>> mFEMBoundariesSubModelParts; /// FEM SubModelParts associated to each boundary of every actuator
    std::map<std::string, std::vector<ModelPart*>> mDEMBoundariesSubModelParts; /// DEM SubModelParts associated to each boundary of every actuator
    std::map<std::string, std::vector<array_1d<double,3>>> mOuterNormals; /// OuterNormal associated to each FEM boundary of every actuator
    std::map<std::string, unsigned int> mTargetStressTableIds; /// TargetStressTableIds associated to every actuator
    std::vector<Vector> mVectorsOfLastStresses;
    Vector mLimitVelocities;
    Vector mVelocity;
    Vector mReactionStress;
    Matrix mStiffness;
    Matrix mDeltaDisplacement;
    Matrix mDeltaReactionStress;


///@}
///@name Protected member r_variables
///@{ template<class T, std::size_t dim>


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

///@name Static Member r_variables
///@{


///@}
///@name Member r_variables
///@{
///@}
///@name Private Operators
///@{

///@}
///@name Private Operations
///@{

double UpdateVectorOfHistoricalStressesAndComputeNewAverage(const double& last_reaction) {
    KRATOS_TRY;
    int length_of_vector = mVectorOfLastStresses.size();
    if (length_of_vector == 0) { //only the first time
        int number_of_steps_for_stress_averaging = (int) (mStressAveragingTime / mrDemModelPart.GetProcessInfo()[DELTA_TIME]);
        if(number_of_steps_for_stress_averaging < 1) number_of_steps_for_stress_averaging = 1;
        mVectorOfLastStresses.resize(number_of_steps_for_stress_averaging);
        KRATOS_INFO("DEM") << " 'number_of_steps_for_stress_averaging' is "<< number_of_steps_for_stress_averaging << std::endl;
    }

    length_of_vector = mVectorOfLastStresses.size();

    if(length_of_vector > 1) {
        for(int i=1; i<length_of_vector; i++) {
            mVectorOfLastStresses[i-1] = mVectorOfLastStresses[i];
        }
    }
    mVectorOfLastStresses[length_of_vector-1] = last_reaction;

    double average = 0.0;
    for(int i=0; i<length_of_vector; i++) {
        average += mVectorOfLastStresses[i];
    }
    average /= (double) length_of_vector;
    return average;

    KRATOS_CATCH("");
}

void IsTimeToApplyCM(){
    const double current_time = mrDemModelPart.GetProcessInfo()[TIME];
    mApplyCM = false;

    if(current_time >= mStartTime) {
        if (mAlternateAxisLoading == true) {
            const unsigned int step = mrDemModelPart.GetProcessInfo()[STEP];
            if(step == mZCounter){
                mApplyCM = true;
                mZCounter += 3;
            }
        } else {
            mApplyCM = true;
        }
    }
}

double CalculateReactionStress() {
    // DEM variables
    ModelPart::ElementsContainerType& rElements = mrDemModelPart.GetCommunicator().LocalMesh().Elements();
    // FEM variables
    const ProcessInfo& CurrentProcessInfo = mrDemModelPart.GetProcessInfo();
    int NElems = static_cast<int>(mrDemModelPart.Elements().size());
    ModelPart::ElementsContainerType::iterator elem_begin = mrDemModelPart.ElementsBegin();

    // Calculate face_area
    double face_area = 0.0;
    // DEM modelpart
    #pragma omp parallel for reduction(+:face_area)
    for (int i = 0; i < (int)rElements.size(); i++) {
        ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + i;
        Element* p_element = ptr_itElem->get();
        SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
        const double radius = pDemElem->GetRadius();
        face_area += Globals::Pi*radius*radius;
    }
    // FEM modelpart
    #pragma omp parallel for reduction(+:face_area)
    for(int i = 0; i < NElems; i++) {
        ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
        face_area += itElem->GetGeometry().Area();
    }

    // Calculate ReactionStress
    double face_reaction = 0.0;
    // DEM modelpart
    #pragma omp parallel for reduction(+:face_reaction)
    for (int i = 0; i < (int)rElements.size(); i++) {
        ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + i;
        Element* p_element = ptr_itElem->get();
        SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
        BoundedMatrix<double, 3, 3> stress_tensor = ZeroMatrix(3,3);
        noalias(stress_tensor) = (*(pDemElem->mSymmStressTensor));
        const double radius = pDemElem->GetRadius();
        face_reaction += stress_tensor(2,2) * Globals::Pi*radius*radius;
    }
    // FEM modelpart
    #pragma omp parallel for reduction(+:face_reaction)
    for(int i = 0; i < NElems; i++)
    {
        ModelPart::ElementsContainerType::iterator itElem = elem_begin + i;
        Element::GeometryType& rGeom = itElem->GetGeometry();
        GeometryData::IntegrationMethod MyIntegrationMethod = itElem->GetIntegrationMethod();
        const Element::GeometryType::IntegrationPointsArrayType& IntegrationPoints = rGeom.IntegrationPoints(MyIntegrationMethod);
        unsigned int NumGPoints = IntegrationPoints.size();
        std::vector<Vector> stress_vector(NumGPoints);
        itElem->CalculateOnIntegrationPoints( CAUCHY_STRESS_VECTOR, stress_vector, CurrentProcessInfo );
        const double area_over_gp = rGeom.Area()/NumGPoints;
        // Loop through GaussPoints
        for ( unsigned int GPoint = 0; GPoint < NumGPoints; GPoint++ )
        {
            face_reaction += stress_vector[GPoint][2] * area_over_gp;
        }
    }

    double reaction_stress;
    if (std::abs(face_area) > 1.0e-12) {
        reaction_stress = face_reaction / face_area;
    } else {
        reaction_stress = 0.0;
    }

    return reaction_stress;
}

double EstimateStiffness(const double& rReactionStress, const double& rDeltaTime) {
    double K_estimated = mStiffness;
    if(std::abs(mVelocity) > 1.0e-12 && std::abs(rReactionStress-mReactionStressOld) > mStressIncrementTolerance) {
        K_estimated = std::abs((rReactionStress-mReactionStressOld)/(mVelocity * rDeltaTime));
    }
    return K_estimated;
}

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
MultiaxialControlModuleGeneralized2DUtilities & operator=(MultiaxialControlModuleGeneralized2DUtilities const& rOther);


///@}

}; // Class MultiaxialControlModuleGeneralized2DUtilities

}  // namespace Python.

#endif // KRATOS_MULTIAXIAL_CONTROL_MODULE_GENERALIZED_2D_UTILITIES
