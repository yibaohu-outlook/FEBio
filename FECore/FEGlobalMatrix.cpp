#include "stdafx.h"
#include "FEGlobalMatrix.h"

//-----------------------------------------------------------------------------
//! Takes a SparseMatrix structure that defines the structure of the global matrix.
FEGlobalMatrix::FEGlobalMatrix(NumCore::SparseMatrix* pK)
{
	m_pA = pK;
	m_LM.resize(MAX_LM_SIZE);
	m_pMP = 0;
	m_nlm = 0;
}

//-----------------------------------------------------------------------------
//! Deletes the SparseMatrix variable.
FEGlobalMatrix::~FEGlobalMatrix()
{
	delete m_pA;
	m_pA = 0;
	if (m_pMP) delete m_pMP;
}

//-----------------------------------------------------------------------------
void FEGlobalMatrix::Clear()
{ 
	if (m_pA) m_pA->Clear(); 
}

//-----------------------------------------------------------------------------
//! Start building the profile. That is delete the old profile (if there was one)
//! and create a new one. 
void FEGlobalMatrix::build_begin(int neq)
{
	if (m_pMP) delete m_pMP;
	m_pMP = new NumCore::SparseMatrixProfile(neq);
	m_nlm = 0;
}

//-----------------------------------------------------------------------------
//! Add an "element" to the matrix profile. The definition of an element is quite
//! general at this point. Any two or more degrees of freedom that are connected 
//! somehow are considered an element. This function takes one argument, namely a
//! list of degrees of freedom that are part of such an "element". Elements are 
//! first copied into a local LM array.  When this array is full it is flushed and
//! all elements in this array are added to the matrix profile.
void FEGlobalMatrix::build_add(vector<int>& lm)
{
	m_LM[m_nlm++] = lm;
	if (m_nlm >= MAX_LM_SIZE) build_flush();
}

//-----------------------------------------------------------------------------
//! Flush the LM array. The LM array stores a buffer of elements that have to be
//! added to the profile. When this buffer is full it needs to be flushed. This
//! flushin operation causes the actual update of the matrix profile.
void FEGlobalMatrix::build_flush()
{
	int i, j, n, *lm;

	// Since prescribed dofs have an equation number of < -1 we need to modify that
	// otherwise no storage will be allocated for these dofs (even not diagonal elements!).
	for (i=0; i<m_nlm; ++i)
	{
		n = m_LM[i].size();
		lm = &(m_LM[i])[0];
		for (j=0; j<n; ++j) if (lm[j] < -1) lm[j] = -lm[j]-2;
	}

	m_pMP->UpdateProfile(m_LM, m_nlm);
	m_nlm = 0;
}

//-----------------------------------------------------------------------------
//! This function makes sure the LM buffer is flushed and creates the actual
//! sparse matrix from the matrix profile.
void FEGlobalMatrix::build_end()
{
	if (m_nlm > 0) build_flush();
	m_pA->Create(*m_pMP);
}
