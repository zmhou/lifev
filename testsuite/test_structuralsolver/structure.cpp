//@HEADER
/*
*******************************************************************************

    Copyright (C) 2004, 2005, 2007 EPFL, Politecnico di Milano, INRIA
    Copyright (C) 2010 EPFL, Politecnico di Milano, Emory University

    This file is part of LifeV.

    LifeV is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LifeV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LifeV.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************
*/
//@HEADER
/**
   \file cylinder.cpp
   \author Christophe Prud'homme <christophe.prudhomme@epfl.ch>
   \date 2005-04-19
 */

// #include <life/lifecore/LifeV.hpp>
// #include <life/lifefilters/GetPot.hpp>
// #include <life/lifecore/LifeDebug.hpp>

// #include <life/lifefilters/importer.hpp>

//#include "NavierStokesSolverIP.hpp"

#include <life/lifearray/MapEpetra.hpp>

#include <life/lifemesh/MeshData.hpp>
#include <life/lifemesh/MeshPartitioner.hpp>

#include <life/lifesolver/VenantKirchhoffElasticData.hpp>

#include <life/lifesolver/StructuralMaterial.hpp>
#include <life/lifesolver/StructuralSolver.hpp>
#include <life/lifesolver/VenantKirchhoffMaterialLinear.hpp>
#include <life/lifesolver/VenantKirchhoffMaterialNonLinear.hpp>

#include <life/lifefilters/ExporterEnsight.hpp>
#include <life/lifefilters/ExporterHDF5.hpp>
#include <life/lifefilters/ExporterEmpty.hpp>

#include "structure.hpp"
#include "ud_functions.hpp"

#include <iostream>

using namespace LifeV;

struct Structure::Private
{
    Private() :
            rho(1), poisson(1), young(1)
    {}
    typedef boost::function<Real ( Real const&, Real const&, Real const&, Real const&, ID const& )> fct_type;
    double rho, poisson, young;

    std::string data_file_name;

    boost::shared_ptr<Epetra_Comm>     comm;


};

Structure::Structure( int                                   argc,
                      char**                                argv,
                      boost::shared_ptr<Epetra_Comm>        structComm):
        parameters( new Private() )
{
    GetPot command_line(argc, argv);
    string data_file_name = command_line.follow("data", 2, "-f", "--file");
    GetPot dataFile( data_file_name );
    parameters->data_file_name = data_file_name;

    parameters->rho = dataFile( "solid/physics/density", 1. );
    parameters->young = dataFile( "solid/physics/young", 1. );
    parameters->poisson  = dataFile( "solid/physics/poisson", 1. );

    std::cout << "density = " << parameters->rho << std::endl
              << "young   = " << parameters->young << std::endl
              << "poisson = " << parameters->poisson << std::endl;

    parameters->comm = structComm;
    int ntasks = parameters->comm->NumProc();

    if (!parameters->comm->MyPID()) std::cout << "My PID = " << parameters->comm->MyPID() << " out of " << ntasks << " running." << std::endl;

}

void
Structure::run2d()
{
    std::cout << "2D cylinder test case is not available yet\n";
}

