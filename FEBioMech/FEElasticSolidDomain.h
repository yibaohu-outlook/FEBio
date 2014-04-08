#pragma once
#include "FECore/FESolidDomain.h"
#include "FECore/FEModel.h"
#include "FEElasticDomain.h"

//-----------------------------------------------------------------------------
//! domain described by Lagrange-type 3D volumetric elements
//!
class FEElasticSolidDomain : public FESolidDomain, public FEElasticDomain
{
public:
	//! constructor
	FEElasticSolidDomain(FEMesh* pm, FEMaterial* pmat) : FESolidDomain(FE_SOLID_DOMAIN, pm, pmat) {}

	//! \todo Do I really use this?
	FEElasticSolidDomain& operator = (FEElasticSolidDomain& d) { m_Elem = d.m_Elem; m_pMesh = d.m_pMesh; return (*this); }

	//! initialize class
	bool Initialize(FEModel& fem);

	//! Init elements
	void InitElements();

	//! reset element data
	void Reset();

	//! Unpack solid element data
	void UnpackLM(FEElement& el, vector<int>& lm);

public: // overrides from FEElasticDomain

	// update stresses
	void UpdateStresses(FEModel& fem);

	// update the element stress
	void UpdateElementStress(int iel, double dt);

	//! calculates the residual
//	void Residual(FESolver* psolver, vector<double>& R);

	//! intertial forces for dynamic problems
	void InertialForces(FEGlobalVector& R, vector<double>& F);

	//! internal stress forces
	void InternalForces(FEGlobalVector& R);

	//! body forces
	void BodyForce(FEGlobalVector& R, FEBodyForce& BF);

	//! calculates the global stiffness matrix for this domain
	void StiffnessMatrix(FESolver* psolver);

	//! calculates inertial stiffness
	void InertialStiffness(FESolver* psolver);

	//! body force stiffness
	void BodyForceStiffness(FESolver* psolver, FEBodyForce& bf);

public:
	// --- S T I F F N E S S ---

	//! calculates the solid element stiffness matrix
	virtual void ElementStiffness(FEModel& fem, int iel, matrix& ke);

	//! geometrical stiffness (i.e. initial stress)
	void ElementGeometricalStiffness(FESolidElement& el, matrix& ke);

	//! material stiffness component
	void ElementMaterialStiffness(FESolidElement& el, matrix& ke);

	//! calculates the solid element inertial stiffness matrix
	void ElementInertialStiffness(FEModel& fem, FESolidElement& el, matrix& ke);

	//! calculates the stiffness matrix due to body forces 
	void ElementBodyForceStiffness(FEBodyForce& bf, FESolidElement& el, matrix& ke);

	// --- R E S I D U A L ---

	//! Calculates the internal stress vector for solid elements
	void ElementInternalForce(FESolidElement& el, vector<double>& fe);

	//! Calculatess external body forces for solid elements
//	void ElementBodyForce(FEModel& fem, FESolidElement& elem, vector<double>& fe);

	//! Calculatess external body forces for solid elements
	void ElementBodyForce(FEBodyForce& BF, FESolidElement& elem, vector<double>& fe);

	// ---

private:
	//! Helper function for setting the material point's local coordinate system
	void SetLocalCoordinateSystem(FEElement& el, int n, FEMaterialPoint& mp, FEElasticMaterial* pme);
};
