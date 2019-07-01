// KRATOS  __  __ _____ ____  _   _ ___ _   _  ____
//        |  \/  | ____/ ___|| | | |_ _| \ | |/ ___|
//        | |\/| |  _| \___ \| |_| || ||  \| | |  _
//        | |  | | |___ ___) |  _  || || |\  | |_| |
//        |_|  |_|_____|____/|_| |_|___|_| \_|\____| APPLICATION
//
//  License:         BSD License
//                   license: MeshingApplication/license.txt
//
//  Main authors:    Riccardo Tosi
//                   Brendan Keith
//

// System includes

// External includes

// Project includes
#include "custom_processes/metrics_divergencefree_process.h"
#include "utilities/variable_utils.h"

namespace Kratos
{
template<SizeType TDim>
MetricDivergenceFreeProcess<TDim>::MetricDivergenceFreeProcess(
        ModelPart& rThisModelPart,
        Parameters ThisParameters
        ):mrThisModelPart(rThisModelPart)
{
    /**
     * We configure using the following parameters:
     * minimal_size: The minimal size to consider on the remeshing
     * maximal_size: The maximal size to consider on the remeshing
     * refinement_strategy: The chosen refinement strategy
     * echo_level: The verbosity
     * reference_variable_name: The refinement indicator
     *
     * mean_distribution_strategy
     * target_refinement_coefficient: Coefficient to make the average element size throughout the domain change by a factor of target_refinement_coefficient after the refinement
     * refinement_bound: Additional tolerance to bound the density factor (refinement coefficient)
     * reference_norm_name: The refinement norm indicator, exploited to estimate some coefficients
     *
     * maximum_strategy
     * target_refinement_coefficient: The range of elements we want to refine w.r.t. the maximum value of reference_variable_name
     * refinement_coefficient: Coefficient to make the average element size throughout the domain change by a factor of target_refinement_coefficient after the refinement
     *
     * global_tolerance_strategy
     * global_tolerance: The error of interpolation allowed
     */
    Parameters default_parameters = Parameters(R"(
    {
        "minimal_size"                        : 0.01,
        "maximal_size"                        : 10.0,
        "refinement_strategy"                 : "global_tolerance_strategy",
        "reference_variable_name"             : "DIVERGENCE_WEIGHTED",
        "mean_distribution_strategy":
        {
            "target_refinement_coefficient"       : 0.9,
            "refinement_bound"                    : 2.0,
            "reference_norm_name"                 : "VELOCITY_H1_SEMINORM"
        },
        "maximum_strategy":
        {
            "target_refinement_coefficient"       : 0.1,
            "refinement_coefficient"              : 2.0
        },
        "global_tolerance_strategy":
        {
            "global_tolerance"                 : 0.1
        },
        "echo_level"                          : 0
    })"
    );

    ThisParameters.RecursivelyValidateAndAssignDefaults(default_parameters);
    mMinSize = ThisParameters["minimal_size"].GetDouble();
    mMaxSize = ThisParameters["maximal_size"].GetDouble();
    mEchoLevel = ThisParameters["echo_level"].GetInt();
    mReferenceVariable = ThisParameters["reference_variable_name"].GetString();
    if (ThisParameters["refinement_strategy"].GetString() == "maximum_strategy") mRefinementStrategy = MaximumStrategy;
    else if (ThisParameters["refinement_strategy"].GetString() == "mean_distribution_strategy") mRefinementStrategy = MeanDistributionStrategy;
    else if (ThisParameters["refinement_strategy"].GetString() == "global_tolerance_strategy") mRefinementStrategy = GlobalToleranceStrategy;

    // Mean distribution strategy
    mMeanStrategyReferenceNorm = ThisParameters["mean_distribution_strategy"]["reference_norm_name"].GetString();
    mMeanStrategyTargetRefinementCoefficient = ThisParameters["mean_distribution_strategy"]["target_refinement_coefficient"].GetDouble();
    mMeanStrategyRefinementBound = ThisParameters["mean_distribution_strategy"]["refinement_bound"].GetDouble();
    mMeanStrategyDivergenceFreeOverAllDomain = 0;

    // Maximum strategy
    mMaxStrategyTargetRefinementCoefficient = ThisParameters["maximum_strategy"]["target_refinement_coefficient"].GetDouble();
    mMaxStrategyRefinementCoefficient = ThisParameters["maximum_strategy"]["refinement_coefficient"].GetDouble();
    mDivergenceFreeMaxValue = -1;

