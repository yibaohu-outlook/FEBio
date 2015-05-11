//
//  FEFiberPowLinear.cpp
//  FEBioMech
//
//  Created by Gerard Ateshian on 5/2/15.
//  Copyright (c) 2015 febio.org. All rights reserved.
//

#include "FEFiberPowLinear.h"

// define the material parameters
BEGIN_PARAMETER_LIST(FEFiberPowLinear, FEElasticMaterial)
    ADD_PARAMETER(m_E    , FE_PARAM_DOUBLE, "E"    );
    ADD_PARAMETER(m_lam0 , FE_PARAM_DOUBLE, "lam0" );
    ADD_PARAMETER(m_beta , FE_PARAM_DOUBLE, "beta" );
    ADD_PARAMETER(m_thd  , FE_PARAM_DOUBLE, "theta");
    ADD_PARAMETER(m_phd  , FE_PARAM_DOUBLE, "phi"  );
END_PARAMETER_LIST();

//-----------------------------------------------------------------------------
// FEFiberPowLinear
//-----------------------------------------------------------------------------

void FEFiberPowLinear::Init()
{
    if (m_E < 0) throw MaterialError("E must be positive.");
    if (m_lam0 <= 1) throw MaterialError("lam0 must be >1.");
    if (m_beta <  2) throw MaterialError("beta must be >= 2.");
    
    // convert angles from degrees to radians
    double pi = 4*atan(1.0);
    double the = m_thd*pi/180.;
    double phi = m_phd*pi/180.;
    // fiber direction in local coordinate system (reference configuration)
    m_n0.x = cos(the)*sin(phi);
    m_n0.y = sin(the)*sin(phi);
    m_n0.z = cos(phi);
    
    // initialize material constants
    m_I0 = m_lam0*m_lam0;
    m_ksi = m_E/4/(m_beta-1)*pow(m_I0, -3./2.)*pow(m_I0-1, 2-m_beta);
    m_b = m_ksi*pow(m_I0-1, m_beta-1) + m_E/2/sqrt(m_I0);
}

//-----------------------------------------------------------------------------
mat3ds FEFiberPowLinear::Stress(FEMaterialPoint& mp)
{
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // deformation gradient
    mat3d &F = pt.m_F;
    double J = pt.m_J;
    
    // loop over all integration points
    vec3d n0, nt;
    double In, sn;
    const double eps = 0;
    mat3ds C = pt.RightCauchyGreen();
    mat3ds s;
    
    // evaluate fiber direction in global coordinate system
    n0 = pt.m_Q*m_n0;
    
    // Calculate In
    In = n0*(C*n0);
    
    // only take fibers in tension into consideration
    if (In - 1 > eps)
    {
        // get the global spatial fiber direction in current configuration
        nt = F*n0/sqrt(In);
        
        // calculate the outer product of nt
        mat3ds N = dyad(nt);
        
        // calculate the fiber stress magnitude
        sn = (In < m_I0) ?
        2*In*m_ksi*pow(In-1, m_beta-1) :
        2*m_b*In - m_E*sqrt(In);
        
        // calculate the fiber stress
        s = N*(sn/J);
    }
    else
    {
        s.zero();
    }
    
    return s;
}

//-----------------------------------------------------------------------------
tens4ds FEFiberPowLinear::Tangent(FEMaterialPoint& mp)
{
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // deformation gradient
    mat3d &F = pt.m_F;
    double J = pt.m_J;
    
    // loop over all integration points
    vec3d n0, nt;
    double In, cn;
    const double eps = 0;
    mat3ds C = pt.RightCauchyGreen();
    tens4ds c;
    
    // evaluate fiber direction in global coordinate system
    n0 = pt.m_Q*m_n0;
    
    // Calculate In
    In = n0*(C*n0);
    
    // only take fibers in tension into consideration
    if (In - 1 > eps)
    {
        // get the global spatial fiber direction in current configuration
        nt = F*n0/sqrt(In);
        
        // calculate the outer product of nt
        mat3ds N = dyad(nt);
        tens4ds NxN = dyad1s(N);
        
        // calculate modulus
        cn = (In < m_I0) ?
        4*In*In*m_ksi*(m_beta-1)*pow(In-1, m_beta-2) :
        m_E*sqrt(In);
        
        // calculate the fiber tangent
        c = NxN*(cn/J);
    }
    else
    {
        c.zero();
    }
    
    return c;
}

//-----------------------------------------------------------------------------
double FEFiberPowLinear::StrainEnergyDensity(FEMaterialPoint& mp)
{
    double sed = 0.0;
    
    FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();
    
    // loop over all integration points
    vec3d n0;
    double In;
    const double eps = 0;
    mat3ds C = pt.RightCauchyGreen();
    
    // evaluate fiber direction in global coordinate system
    n0 = pt.m_Q*m_n0;
    
    // Calculate In = n0*C*n0
    In = n0*(C*n0);
    
    // only take fibers in tension into consideration
    if (In - 1 > eps)
    {
        // calculate strain energy density
        sed = (In < m_I0) ?
        m_ksi/m_beta*pow(In-1, m_beta) :
        m_b*(In-m_I0) - m_E*(sqrt(In)-sqrt(m_I0)) + m_ksi/m_beta*pow(m_I0-1, m_beta);
    }
    
    return sed;
}