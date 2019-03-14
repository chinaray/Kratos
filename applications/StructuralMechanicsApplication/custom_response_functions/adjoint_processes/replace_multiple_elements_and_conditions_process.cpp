// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Armin Geiser
//

// System includes

// External includes

// Project includes
#include "replace_multiple_elements_and_conditions_process.h"
#include "utilities/compare_elements_and_conditions_utility.h"

namespace Kratos
{

namespace {

template <class TEntityContainer>
void ReplaceEntities(TEntityContainer& rEntityContainer,
                     Parameters EntitySettings,
                     bool ThrowError)
{
    typedef typename TEntityContainer::data_type EntityType;

    // create map with reference entities
    std::map<std::string, const EntityType*> entities_table;
    for(Parameters::iterator it=EntitySettings.begin(); it!=EntitySettings.end(); ++it){
        entities_table[it.name()] = &KratosComponents<EntityType>::Get(
            EntitySettings[it.name()].GetString()
        );
    }

    #pragma omp parallel for
    for(int i=0; i< static_cast<int>(rEntityContainer.size()); i++) {
        auto it = rEntityContainer.begin() + i;

        std::string current_name;
        CompareElementsAndConditionsUtility::GetRegisteredName(*it, current_name);

        auto it_reference_entity = entities_table.find(current_name);

        if (it_reference_entity == entities_table.end()) {
            // This error is thrown in a parallel region and can not get catched
            // or even printed properly!
            KRATOS_ERROR_IF(ThrowError) << current_name
                << " was not defined in the replacement table!" << std::endl;
            // skip if no error should be thrown
            continue;
        }

        auto p_entitiy = it_reference_entity->second->Create(it->Id(),
            it->pGetGeometry(),
            it->pGetProperties()
        );

        // Deep copy data and flags
        p_entitiy->Data() = it->Data();
        p_entitiy->Set(Flags(*it));

        (*it.base()) = p_entitiy;
    }
}

} // namespace


void ReplaceMultipleElementsAndConditionsProcess::Execute()
{
    ModelPart& r_root_model_part = mrModelPart.GetRootModelPart();

    bool throw_error = mSettings["throw_error"].GetBool();

    // replace elements
    ReplaceEntities(mrModelPart.Elements(), mSettings["element_name_table"], throw_error);

    // replace conditions
    ReplaceEntities(mrModelPart.Conditions(), mSettings["condition_name_table"], throw_error);

    // Change the submodelparts
    for (auto& i_sub_model_part : r_root_model_part.SubModelParts()) {
        UpdateSubModelPart( i_sub_model_part, r_root_model_part );
    }
}

void ReplaceMultipleElementsAndConditionsProcess::UpdateSubModelPart(
    ModelPart& rModelPart,
    ModelPart& rRootModelPart
    )
{
    // Change the model part itself
    #pragma omp parallel for
    for(int i=0; i< static_cast<int>(rModelPart.Elements().size()); i++) {
        auto it_elem = rModelPart.ElementsBegin() + i;

        (*it_elem.base()) = rRootModelPart.Elements()(it_elem->Id());
    }

    #pragma omp parallel for
    for(int i=0; i< static_cast<int>(rModelPart.Conditions().size()); i++) {
        auto it_cond = rModelPart.ConditionsBegin() + i;

        (*it_cond.base()) = rRootModelPart.Conditions()(it_cond->Id());
    }

    // Change the submodelparts
    for (auto& i_sub_model_part : rModelPart.SubModelParts()) {
        UpdateSubModelPart( i_sub_model_part, rRootModelPart );
    }
}

}  // namespace Kratos.