    // Global tolerance strategy
    mGlobalErrorStrategyGlobalTolerance = ThisParameters["global_tolerance_strategy"]["global_tolerance"].GetDouble();
    mGlobalErrorStrategyMeshConstant = 1.0;

}

/***********************************************************************************/
/***********************************************************************************/

template<SizeType TDim>
void MetricDivergenceFreeProcess<TDim>::Execute()
{
    // 1) Initialize metric

    // Tensor variable definition
    KRATOS_ERROR_IF_NOT(KratosComponents<Variable<TensorArrayType>>::Has("METRIC_TENSOR_"+std::to_string(TDim)+"D")) << "Import Meshing Application" << std::endl;
    const Variable<TensorArrayType>& tensor_variable = KratosComponents<Variable<TensorArrayType>>::Get("METRIC_TENSOR_"+std::to_string(TDim)+"D");

    NodesArrayType& nodes_array = mrThisModelPart.Nodes();
    KRATOS_DEBUG_ERROR_IF(nodes_array.size() == 0) <<  "ERROR:: Empty list of nodes" << std::endl;
    ElementsArrayType& elements_array = mrThisModelPart.Elements();
    KRATOS_DEBUG_ERROR_IF(elements_array.size() == 0) <<  "ERROR:: Empty list of elements" << std::endl;

    if (nodes_array.begin()->Has(tensor_variable) == false) {
        const TensorArrayType zero_array = ZeroVector(3 * (TDim - 1));
        // Iteration over the nodes
        #pragma omp parallel for
        for(int i = 0; i < static_cast<int>(nodes_array.size()); ++i)
            (nodes_array.begin() + i)->SetValue(tensor_variable, zero_array);
    }

    // 2) Initialize what is needed for each refinement strategy
    InitializeRefinementStrategy();

    // 3) Calculate metric
    CalculateMetric();
}

/***********************************************************************************/
/***********************************************************************************/

template<SizeType TDim>
void MetricDivergenceFreeProcess<TDim>::InitializeRefinementStrategy()
{
    // Prepare arrays
    NodesArrayType& nodes_array = mrThisModelPart.Nodes();
    ElementsArrayType& elements_array = mrThisModelPart.Elements();

    // Maximum strategy
    if (mRefinementStrategy == MaximumStrategy) {
        // Iteration over all elements to find maximum value of reference variable
        const int number_elements = static_cast<int>(elements_array.size());
        mDivergenceFreeMaxValue = -1;
        const auto& r_reference_var = KratosComponents<Variable<double>>::Get(mReferenceVariable);
        for(int i_elem = 0; i_elem < number_elements; ++i_elem) {
            auto it_elem = elements_array.begin() + i_elem;
            const double divergencefree_elem_value = abs(it_elem->GetValue(r_reference_var));
            if (divergencefree_elem_value > mDivergenceFreeMaxValue) {
                mDivergenceFreeMaxValue = divergencefree_elem_value;
            }
        }
    }

    // Mean distribution strategy
    else if (mRefinementStrategy == MeanDistributionStrategy) {

        // Reference variable
        const auto& r_reference_var = KratosComponents<Variable<double>>::Get(mReferenceVariable);

        // Loop over elements for computing divergence value over the whole domain
        const int number_elements = static_cast<int>(elements_array.size());
        for(int i_elem = 0; i_elem < number_elements; ++i_elem) {
            auto it_elem = elements_array.begin() + i_elem;
            mMeanStrategyDivergenceFreeOverAllDomain += std::pow(it_elem->GetValue(r_reference_var),2);
        }
        mMeanStrategyDivergenceFreeOverAllDomain = std::sqrt(mMeanStrategyDivergenceFreeOverAllDomain);
    }
}

/***********************************************************************************/
/***********************************************************************************/

