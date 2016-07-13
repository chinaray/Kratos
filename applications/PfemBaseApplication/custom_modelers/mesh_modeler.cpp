//
//   Project Name:        KratosPfemBaseApplication $
//   Created by:          $Author:      JMCarbonell $
//   Last modified by:    $Co-Author:               $
//   Date:                $Date:      February 2016 $
//   Revision:            $Revision:            0.0 $
//
//

// System includes

// External includes

// Project includes
#include "custom_modelers/laplacian_smoothing.hpp"
#include "custom_modelers/mesh_modeler.hpp"

#include "pfem_base_application_variables.h"

namespace Kratos
{

  //*******************************************************************************************
  //*******************************************************************************************
  void MeshModeler::StartEcho(ModelPart& rModelPart,
			      std::string GenerationMessage, 
			      ModelPart::IndexType MeshId)
  {

    if( this->GetEchoLevel() > 0 ){
      std::cout<<" [ [ [ ] ] ]"<<std::endl;
      std::cout<<" [ "<<GenerationMessage <<" ]"<<std::endl;
      std::cout<<" [ PREVIOUS MESH (Elements: "<<rModelPart.NumberOfElements(MeshId)<<" Nodes: "<<rModelPart.NumberOfNodes(MeshId)<<" Conditions: "<<rModelPart.NumberOfConditions(MeshId)<<") ] MESH_ID: ["<<MeshId<<"]"<<std::endl;
    }
  }

