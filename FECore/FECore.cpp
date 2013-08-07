#include "stdafx.h"
#include "FECore.h"
#include "FECoordSysMap.h"
#include "febio.h"

#define FECORE_VERSION		0
#define FECORE_SUBVERSION	1

//-----------------------------------------------------------------------------
// Get the version info
void FECore::get_version(int& version, int& subversion)
{
	version = FECORE_VERSION;
	subversion = FECORE_SUBVERSION;
}

//-----------------------------------------------------------------------------
// get the version string
const char* FECore::get_version_string()
{
	static const char fecore_str[4] = {'0'+FECORE_VERSION, '.', '0'+FECORE_SUBVERSION };
	return fecore_str;
}

//-----------------------------------------------------------------------------
void FECore::InitModule()
{
REGISTER_FEBIO_CLASS(FELocalMap      , FECoordSysMap, "local"      );
REGISTER_FEBIO_CLASS(FESphericalMap  , FECoordSysMap, "spherical"  );
REGISTER_FEBIO_CLASS(FECylindricalMap, FECoordSysMap, "cylindrical");
REGISTER_FEBIO_CLASS(FEVectorMap     , FECoordSysMap, "vector"     );
}