template<SizeType TDim>
void MetricDivergenceFreeProcess<TDim>::CalculateMetric()
{
    // Prepare arrays
    NodesArrayType& nodes_array = mrThisModelPart.Nodes();
    ElementsArrayType& elements_array = mrThisModelPart.Elements();

    // Tensor variable definition
    const Variable<TensorArrayType>& tensor_variable = KratosComponents<Variable<TensorArrayType>>::Get("METRIC_TENSOR_"+std::to_string(TDim)+"D");

    // Prepare iterations over all nodes and elements
    const int number_nodes = static_cast<int>(nodes_array.size());
    const int number_elements = static_cast<int>(elements_array.size());
    KRATOS_DEBUG_ERROR_IF(number_nodes == 0) <<  "ERROR:: Empty list of nodes" << std::endl;

    const auto& r_reference_var  = KratosComponents<Variable<double>>::Get(mReferenceVariable);

    //  Compute nodal divergence loops
    Vector divergence_nodal_values(number_nodes);
    Vector area_nodal_values(number_nodes);
    // Initialize to zero
    for (int i_node = 0; i_node < number_nodes; ++i_node) {
        divergence_nodal_values[i_node] = 0;
        area_nodal_values[i_node] = 0;
    }
    // Weighted mean
    for (int i_elem = 0; i_elem < number_elements; ++i_elem) {
        auto it_elem = elements_array.begin() + i_elem;
        auto r_geometry = it_elem->GetGeometry();
        for (unsigned int i_node = 0; i_node < r_geometry.size(); ++i_node) {
            area_nodal_values[r_geometry[i_node].Id()-1] = area_nodal_values[r_geometry[i_node].Id()-1] + it_elem->GetGeometry().Area() / r_geometry.size();
            divergence_nodal_values[r_geometry[i_node].Id()-1] = divergence_nodal_values[r_geometry[i_node].Id()-1] + it_elem->GetValue(r_reference_var) * it_elem->GetGeometry().Area() / r_geometry.size();
        }
    }
    // Delete area contribution
    for (int i_node = 0; i_node < number_nodes; ++i_node) {
        divergence_nodal_values[i_node] = divergence_nodal_values[i_node] / area_nodal_values[i_node];
    }

    // Maximum strategy
    if (mRefinementStrategy == MaximumStrategy) {

        #pragma omp parallel for
        for(int i_node = 0; i_node < number_nodes; ++i_node) {
            auto it_node = nodes_array.begin() + i_node;

            // Retrieve variable
            const double divergencefree_elem_value = divergence_nodal_values[i_node];

            // Estimate element size
            const double nodal_h = it_node->GetValue(NODAL_H);

            // Check for zero values
            if (mMaxStrategyRefinementCoefficient < std::numeric_limits<double>::epsilon())
            {
                KRATOS_WARNING("maximum_strategy") << "WARNING: Your variable refinement coefficient is near zero: " << mMaxStrategyRefinementCoefficient << std::endl;
                KRATOS_WARNING("maximum_strategy") << "WARNING: Setting refinement coefficient to lower numeric limit: " << std::numeric_limits<double>::epsilon() << std::endl;
                mMaxStrategyRefinementCoefficient = std::numeric_limits<double>::epsilon();
            }

            // Set element size
            double element_size;
            if (divergencefree_elem_value >= mMaxStrategyTargetRefinementCoefficient*mDivergenceFreeMaxValue) {
                element_size = nodal_h/mMaxStrategyRefinementCoefficient;
            }
            else {
                element_size = nodal_h;
            }

            // Check with max and min size
            if (element_size < mMinSize) element_size = mMinSize;
            if (element_size > mMaxSize) element_size = mMaxSize;

            // Set metric
            BoundedMatrix<double, TDim, TDim> metric_matrix = ZeroMatrix(TDim, TDim);
            for(IndexType i = 0;i < TDim; ++i)
                metric_matrix(i,i) = 1.0/std::pow(element_size, 2);

            // Transform metric matrix to a vector
            const TensorArrayType metric = MathUtils<double>::StressTensorToVector<MatrixType, TensorArrayType>(metric_matrix);

            // Setting value
            it_node->SetValue(tensor_variable, metric);
            KRATOS_INFO_IF("MetricDivergenceFreeProcess", mEchoLevel > 2) << "Node " << it_node->Id() << " has metric: "<< metric << std::endl;

        }
    }

    // Mean distribution strategy
    else if (mRefinementStrategy == MeanDistributionStrategy) {

        // Reference norm
        const auto& r_reference_norm = KratosComponents<Variable<double>>::Get(mMeanStrategyReferenceNorm);

        #pragma omp parallel for
        for(int i_node = 0; i_node < number_nodes; ++i_node) {
            auto it_node = nodes_array.begin() + i_node;
            // Retrieve variable
            double divergencefree_interp_value = divergence_nodal_values[i_node];
            // Estimate element size
            const double nodal_h = it_node->GetValue(NODAL_H);
            // Set element size
            double element_size;

            // Check for zero value
            if (mMeanStrategyRefinementBound < std::numeric_limits<double>::epsilon())
            {
                KRATOS_WARNING("mean_distribution_strategy") << "WARNING: Your variable refinement_bound is near zero: " << mMeanStrategyRefinementBound << std::endl;
                KRATOS_WARNING("mean_distribution_strategy") << "WARNING: Setting refinement_bound to lower numeric limit: " << std::numeric_limits<double>::epsilon() << std::endl;
                mMeanStrategyRefinementBound = std::numeric_limits<double>::epsilon();
            }

            // Compute factor to estimate element size
            double factor;
            if (divergencefree_interp_value < std::numeric_limits<double>::epsilon())
            {
                KRATOS_WARNING("mean_distribution_strategy") << "WARNING: Your variable interpolation nodal value is near zero: " << divergencefree_interp_value << std::endl;
                factor = mMeanStrategyRefinementBound;
            }
            else
            {
                // Compute factor to estimate element size
                factor = mMeanStrategyTargetRefinementCoefficient*mMeanStrategyDivergenceFreeOverAllDomain/std::sqrt(number_elements)/divergencefree_interp_value;
            }

            // Bound factor coefficient
            if (factor < 1.0/mMeanStrategyRefinementBound) factor = 1.0/mMeanStrategyRefinementBound;
            if (factor > mMeanStrategyRefinementBound) factor = mMeanStrategyRefinementBound;

            // Compute element size
            element_size = factor*nodal_h;

            // Check with max and min size
            if (element_size < mMinSize) element_size = mMinSize;
            if (element_size > mMaxSize) element_size = mMaxSize;

            // Set metric
            BoundedMatrix<double, TDim, TDim> metric_matrix = ZeroMatrix(TDim, TDim);
            for(IndexType i = 0;i < TDim; ++i)
                metric_matrix(i,i) = 1.0/std::pow(element_size, 2);

            // Transform metric matrix to a vector
            const TensorArrayType metric = MathUtils<double>::StressTensorToVector<MatrixType, TensorArrayType>(metric_matrix);

            // Setting value
            it_node->SetValue(tensor_variable, metric);
            KRATOS_INFO_IF("MetricDivergenceFreeProcess", mEchoLevel > 2) << "Node " << it_node->Id() << " has metric: "<< metric << std::endl;

        }
    }

    // Global tolerance strategy
    else if (mRefinementStrategy == GlobalToleranceStrategy) {

        const double min_ratio = 1.0/std::pow(mMinSize, 2);
        const double max_ratio = 1.0/std::pow(mMaxSize, 2);

        // Check if the global tolerance is near zero. If it is we will correct it
        if (mGlobalErrorStrategyGlobalTolerance < std::numeric_limits<double>::epsilon())
        {
            KRATOS_WARNING("global_tolerance_strategy") << "WARNING: Your global tolerance is near zero: " << mGlobalErrorStrategyGlobalTolerance << std::endl;
            KRATOS_WARNING("global_tolerance_strategy") << "WARNING: Setting global tolerance to lower numeric limit: " << std::numeric_limits<double>::epsilon() << std::endl;
            mGlobalErrorStrategyGlobalTolerance = std::numeric_limits<double>::epsilon();
        }

        #pragma omp parallel for
        for(int i_node = 0; i_node < number_nodes; ++i_node) {
            auto it_node = nodes_array.begin() + i_node;

            // Initialize Divergence
            double divergencefree_interp_value = divergence_nodal_values[i_node];

            // Set element size
            double aux_element_size;
            double factor = mGlobalErrorStrategyMeshConstant*std::abs(divergencefree_interp_value)/mGlobalErrorStrategyGlobalTolerance;

            // Check with max and min size
            aux_element_size = MathUtils<double>::Min(MathUtils<double>::Max(factor, max_ratio), min_ratio);

            // Set metric
            BoundedMatrix<double, TDim, TDim> metric_matrix = ZeroMatrix(TDim, TDim);
            for(IndexType i = 0;i < TDim; ++i)
                metric_matrix(i,i) = aux_element_size;

            // Transform metric matrix to a vector
            const TensorArrayType metric = MathUtils<double>::StressTensorToVector<MatrixType, TensorArrayType>(metric_matrix);

            // Setting value
            it_node->SetValue(tensor_variable, metric);
            KRATOS_INFO_IF("MetricDivergenceFreeProcess", mEchoLevel > 2) << "Node " << it_node->Id() << " has metric: "<< metric << std::endl;

        }
    }

}

/***********************************************************************************/
/***********************************************************************************/

template class MetricDivergenceFreeProcess<2>;
template class MetricDivergenceFreeProcess<3>;

};// namespace Kratos.
