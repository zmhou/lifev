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

/*!
 *  @file
 *  @brief This file contains a class which can be used to evaluate the wall tension in the arterial wall.
 *
 *  @version 1.0
 *  @date 01-01-2010
 *  @author Paolo Tricerri
 * 
 *  @maintainer  Paolo Tricerri <paolo.tricerri@epfl.ch>
 *  
 */

#ifndef _WALLTENSION_H_
#define _WALLTENSION_H_ 1

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

//Trilinos include
#include "Epetra_SerialDenseMatrix.h"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

// LifeV core includes
#include <lifev/core/array/MatrixElemental.hpp>
#include <lifev/core/array/VectorElemental.hpp>
#include <lifev/core/array/MatrixEpetra.hpp>
#include <lifev/core/array/VectorEpetra.hpp>
#include <lifev/core/fem/Assembly.hpp>
#include <lifev/core/fem/AssemblyElemental.hpp>
#include <lifev/core/fem/FESpace.hpp>
#include <lifev/core/LifeV.hpp>
#include <lifev/core/util/Displayer.hpp>
#include <lifev/core/util/Factory.hpp>
#include <lifev/core/util/FactorySingleton.hpp>
#include <lifev/core/array/MapEpetra.hpp>

#include <lifev/core/filter/ExporterEnsight.hpp>
#ifdef HAVE_HDF5
#include <lifev/core/filter/ExporterHDF5.hpp>
#endif
#include <lifev/core/filter/ExporterEmpty.hpp>


// Structure module include
#include <lifev/structure/fem/AssemblyElementalStructure.hpp>
#include <lifev/structure/solver/VenantKirchhoffElasticData.hpp>
#include <lifev/structure/solver/WallTensionEstimatorData.hpp>
#include <lifev/structure/solver/StructuralMaterial.hpp>

//Materials
#include <lifev/structure/solver/VenantKirchhoffMaterialLinear.hpp>
#include <lifev/structure/solver/VenantKirchhoffMaterialNonLinear.hpp>
#include <lifev/structure/solver/ExponentialMaterialNonLinear.hpp>
#include <lifev/structure/solver/NeoHookeanMaterialNonLinear.hpp>


namespace LifeV
{
/*!
  \class WallTensionEstimator
  \brief
  This class lets to compute the wall tensions inside the arterial wall. The tensorial operations
  that are needed to compute the stress tensor are defined in AssemblyElementalStructure. When a new
  type of analysis wants to be performed new methods can be added
*/

template <typename Mesh>
class WallTensionEstimator
{
public:

//!@name Type definitions
//@{

    // Data classes
    typedef VenantKirchhoffElasticData                   data_Type;
    typedef WallTensionEstimatorData                     analysisData_Type;
    typedef typename boost::shared_ptr<data_Type>        dataPtr_Type;
    typedef typename boost::shared_ptr<data_Type>        analysisDataPtr_Type;

    //Matrices 3x3 and std::vector for the invariants
    typedef Epetra_SerialDenseMatrix                     matrix_Type;
    typedef boost::shared_ptr<matrix_Type>               matrixPtr_Type;
    typedef std::vector<LifeV::Real>                     vector_Type;
    typedef boost::shared_ptr<vector_Type>               vectorPtr_Type;

    // These two are to handle the vector displacement read from hdf5
    typedef VectorEpetra                                 solutionVect_Type;
    typedef boost::shared_ptr<VectorEpetra>              solutionVectPtr_Type;

    // Displayer and Exporter classes
    typedef typename boost::shared_ptr<const Displayer>   displayerPtr_Type;
    typedef typename boost::shared_ptr< Exporter<Mesh> >  exporterPtr_Type;

    // Materials
    typedef StructuralMaterial<Mesh>               material_Type;
    typedef boost::shared_ptr<material_Type>       materialPtr_Type;

//@}


//! @name Constructor &  Deconstructor
//@{

  WallTensionEstimator();

  virtual ~WallTensionEstimator() {}

//@}



//!@name Methods
//@{

    //! Setup the created object of the class WallTensionEstimator
    /*!
      \param dataMaterial: the class containing the VenantKirchhoffElasticData
      \param tensionData: the class containing the WallTensionEstimatorData
      \param dFESpace: the FiniteElement Space
      \param displayer: the displayer object
    */
    void setup( const dataPtr_Type& dataMaterial,
		const analysisDataPtr_Type& tensionData,
	        const boost::shared_ptr< FESpace<Mesh, MapEpetra> >& dFESpace,
		boost::shared_ptr<Epetra_Comm>&     comm);


    //! Computes the principal tensions for each DOF of each volume
    /*!
      \param no parameters
    */
     void analizeTensions( void );

    //! Updates the Jacobian matrix in StructuralSolver::updateJacobian
    /*!
      \param disp: solution at the k-th iteration of NonLinearRichardson Method
      \param dataMaterial: a pointer to the dataType member in StructuralSolver class to get the material coefficients (e.g. Young modulus, Poisson ratio..)
      \param displayer: a pointer to the Dysplaier member in the StructuralSolver class
    */
    virtual  void updateJacobianMatrix( const vector_Type& disp, const dataPtr_Type& dataMaterial, const displayerPtr_Type& displayer ) = 0;