void
Structure::run3d()
{
    typedef StructuralSolver< RegionMesh3D<LinearTetra> >::vector_Type  vector_Type;
    typedef boost::shared_ptr<vector_Type> vectorPtr_Type;
    
    bool verbose = (parameters->comm->MyPID() == 0);
    // Number of boundary conditions for the velocity and mesh motion
    //
    boost::shared_ptr<BCHandler> BCh( new BCHandler() );

    //
    // dataElasticStructure
    //

    GetPot dataFile( parameters->data_file_name.c_str() );

    boost::shared_ptr<VenantKirchhoffElasticData> dataStructure(new VenantKirchhoffElasticData( ));
    dataStructure->setup(dataFile);

    MeshData             meshData;
    meshData.setup(dataFile, "solid/space_discretization");

    boost::shared_ptr<RegionMesh3D<LinearTetra> > fullMeshPtr(new RegionMesh3D<LinearTetra>);
    readMesh(*fullMeshPtr, meshData);


    MeshPartitioner< RegionMesh3D<LinearTetra> > meshPart( fullMeshPtr, parameters->comm );

    std::string dOrder =  dataFile( "solid/space_discretization/order", "P1");

    typedef FESpace< RegionMesh3D<LinearTetra>, MapEpetra > solidFESpace_type;
    typedef boost::shared_ptr<solidFESpace_type> solidFESpace_ptrtype;
    solidFESpace_ptrtype dFESpace( new solidFESpace_type(meshPart,dOrder,3,parameters->comm) );
    if (verbose) std::cout << std::endl;

    MapEpetra structMap(dFESpace->refFE(), meshPart, parameters->comm);

    MapEpetra fullMap;

    for (UInt ii = 0; ii < nDimensions; ++ii)
    {
        fullMap += structMap;
    }

    //BCFunctions
    BCFunctionBase fixed1(g1);  // homogeneous boundary values
    BCFunctionBase fixed2(g2);  // pressure
    BCFunctionBase Homogen(fzero_scalar);
    BCFunctionBase Intern(InternalPressure);

    /*
    // BC for cyl1x02_1796_edge.mesh
    vector <ID> compx(1), compy(1), compz(1);
    compx[0]=0; compy[0]=1, compz[0]=2;

    // lower base
    BCh->addBC("Base10 ", 2 , Essential, Component, fixed1, compz);
    BCh->addBC("Base10 ", 20 , Essential, Component, fixed1, compz);

    // upper base
    BCh->addBC("Base3", 3 , Natural, Normal,Intern); // traction
    BCh->addBC("Base3", 30 , Natural, Normal,Intern); // traction
    //BCh->addBC("Base2 ", 3, Natural, Full, fixed1, 3); // free stress
    //BCh->addBC("Base2 ", 30, Natural, Full, fixed1, 3); // free stress

    //BCh->addBC("Base3", 1 , Natural, Normal,Intern); // compression
    BCh->addBC("Base3", 1 , Natural, Full, fixed1, 3); // free stress surfac
    */
    
// BC for vessel2x4x20_10cm.mesh
    vector <ID> compx(1), compy(1), compz(1);
    compx[0]=0; compy[0]=1, compz[0]=2;

    // lower base
    BCh->addBC("BaseSx ", 2 , Essential, Component, fixed1, compz);
    BCh->addBC("BaseDx ", 3 , Essential, Component, fixed1, compz);

    // upper base
    BCh->addBC("BaseEx", 1, Natural, Normal,Intern); // internal pressure
    ////////////vessel20.mesh////////////////////////////////////////////
    BCh->addBC("BaseEx", 20, Natural, Normal,Intern); // internal pressure

    BCh->addBC("BaseIn", 10, Natural, Normal,  Homogen); // external pressure
    //BCh->addBC("BaseEx", 10, Natural, Full, fixed1, 3); // external pressure
    

    StructuralSolver< RegionMesh3D<LinearTetra> > solid;
    solid.setup(dataStructure,
                dFESpace,
		BCh,
                parameters->comm);

    solid.setDataFromGetPot(dataFile);
    solid.buildSystem();
    

    //
    // Temporal data and initial conditions
    //

    Real dt = dataStructure->dataTime()->timeStep();
    Real T  = dataStructure->dataTime()->endTime();

    vectorPtr_Type disp(new vector_Type(solid.displacement(), Unique));
    vectorPtr_Type vel(new vector_Type(solid.velocity(), Unique));
    vectorPtr_Type acc(new vector_Type(solid.acceleration(), Unique));

    dFESpace->interpolate(d0, *disp, 0.0);
    dFESpace->interpolate(w0, *vel , 0.0);
    dFESpace->interpolate(a0, *acc , 0.0);

    if (verbose) std::cout << "S- initialization ... ";

    solid.initialize(d0,w0,a0); // displacement, velocity, acceleration

    MPI_Barrier(MPI_COMM_WORLD);

    if (verbose ) std::cout << "ok." << std::endl;
    //if (parameters->comm->NumProc() == 1 )  solid.postProcess();


    boost::shared_ptr< Exporter<RegionMesh3D<LinearTetra> > > exporter;

    std::string const exporterType =  dataFile( "exporter/type", "ensight");
#ifdef HAVE_HDF5
    if (exporterType.compare("hdf5") == 0)
    {
      exporter.reset( new ExporterHDF5<RegionMesh3D<LinearTetra> > ( dataFile, "structure" ) );
    }
    else
#endif
      {
        if (exporterType.compare("none") == 0)
	  {
	    exporter.reset( new ExporterEmpty<RegionMesh3D<LinearTetra> > ( dataFile, meshPart.meshPartition(), "structure", parameters->comm->MyPID()) );
	  }
        else
	  {
	    exporter.reset( new ExporterEnsight<RegionMesh3D<LinearTetra> > ( dataFile, meshPart.meshPartition(), "structure", parameters->comm->MyPID()) );
	  }
      }

	exporter->setPostDir( "./" ); // This is a test to see if M_post_dir is working
	exporter->setMeshProcId( meshPart.meshPartition(), parameters->comm->MyPID() );

	//vectorPtr_Type solidDisp ( new vector_Type(solid.getDisplacement(), exporter->mapType() ) );
	//vectorPtr_Type solidVel  ( new vector_Type(solid.getVelocity(),  exporter->mapType() ) );
	
	vectorPtr_Type solidDisp ( new vector_Type(solid.displacement(), exporter->mapType() ) );
	vectorPtr_Type solidVel  ( new vector_Type(solid.velocity(),  exporter->mapType() ) );
	vectorPtr_Type solidAcc  ( new vector_Type(solid.acceleration(),  exporter->mapType() ) );
	
	exporter->addVariable( ExporterData::Vector, "displacement", solidDisp,
			       UInt(0), dFESpace->dof().numTotalDof() );
	
	exporter->addVariable( ExporterData::Vector, "velocity", solidVel,
			       UInt(0), dFESpace->dof().numTotalDof() );

	exporter->addVariable( ExporterData::Vector, "acceleration", solidAcc,
			       UInt(0), dFESpace->dof().numTotalDof() );


	exporter->postProcess( 0 );


	//
	// Temporal loop
	//

	for (Real time = dt; time <= T; time += dt)
	  {
	    dataStructure->dataTime()->setTime(time);

	    if (verbose)
	      {
		std::cout << std::endl;
		std::cout << "S- Now we are at time " << dataStructure->dataTime()->time() << " s." << std::endl;
	      }
	
	    //solid.updateSystem(dZero);    // Computes the rigth hand side
	    solid.updateSystem();    // Computes the rigth hand side

	    std::cout << "Updated system at " << time << std::endl;

	    solid.iterate( BCh );                  // Computes the matrices and solves the system
	    //if (parameters->comm->NumProc() == 1 )  solid.postProcess(); // Post-presssing

	    *solidDisp = solid.displacement();
	    *solidVel  = solid.velocity();
	    *solidAcc  = solid.acceleration();
	    
	    //if (parameters->comm->NumProc() == 1 )  solid.postProcess(); // Post-presssing

	    exporter->postProcess( time );

	  }

}
    

//////////////////////

