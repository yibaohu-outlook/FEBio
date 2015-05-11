#pragma once

#include "FEContactInterface.h"
#include "FEPeriodicBoundary.h"
#include "FEContactSurface.h"
#include <FECore/tens3d.h>

//-----------------------------------------------------------------------------

class FEPeriodicBoundary2O : public FEContactInterface
{
public:
	//! constructor
	FEPeriodicBoundary2O(FEModel* pfem);

	//! destructor
	virtual ~FEPeriodicBoundary2O(void) {}

	//! initialization
	bool Init();

	//! interface activation
	void Activate();

	//! update
	void Update(int niter);

	//! shallow copy
	void ShallowCopy(DumpStream& dmp, bool bsave);

	//! calculate contact forces
	void ContactForces(FEGlobalVector& R);

	//! calculate contact stiffness
	void ContactStiffness(FESolver* psolver);

	//! calculate Lagrangian augmentations
	bool Augment(int naug);

	//! serialize data to archive
	void Serialize(DumpFile& ar);

	//! return the master and slave surface
	FESurface* GetMasterSurface() { return &m_ms; }
	FESurface* GetSlaveSurface () { return &m_ss; }

	//! return integration rule class
	bool UseNodalIntegration() { return true; }

	//! build the matrix profile for use in the stiffness matrix
	void BuildMatrixProfile(FEStiffnessMatrix& K);

	//! create a copy of this interface
	void CopyFrom(FESurfacePairInteraction* pci);

protected:
	void ProjectSurface(FEPeriodicSurface& ss, FEPeriodicSurface& ms, bool bmove);

public:
	FEPeriodicSurface		m_ss;	//!< slave surface
	FEPeriodicSurface		m_ms;	//!< master surface

	double	m_atol;			//!< augmentation tolerance
	double	m_eps;			//!< penalty scale factor
	double	m_stol;			//!< search tolerance
	double  m_srad;			//!< search radius (%)
	bool	m_btwo_pass;	//!< two-pass flag
	int		m_naugmin;		//!< minimum number of augmentations
	vec3d	m_off;			//!< relative displacement offset

	mat3d		m_Fmacro;		//!< Macroscopic deformation gradient
	tens3drs	m_Gmacro;		//!< Macroscopic deformation Hessian
	
	DECLARE_PARAMETER_LIST();
};