  //*******************************************************************************************
  //*******************************************************************************************
  void MeshModeler::EndEcho(ModelPart& rModelPart,
			    std::string GenerationMessage, 
			    ModelPart::IndexType MeshId)
  {

    if( this->GetEchoLevel() > 0 ){
      std::cout<<" [ NEW MESH (Elements: "<<rModelPart.Elements(MeshId).size()<<" Nodes: "<<rModelPart.Nodes(MeshId).size()<<" Conditions: "<<rModelPart.Conditions(MeshId).size()<<") ]  MESH_ID: ["<<MeshId<<"]"<<std::endl;
      std::cout<<" [ "<<GenerationMessage <<" ]"<<std::endl;
      std::cout<<" [ Finished Remeshing ] "<<std::endl;
      std::cout<<" [ [ [ ] ] ]"<<std::endl;
    }

  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::Initialize()
  {
    KRATOS_TRY
    
    KRATOS_CATCH(" ")
  }
  
  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::InitializeMeshGeneration(ModelPart& rModelPart,
					     MeshingParametersType& rMeshingVariables,
					     ModelPart::IndexType MeshId)
  {
    KRATOS_TRY
    
    //Refine and Remove nodes processes
    ////////////////////////////////////////////////////////////
    if( mPreMeshingProcesses.size() )
      for(unsigned int i=0; i<mPreMeshingProcesses.size(); i++)
	mPreMeshingProcesses[i]->Execute();
    ////////////////////////////////////////////////////////////

    KRATOS_CATCH( "" )
  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::InitializeMeshRefinement(ModelPart& rModelPart,
					     MeshingParametersType& rMeshingVariables,
					     ModelPart::IndexType MeshId)
  {
    KRATOS_TRY
    
    //Refine and Remove nodes processes
    ////////////////////////////////////////////////////////////
    if( mPreRefiningProcesses.size() )
      for(unsigned int i=0; i<mPreRefiningProcesses.size(); i++)
	mPreRefiningProcesses[i]->Execute();
    ////////////////////////////////////////////////////////////

    KRATOS_CATCH( "" )
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::FinalizeMeshRefinement(ModelPart& rModelPart,
					   MeshingParametersType& rMeshingVariables,
					   ModelPart::IndexType MeshId)
  {
    KRATOS_TRY
    
    //Refine and Remove nodes processes
    ////////////////////////////////////////////////////////////
    if( mPostRefiningProcesses.size() )
      for(unsigned int i=0; i<mPostRefiningProcesses.size(); i++)
	mPostRefiningProcesses[i]->Execute();
    ////////////////////////////////////////////////////////////

    KRATOS_CATCH( "" )
  }



  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::FinalizeMeshGeneration(ModelPart& rModelPart,
					   MeshingParametersType& rMeshingVariables,
					   ModelPart::IndexType MeshId)
  {
    KRATOS_TRY

    //Rebuild Boundary processes
    ////////////////////////////////////////////////////////////
    if( mPostMeshingProcesses.size() )
      for(unsigned int i=0; i<mPostMeshingProcesses.size(); i++)
	mPostMeshingProcesses[i]->Execute();
    ////////////////////////////////////////////////////////////

 
    KRATOS_CATCH( "" )

  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetMeshingParameters( MeshingParametersType::Pointer& rMeshingParameters, ModelPart::IndexType MeshId  )
  {
    KRATOS_TRY
    
    // if(mMeshingVariables.size() == 1 && MeshId != 0)
    //   std::cout<<" Something wrong with mesh ID "<<MeshId<<std::endl;

    mpMeshingVariables = rMeshingParameters;

    bool Remesh = false;
    if( mpMeshingVariables->Options.Is(ModelerUtilities::REMESH) )
      Remesh = true;

    bool Refine = false;
    if( mpMeshingVariables->Options.Is(ModelerUtilities::REFINE) )
      Refine = true;

    bool Transfer = false;
    if( mpMeshingVariables->Options.Is(ModelerUtilities::TRANSFER) )
      Transfer = true;

    if( mEchoLevel > 0 )
      std::cout<<" SetRemeshData ["<<MeshId<<"]: [ RefineFlag: "<<Refine<<" RemeshFlag : "<<Remesh<<" TransferFlag : "<<Transfer<<" ] "<<std::endl;

    KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPreMeshingProcess( Process::Pointer pPreMeshingProcess )
  {
     KRATOS_TRY
       
     mPreMeshingProcesses.push_back(pPreMeshingProcess); //NOTE: order set = order of execution
       
     KRATOS_CATCH(" ")
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPostMeshingProcess( Process::Pointer pPostMeshingProcess )
  {
     KRATOS_TRY
       
     mPostMeshingProcesses.push_back(pPostMeshingProcess); //NOTE: order set = order of execution
       
     KRATOS_CATCH(" ")
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPreRefiningProcess( Process::Pointer pPreRefiningProcess )
  {
     KRATOS_TRY
       
     mPreRefiningProcesses.push_back(pPreRefiningProcess); //NOTE: order set = order of execution
       
     KRATOS_CATCH(" ")
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPostRefiningProcess( Process::Pointer pPostRefiningProcess )
  {
     KRATOS_TRY
       
     mPostRefiningProcesses.push_back(pPostRefiningProcess); //NOTE: order set = order of execution
       
     KRATOS_CATCH(" ")
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPreMeshingProcessVector( std::vector<Process::Pointer>& rPreMeshingProcessVector )
  {
     KRATOS_TRY
       
     mPreMeshingProcesses = rPreMeshingProcessVector; 
       
     KRATOS_CATCH(" ")
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPostMeshingProcessVector( std::vector<Process::Pointer>& rPostMeshingProcessVector )
  {
     KRATOS_TRY
       
     mPostMeshingProcesses = rPostMeshingProcessVector; 
       
     KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPreRefiningProcessVector( std::vector<Process::Pointer>& rPreRefiningProcessVector )
  {
     KRATOS_TRY
       
     mPreRefiningProcesses = rPreRefiningProcessVector; 
       
     KRATOS_CATCH(" ")
  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetPostRefiningProcessVector( std::vector<Process::Pointer>& rPostRefiningProcessVector )
  {
     KRATOS_TRY
       
     mPostRefiningProcesses = rPostRefiningProcessVector; 
       
     KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetModelerUtilities(ModelerUtilities::Pointer rModelerUtilities )
  {
    KRATOS_TRY

    mpModelerUtilities = rModelerUtilities;

    KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetDataTransferUtilities(MeshDataTransferUtilities::Pointer rDataTransferUtilities )
  {
    KRATOS_TRY

    mpDataTransferUtilities = rDataTransferUtilities;

    KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************


  void MeshModeler::InitializeMeshModeler( ModelPart& rModelPart, ModelPart::IndexType MeshId )
  {
    KRATOS_TRY


    KRATOS_CATCH(" ")
  }



  //*******************************************************************************************
  //*******************************************************************************************


  void MeshModeler::FinalizeMeshModeler( ModelPart& rModelPart, ModelPart::IndexType MeshId )
  {
    KRATOS_TRY



    KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************


  void MeshModeler::GenerateMesh ( ModelPart& rModelPart, ModelPart::IndexType MeshId )
  {
    KRATOS_TRY


    if( GetEchoLevel() > 0 ){
      std::cout<<" [ GetRemeshData ["<<MeshId<<"]: [ RefineFlag: "<<mpMeshingVariables->Options.Is(ModelerUtilities::REFINE)<<"; RemeshFlag : "<<mpMeshingVariables->Options.Is(ModelerUtilities::REMESH)<<" ] ]"<<std::endl;
    }
 
    // bool out_buffer_active = true;
    // std::streambuf* buffer = NULL;
    // if( mEchoLevel == 0 ){
    //   //std::cout<<" Deactivate cout "<<std::endl;
    //   buffer = std::cout.rdbuf();
    //   std::ofstream fout("/dev/null");
    //   std::cout.rdbuf(fout.rdbuf());
    //   //std::cout<<output(off,buffer);
    //   out_buffer_active = false;
    // }

    if(mpMeshingVariables->Options.Is(ModelerUtilities::REMESH))
      rModelPart.GetMesh(MeshId).Set( ModelerUtilities::REMESH );
    else
      rModelPart.GetMesh(MeshId).Reset( ModelerUtilities::REMESH );
    
    if(mpMeshingVariables->Options.Is(ModelerUtilities::REFINE))
      rModelPart.GetMesh(MeshId).Set( ModelerUtilities::REFINE );
    
    if(mpMeshingVariables->Options.Is(ModelerUtilities::CONSTRAINED))
      rModelPart.GetMesh(MeshId).Set( ModelerUtilities::CONSTRAINED );

    if(mpMeshingVariables->Options.Is(ModelerUtilities::TRANSFER))
      rModelPart.GetMesh(MeshId).Set( ModelerUtilities::TRANSFER );
	  
	
    // check mesh size introduced :: warning must be shown
    // if(!out_buffer_active)
    //   std::cout.rdbuf(buffer);
        
    if(mpMeshingVariables->Options.Is( ModelerUtilities::REFINE ))
      mpModelerUtilities->CheckCriticalRadius(rModelPart, mpMeshingVariables->Refine->CriticalRadius, MeshId);
		
    // if(!out_buffer_active){
    //   buffer = std::cout.rdbuf();
    //   std::ofstream fout("/dev/null");
    //   std::cout.rdbuf(fout.rdbuf());
    // }
    // check mesh size introduced :: warning must be shown
    


    LaplacianSmoothing   MeshGeometricSmoothing(rModelPart);
    MeshGeometricSmoothing.SetEchoLevel(GetEchoLevel());
	
    //bool remesh_performed=false;
	
    if( GetEchoLevel() > 0 ){
      std::cout<<" --------------                     -------------- "<<std::endl;
      std::cout<<" --------------       DOMAIN        -------------- "<<std::endl;
    }
	
    ModelPart::MeshesContainerType Meshes = rModelPart.GetMeshes(); 	  
 
    //initialize info
    mpMeshingVariables->Info->Initialize();
    
    mpMeshingVariables->Info->NumberOfElements   = rModelPart.NumberOfElements(MeshId);
    mpMeshingVariables->Info->NumberOfNodes      = rModelPart.NumberOfNodes(MeshId);
    mpMeshingVariables->Info->NumberOfConditions = rModelPart.NumberOfConditions(MeshId);
    
    //set properties in all meshes
    if(rModelPart.NumberOfProperties(MeshId)!=rModelPart.NumberOfProperties())
      rModelPart.GetMesh(MeshId).SetProperties(rModelPart.GetMesh().pProperties());
    
    // VARIABLES SMOOTHING, transfer to nodes
    if(mpMeshingVariables->Options.Is(ModelerUtilities::VARIABLES_SMOOTHING)){
      if( mpMeshingVariables->TransferVariablesSetFlag ){
	mpDataTransferUtilities->TransferElementalValuesToNodes(*(mpMeshingVariables->Transfer),rModelPart,MeshId);
      }
    }
    
    if(Meshes[MeshId].Is( ModelerUtilities::REMESH )){
	  

      if(Meshes[MeshId].Is( ModelerUtilities::CONSTRAINED ))
	{
	  if(Meshes[MeshId].Is( ModelerUtilities::REFINE )){ //Remesh Constrained and Refine
	    //Constrained Delaunay Triangulation
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REFINE RCDT START]:"<<std::endl;
	    this->GenerateRCDT(rModelPart,*(mpMeshingVariables),MeshId);	
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REFINE RCDT END]"<<std::endl;	  
	  }
	  else{ 	                                     //Remesh Constrained
	    //Generate Constrained Delaunay Triangulation
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REMESH CDT ]:"<<std::endl;
	    this->GenerateCDT(rModelPart,*(mpMeshingVariables),MeshId);		
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REMESH END ]"<<std::endl;    		    
	  }
	}
      else
	{
	  if(Meshes[MeshId].Is( ModelerUtilities::REFINE )){ //Remesh Non-Constrained and Refine
	    //Constrained Delaunay Triangulation
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REFINE RDT START]:"<<std::endl;
	    this->GenerateRDT(rModelPart,*(mpMeshingVariables),MeshId);	
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REFINE RDT END]"<<std::endl;	  
	  }
	  else{                                              //Remesh Non-Constrained
	    //Generate Delaunay Triangulation
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REMESH DT START ]:"<<std::endl;
	    this->GenerateDT(rModelPart,*(mpMeshingVariables),MeshId);
	    if( GetEchoLevel() > 0 )
	      std::cout<<" [ MESH: "<<MeshId<<" REMESH DT END ]"<<std::endl;
	  }
	}
      
      if( GetEchoLevel() > 0 ){
	std::cout<<" --------------                     -------------- "<<std::endl;
	std::cout<<" --------------  REMESH PERFORMED   -------------- "<<std::endl;
	std::cout<<" --------------                     -------------- "<<std::endl;
      }
      
      //remesh_performed=true;
    }
    else if(mpMeshingVariables->Options.Is(ModelerUtilities::TRANSFER)){
      
      if( GetEchoLevel() > 0 )
	std::cout<<" [ MESH: "<<MeshId<<" TRANSFER START ]:"<<std::endl;
		
      this->PerformTransferOnly(rModelPart,*(mpMeshingVariables),MeshId);
	
      //remesh_performed=true;
	
      if( GetEchoLevel() > 0 )
	std::cout<<" [ MESH: "<<MeshId<<" TRANSFER END ]"<<std::endl;

    }
    else if(mpMeshingVariables->Options.Is(ModelerUtilities::MESH_SMOOTHING)){
	
      if( GetEchoLevel() > 0 )
	std::cout<<" [ MESH: "<<MeshId<<" TRANSFER START ]:"<<std::endl;
	
      //transfer only is done if the remesh option is active
      rModelPart.GetMesh(MeshId).Set( ModelerUtilities::REMESH );  
      //and if there is a minimum of inserted or removed nodes
      mpMeshingVariables->Info->InsertedNodes = mpMeshingVariables->Info->NumberOfNewNodes;
	
      this->PerformTransferOnly(rModelPart,*(mpMeshingVariables),MeshId);
	
      //remesh_performed=true;
	
      if( GetEchoLevel() > 0 )
	std::cout<<" [ MESH: "<<MeshId<<" TRANSFER END ]"<<std::endl;
    }
    else{
      if( GetEchoLevel() > 0 )
	std::cout<<" [ MESH: "<<MeshId<<" NO REMESH ]"<<std::endl;
    }
      
    // VARIABLES SMOOTHING, transfer to elements
    if(mpMeshingVariables->Options.Is(ModelerUtilities::VARIABLES_SMOOTHING)){

      // Geometrical smoothing is done when new elements are build:
      
      //Check Mesh Info to perform smoothing:
      //mpMeshingVariables->Info->CheckGeometricalSmoothing();
      //if( mOptions.Is(ModelerUtilities::MESH_SMOOTHING) && mpMeshingVariables->Info->GeometricalSmoothingRequired ){
      //MeshGeometricSmoothing.ApplyMeshSmoothing(mrModelPart,LaplacianSmoothing::SMOOTH_ALL,MeshId);
      //}
		  
      //Check Mesh Info to perform smoothing:
      mpMeshingVariables->Info->CheckMechanicalSmoothing();

      if( mpMeshingVariables->Options.Is(ModelerUtilities::VARIABLES_SMOOTHING) && mpMeshingVariables->Info->MechanicalSmoothingRequired ){

	//recover DETERMINANT_F FOR VARIABLES SMOOTHING from nodes
	if( mpMeshingVariables->Info->CriticalElements > 0 ){

	  if( mpMeshingVariables->Transfer->VariablesSetFlag ){
	    //Smoothing performed only in critical elements (usually based on PLASTIC_STRAIN variable)
	    mpDataTransferUtilities->TransferNodalValuesToElements(*(mpMeshingVariables->Transfer),mpMeshingVariables->Refine->GetThresholdVariable(),mpMeshingVariables->Refine->ReferenceThreshold,rModelPart,MeshId);
	    
	    //std::cout<<" Transfer on critical elements "<<*mpMeshingVariables->Transfer->DoubleVariables[0]<<" "<<mpMeshingVariables->Refine->GetThresholdVariable()<<std::endl;
		   
	    //Smoothing performed only in critical elements (based on Set Dissipation Variable)
	    //mpDataTransferUtilities->TransferNodalValuesToElements(DETERMINANT_F,mpMeshingVariables->Refine->GetThresholdVariable(),mpMeshingVariables->Refine->ReferenceThreshold,rModelPart,MeshId);

	    //Smoothing performed only in critical elements (based on Plastic Energy Dissipation)
	    //mpDataTransferUtilities->TransferNodalValuesToElements(DETERMINANT_F,PLASTIC_DISSIPATION,mpMeshingVariables->Refine->ReferenceThreshold,rModelPart,MeshId);
								  
	  }

	}
	else{

	  //Smoothing performed to all mesh
	  if( mpMeshingVariables->Transfer->VariablesSetFlag )
	    mpDataTransferUtilities->TransferNodalValuesToElements(*(mpMeshingVariables->Transfer),rModelPart,MeshId);
	      
	}

      }

    }


    //finalize info
    mpMeshingVariables->Info->NumberOfNewElements   = rModelPart.NumberOfElements(MeshId) - mpMeshingVariables->Info->NumberOfElements;
    mpMeshingVariables->Info->NumberOfNewNodes      = rModelPart.NumberOfNodes(MeshId) - mpMeshingVariables->Info->NumberOfNodes;
    mpMeshingVariables->Info->NumberOfNewConditions = rModelPart.NumberOfConditions(MeshId) - mpMeshingVariables->Info->NumberOfConditions;
    

    KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************


  void MeshModeler::ExecuteMeshing ( ModelPart& rModelPart, ModelPart::IndexType MeshId )
  {
    KRATOS_TRY


    if( GetEchoLevel() > 0 ){
      std::cout<<" [ GetRemeshData ["<<MeshId<<"]: [ RefineFlag: "<<mpMeshingVariables->Options.Is(ModelerUtilities::REFINE)<<"; RemeshFlag : "<<mpMeshingVariables->Options.Is(ModelerUtilities::REMESH)<<" ] ]"<<std::endl;
    }

    // bool out_buffer_active = true;
    // std::streambuf* buffer = NULL;
    // if( mEchoLevel == 0 ){
    //   //std::cout<<" Deactivate cout "<<std::endl;
    //   buffer = std::cout.rdbuf();
    //   std::ofstream fout("/dev/null");
    //   std::cout.rdbuf(fout.rdbuf());
    //   //std::cout<<output(off,buffer);
    //   out_buffer_active = false;
    // } 
	
    // Located in the begining of the assignation:

    // check mesh size introduced :: warning must be shown
    // if(!out_buffer_active)
    //   std::cout.rdbuf(buffer);
        
    if(mpMeshingVariables->Options.Is( ModelerUtilities::REFINE ))
      mpModelerUtilities->CheckCriticalRadius(rModelPart, mpMeshingVariables->Refine->CriticalRadius, MeshId);
		
    // if(!out_buffer_active){
    //   buffer = std::cout.rdbuf();
    //   std::ofstream fout("/dev/null");
    //   std::cout.rdbuf(fout.rdbuf());
    // }
    // check mesh size introduced :: warning must be shown
    
	
    //bool remesh_performed=false;
	
    if( GetEchoLevel() > 0 ){
      std::cout<<" --------------                     -------------- "<<std::endl;
      std::cout<<" --------------       DOMAIN        -------------- "<<std::endl;
    }
	
    // Located in the begining of the assignation:

    //set properties in all meshes
    if(rModelPart.NumberOfProperties(MeshId)!=rModelPart.NumberOfProperties())
      rModelPart.GetMesh(MeshId).SetProperties(rModelPart.GetMesh().pProperties());


    //generate mesh
    this->Generate(rModelPart,*(mpMeshingVariables),MeshId);
   

    KRATOS_CATCH(" ")
  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetNodes(ModelPart& rModelPart,
			     MeshingParametersType& rMeshingVariables,
			     ModelPart::IndexType MeshId)
  {
    KRATOS_TRY
     
    const unsigned int dimension = rModelPart.ElementsBegin(MeshId)->GetGeometry().WorkingSpaceDimension();

    //*********************************************************************
    //input mesh: NODES
    ModelerUtilities::MeshContainer& InMesh = rMeshingVariables.InMesh;
    
    InMesh.CreatePointList(rModelPart.Nodes(MeshId).size(), dimension);

    double* PointList     = InMesh.GetPointList();
    int& NumberOfPoints   = InMesh.GetNumberOfPoints();

    if(!rMeshingVariables.InputInitializedFlag){

      if((int)rMeshingVariables.NodalPreIds.size() != NumberOfPoints)
	rMeshingVariables.NodalPreIds.resize(NumberOfPoints+1);
      
      std::fill( rMeshingVariables.NodalPreIds.begin(), rMeshingVariables.NodalPreIds.end(), 0 );
    }
    
    //writing the points coordinates in a vector and reordening the Id's
    ModelPart::NodesContainerType::iterator nodes_begin = rModelPart.NodesBegin(MeshId);

    int base   = 0;
    int direct = 1;

    for(int i = 0; i<NumberOfPoints; i++)
      {
	//from now on it is consecutive
	if(!rMeshingVariables.InputInitializedFlag){
	  rMeshingVariables.NodalPreIds[direct]=(nodes_begin + i)->Id();
	  (nodes_begin + i)->SetId(direct);
	}

	array_1d<double, 3>& Coordinates = (nodes_begin + i)->Coordinates();

	if(rMeshingVariables.ExecutionOptions.Is(ModelerUtilities::CONSTRAINED)){

	  if( (nodes_begin + i)->Is(BOUNDARY) ){
	       
	    array_1d<double, 3>&  Normal=(nodes_begin + i)->FastGetSolutionStepValue(NORMAL); //BOUNDARY_NORMAL must be set as nodal variable
	    double Shrink = (nodes_begin + i)->FastGetSolutionStepValue(SHRINK_FACTOR);   //SHRINK_FACTOR   must be set as nodal variable
	           
	    array_1d<double, 3> Offset;

	    Normal /= norm_2(Normal);
	    for(unsigned int j=0; j<dimension; j++){
	      Offset[j] = ( (-1) * Normal[j] * Shrink * rMeshingVariables.OffsetFactor * 0.25 );
	    }

	    for(unsigned int j=0; j<dimension; j++){
	      PointList[base+j]   = Coordinates[j] + Offset[j];
	    }
	  }
	  else{
	    for(unsigned int j=0; j<dimension; j++){
	      PointList[base+j]   = Coordinates[j];
	    }
	  }

	}
	else{
	  for(unsigned int j=0; j<dimension; j++){
	    PointList[base+j]   = Coordinates[j];
	  }
	}
	   
	base+=dimension;
	direct+=1;
      }

    //InMesh.SetPointList(PointList);

    KRATOS_CATCH( "" )

  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetElements(ModelPart& rModelPart,
				MeshingParametersType& rMeshingVariables,
				ModelPart::IndexType MeshId)
  {
    KRATOS_TRY
       
          
    //*********************************************************************
    //input mesh: ELEMENTS
    ModelPart::ElementsContainerType::iterator element_begin = rModelPart.ElementsBegin(MeshId);
    const unsigned int nds       = element_begin->GetGeometry().size();

    ModelerUtilities::MeshContainer& InMesh = rMeshingVariables.InMesh;

    InMesh.CreateElementList(rModelPart.Elements(MeshId).size(), nds);
    
    int* ElementList      = InMesh.GetElementList();
    int& NumberOfElements = InMesh.GetNumberOfElements();

 
    int base=0;
    for(unsigned int el = 0; el<(unsigned int)NumberOfElements; el++)
      {
	Geometry<Node<3> >& geom = (element_begin+el)->GetGeometry();
	
	for(unsigned int i=0; i<nds; i++)
	  {
	    ElementList[base+i] = geom[i].Id();
	  }
	base+=nds;
      }

    KRATOS_CATCH( "" )

  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetNeighbours(ModelPart& rModelPart,
				  MeshingParametersType& rMeshingVariables,
				  ModelPart::IndexType MeshId)
  {
    KRATOS_TRY

        
    //*********************************************************************
    //input mesh: NEIGHBOURELEMENTS
    ModelPart::ElementsContainerType::iterator element_begin = rModelPart.ElementsBegin(MeshId);
    const unsigned int nds          = element_begin->GetGeometry().size();

    ModelerUtilities::MeshContainer& InMesh = rMeshingVariables.InMesh;

    InMesh.CreateElementNeighbourList(rModelPart.Elements(MeshId).size(), nds);

    int* ElementNeighbourList      = InMesh.GetElementNeighbourList();   

    for(unsigned int el = 0; el<rModelPart.Elements(MeshId).size(); el++)
      {
	WeakPointerVector<Element >& rE = (element_begin+el)->GetValue(NEIGHBOUR_ELEMENTS);

	for(unsigned int pn=0; pn<nds; pn++){
	  if( (element_begin+el)->Id() == rE[pn].Id() )
	    ElementNeighbourList[el*nds+pn] = 0;
	  else
	    ElementNeighbourList[el*nds+pn] = rE[pn].Id();
	}
	      
      }

    KRATOS_CATCH( "" )

  }

  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::SetElementNeighbours(ModelPart& rModelPart,
					 MeshingParametersType & rMeshingVariables,
					 ModelPart::IndexType MeshId)
  {
    KRATOS_TRY

    if( this->GetEchoLevel() > 0 ){
      std::cout<<" [ SET ELEMENT NEIGHBOURS : "<<std::endl;
      std::cout<<"   Initial Faces : "<<rModelPart.Conditions(MeshId).size()<<std::endl;
    }

    ModelPart::ElementsContainerType::const_iterator el_begin = rModelPart.ElementsBegin(MeshId);
	
    int facecounter=0;
    for(ModelPart::ElementsContainerType::const_iterator iii = rModelPart.ElementsBegin(MeshId);
	iii != rModelPart.ElementsEnd(MeshId); iii++)
      {

	int Id= iii->Id() - 1;
	//std::cout<<" Id ELNEIG "<<Id<<std::endl;


	int number_of_faces = iii->GetGeometry().FacesNumber(); //defined for triangles and tetrahedra
	(iii->GetValue(NEIGHBOUR_ELEMENTS)).resize(number_of_faces);
	WeakPointerVector< Element >& neighb = iii->GetValue(NEIGHBOUR_ELEMENTS);

	for(int i = 0; i<number_of_faces; i++)
	  {
	    int index = rMeshingVariables.NeighbourList[Id][i];
				
	    if(index > 0)
	      {
		//std::cout<<" Element "<<Id<<" size "<<rMeshingVariables.PreservedElements.size()<<std::endl;			    
		//std::cout<<" Index pre "<<index<<" size "<<rMeshingVariables.PreservedElements.size()<<std::endl;
		index = rMeshingVariables.PreservedElements[index-1];
		//std::cout<<" Index post "<<index<<std::endl;
	      }

	    if(index > 0)
	      {
		neighb(i) = *((el_begin + index -1 ).base());
	      }
	    else
	      {
		//neighb(i) = Element::WeakPointer();
		neighb(i) = *(iii.base());
		facecounter++;
	      }
	  }
      }
	
    if( this->GetEchoLevel() > 0 ){
      std::cout<<"   Final Faces : "<<facecounter<<std::endl;
      std::cout<<"   SET ELEMENT NEIGHBOURS ]; "<<std::endl;
    }

    KRATOS_CATCH( "" )

  }


  //*******************************************************************************************
  //*******************************************************************************************

  void MeshModeler::RecoverBoundaryPosition(ModelPart& rModelPart,
					    MeshingParametersType& rMeshingVariables,
					    ModelPart::IndexType MeshId)
  {
    KRATOS_TRY
    
    const unsigned int dimension = rModelPart.ElementsBegin(MeshId)->GetGeometry().WorkingSpaceDimension();
          
    //*********************************************************************
    //input mesh: ELEMENTS

    ModelerUtilities::MeshContainer& InMesh = rMeshingVariables.InMesh;
    double* InPointList  = InMesh.GetPointList();

    ModelerUtilities::MeshContainer& OutMesh = rMeshingVariables.OutMesh;
    double* OutPointList = OutMesh.GetPointList();

    ModelPart::NodesContainerType::iterator nodes_begin = rModelPart.NodesBegin(MeshId);

    int base=0;
    for(unsigned int i = 0; i<rModelPart.Nodes(MeshId).size(); i++)
      { 
	   
	if( (nodes_begin + i)->Is(BOUNDARY) ){
	  
	  array_1d<double, 3>& Position = (nodes_begin + i)->Coordinates();
	  for( unsigned int j=0; j<dimension; j++)
	    {
	      InPointList[base+j]    = Position[j];
	      OutPointList[base+j]   = Position[j];
	    }
	}
	   
	base+=dimension;
      }

    KRATOS_CATCH( "" )
  }


} // Namespace Kratos