    //! Computes the new Stiffness matrix in StructuralSolver given a certain displacement field. This function is used both in StructuralSolver::evalResidual and in
    //! StructuralSolver::updateSystem since the matrix is the expression of the matrix is the same.
    //!This is virtual and not pure virtual since in the linear St. Venant-Kirchhoff law it is not needed.
    /*!
      \param sol:  the solution vector
      \param factor: scaling factor used in FSI
      \param dataMaterial: a pointer to the dataType member in StructuralSolver class to get the material coefficients (e.g. Young modulus, Poisson ratio..)
      \param displayer: a pointer to the Dysplaier member in the StructuralSolver class
    */
    virtual  void computeStiffness( const vector_Type& sol, Real factor, const dataPtr_Type& dataMaterial, const displayerPtr_Type& displayer ) = 0;


    //! Computes the deformation Gradient F, the cofactor of F Cof(F), the determinant of F J = det(F), the trace of C Tr(C).
    /*!
      \param dk_loc: local displacement vector
    */
    virtual  void computeKinematicsVariables( const VectorElemental& dk_loc ) = 0;


  //! Output of the class
  /*!
    \param fileNamelinearStiff the filename where to apply the spy method for the linear part of the Stiffness matrix
    \param fileNameStiff the filename where to apply the spy method for the Stiffness matrix
  */
  virtual void showMe( std::string const& fileNameStiff, std::string const& fileNameJacobian ) = 0;


//! @name Set Methods
//@{

  //! Set the displacement vector
  void setDisplacement(solutionVect_Type& displVect) {M_displ = displVect;}

//@}


//! @name Get Methods
//@{

  //! Getters
  //! Get the Epetramap
  MapEpetra   const& map()     const { return *M_localMap; }

  //! Get the FESpace object
  FESpace<Mesh, MapEpetra>& dFESpace()  {return *M_FESpace;}

  //! Get the pointer to the FESpace object
  boost::shared_ptr<FESpace<Mesh, MapEpetra> > dFESpacePtr()  {return M_FESpace;}

  //! Get the Stiffness matrix
  virtual matrixPtr_Type const stiffMatrix() const = 0;

  //! Get the Stiffness matrix
  virtual vectorPtr_Type const stiffVector() const = 0;

  virtual void Apply( const vector_Type& sol, vector_Type& res) =0;
  
//@}

protected:

    //!Protected Members
    boost::shared_ptr<FESpace<Mesh, MapEpetra> >   M_FESpace;

    boost::shared_ptr<const MapEpetra>             M_localMap;

    //! Elementary matrix for the tensor F
    matrixPtr_Type                                 M_deformationF;
    //! Elementary matrix for the tensor C
    matrixPtr_Type                                 M_rightCauchyC;
    //! Elementary matrix for the tensor P
    matrixPtr_Type                                 M_firstPiola;

    //! Elementary matrix for the tensor \sigma (Cauchy tensor on the current config)
    matrixPtr_Type                                 M_sigma;

    //! Vector of the invariants of C
    vector_Type                                    M_invariants;

    //! Vector for the displacement field
    solutionVectPtr_Type                            M_displ;

    //! Determinant of the deformation gradient F
    Real                                           M_detF;
  
    //! The Offset parameter
    UInt                                           M_offset;

    //Class for material parameter
    dataPtr_Type                                   M_dataMaterial;

    //Class for analysis parameter
    analysisDataPtr_Type                           M_analysisData;

    //Displayer
    displayerPtr_Type                              M_displayer;


    //! Material class
    materialPtr_Type                               M_material;

    //Importer
    //exporterPtr_Type                                M_importer;

};

//=====================================
// Constructor
//=====================================

template <typename Mesh>
WallTensionEstimator<Mesh>::WallTensionEstimator( ):
    M_FESpace                    ( ),
    M_localMap                   ( ),
    M_offset                     ( 0 ),
    M_dataMaterial               ( ),
    M_analysisData               ( ),
    M_displayer                  ( ),
    //    M_importer                   ( ),
    M_detF                       ( 0 ),
    M_sigma                      ( ),
    M_deformationF               ( ),
    M_rightCauchyC               ( ),
    M_firstPiola                 ( ),
    M_invariants                 ( ),
    M_displ                      ( ),
    M_material                   ( )
{
  
}



//====================================
// Public Methods
//===================================
template <typename Mesh>
void 
WallTensionEstimator<Mesh >::setup( const dataPtr_Type& dataMaterial,
				    const analysisDataPtr_Type& tensionData,
				    const boost::shared_ptr< FESpace<Mesh, MapEpetra> >& dFESpace,
				    boost::shared_ptr<Epetra_Comm>&     comm)
  
{

  // Data classes
  M_dataMaterial = dataMaterial;
  M_analysisData = tensionData;

  // FESpace and EpetraMap
  M_FESpace      = dFESpace;
  M_localMap     = dFESpace->mapPtr();

  // Displayer
  M_displayer.reset    (new Displayer(comm));

  // Vector and Tensors
  M_sigma.reset        (new matrix_Type( M_FESpace->fieldDim(),M_FESpace->fieldDim() ) );
  M_deformationF.reset (new matrix_Type( M_FESpace->fieldDim(),M_FESpace->fieldDim() ) );
  M_rightCauchyC.reset (new matrix_Type( M_FESpace->fieldDim(),M_FESpace->fieldDim() ) );
  M_firstPiola.reset   (new matrix_Type( M_FESpace->fieldDim(),M_FESpace->fieldDim() ) );
  M_displ.reset        (new solutionVect_Type(*M_localMap) );

  // Materials
  M_material.reset( material_Type::StructureMaterialFactory::instance().createObject( M_dataMaterial->solidType() ) );
  M_material->setup( dFESpace,M_localMap,M_offset, M_dataMaterial, M_displayer );
}

template <typename Mesh>
void 
WallTensionEstimator<Mesh >::analyzeTensions( void )
{

}

}
#endif /*_STRUCTURALMATERIAL_H*/