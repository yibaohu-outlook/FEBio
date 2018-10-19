#include "stdafx.h"
#include "FEDomain.h"
#include "FEMaterial.h"

//-----------------------------------------------------------------------------
FEDomain::FEDomain(int nclass, FEModel* fem) : FEMeshPartition(nclass, fem)
{

}

//-----------------------------------------------------------------------------
void FEDomain::SetMaterial(FEMaterial* pm)
{
	assert(pm);
	if (pm) pm->AddDomain(this);
}

//-----------------------------------------------------------------------------
void FEDomain::SetMatID(int mid)
{
	for (int i = 0; i<Elements(); ++i) ElementRef(i).SetMatID(mid);
}

//-----------------------------------------------------------------------------
// This routine allocates the material point data for the element's integration points.
// Currently, this has to be called after the elements have been assigned a type (since this
// determines how many integration point an element gets). 
void FEDomain::CreateMaterialPointData()
{
	FEMaterial* pmat = GetMaterial();
	if (pmat != 0)
	{
		for (int i = 0; i<Elements(); ++i)
		{
			FEElement& el = ElementRef(i);
			for (int k = 0; k<el.GaussPoints(); ++k) el.SetMaterialPointData(pmat->CreateMaterialPointData(), k);
		}
	}
}

// serialization
void FEDomain::Serialize(DumpStream& ar)
{
	FEMeshPartition::Serialize(ar);

	// TODO: Do I need to call CreateMaterialPointData() here?
}
