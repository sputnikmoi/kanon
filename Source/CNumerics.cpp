/*******************************************************************************
Matrix inversion
*******************************************************************************/
#include <complex>
#include <cstdio>
#include <iostream>
#include "CModelData.h"
#include "CNumerics.h"
#include "matrix.h"

using namespace math;

typedef std::complex<double> TComplex;
const double CNumerics::INVALID_CRITDIM{-99999.9f};

/*******************************************************************************
Debug
*******************************************************************************/
namespace
{
//#define NO_DBG
//	void dbgPrintMatrix(const char* text, matrix<TComplex>& m, bool isFloat)
//	{
//		if (text && isFloat && &m) {}
//#ifdef DBG
//		fprintf(stderr, "---%s\n", text);
//		for (unsigned rx{0}; rx < m.RowNo(); rx++)
//		{
//			for (unsigned cx{}; cx < m.ColNo(); cx++)
//			{
//				if (isFloat)
//				{
//					fprintf(stderr, "%2f ", std::real(m(rx, cx)));
//				}
//				else fprintf(stderr, "%2d ", int(m(rx, cx).real()));
//			}
//			fprintf(stderr, "\n");
//		}
//		fprintf(stderr, "\n");
//#endif
//	}
//	void dbgPrintMatrix(const char* text, matrix<TComplex>& m)
//	{
//		if (text && &m) {}
//#ifdef DBG
//		fprintf(stderr, "%s\n", text);
//		for (unsigned rx{}; rx < m.RowNo(); rx++)
//		{
//			for (unsigned cx{}; cx < m.ColNo(); cx++)
//			{
//				fprintf(stderr, "(%f, %f) ", m(rx, cx).real(), m(rx, cx).imag());
//			}
//			fprintf(stderr, "\n");
//		}
//		fprintf(stderr, "\n");
//#endif
//	}
//	void dbgPrintMatrix(const char* text, matrix<double>& m)
//	{
//		if (text && &m) {}
//#ifdef DBG
//		fprintf(stderr, "%s\n", text);
//		for (unsigned rx{}; rx < m.RowNo(); rx++)
//		{
//			for (unsigned cx{}; cx < m.ColNo(); cx++)
//			{
//				fprintf(stderr, "%lf ", m(rx, cx));
//			}
//			fprintf(stderr, "\n");
//		}
//		fprintf(stderr, "\n");
//#endif
//	}
}

/* METHOD *********************************************************************/
/**
  Determines the canonical dimensions
@param critDim: [out]
@param  canDim: [out] Canonical dimensions of coords/fields/coupling constants.
@param     mod: [in]
@param rxOfCoupling: 0-based index of term selected as coupling constant
@param expMatrix: [out/optional] Exponent matrix
@param invMatrix: [out/optional] Inverted matrix
*******************************************************************************/
void CNumerics::determineCanonicalDimensions(double& critDim, std::vector<SCanDim>& canDim,
	const CModelData& mod, int rxOfCoupling,
	matrix<double>* expMatrix, matrix<double>* invMatrix)
{
#if 1
	determineRank(mod);
#endif
	// Set output to default//
	critDim = INVALID_CRITDIM;
	canDim.clear();
	SCanDim dimFirstCoord;
	dimFirstCoord.constVal = 1.0;
	dimFirstCoord.dVal = 0.0;
	canDim.push_back(dimFirstCoord);
	// Create E1 matrix (WITHOUT exponents of 1st coordinate, which enter the rhsVector). //
	// E1 also hast 1 in all columns with index >= modelOrder (first 1 for rxOfCoupling). //
	matrix<TComplex> E1(mod.numTerm(), mod.numTerm());
	// Columns in which 1.0 is to be inserted for a coupling constant.
	unsigned couplingCol{unsigned(mod.modelOrder())};
	unsigned rx1{unsigned(rxOfCoupling + 1)};
	for (unsigned rx{1}; rx <= mod.numTerm(); rx++)
	{	// All terms (with extra terms)
		for (unsigned cx{1}; cx <= mod.numTerm(); cx++)
		{	// First column gets removed
			if (cx < mod.modelOrder())
			{	// Includes contribution from 1-dimensional integrals (and delta-function)
				double Exp{double(mod.getExp(rx - 1, cx))};
				E1(rx - 1, cx - 1) = Exp;
			}
			else if (cx == couplingCol)
			{	// Add one 1.0 for each coupling constant
				double Exp{};
				if (rx == rx1 || (rx > mod.modelOrder() && rx == cx))
				{	// Explicitely selected coupling (rx1) or extra term
					rx1 = UINT_MAX;
					Exp = 1.0;
					couplingCol++;
				}
				E1(rx - 1, cx - 1) = Exp;
			}
		}
		//fprintf(stderr, "\n");
	}
//	dbgPrintMatrix("(can) E1 = ", E1, false);
	// Generate the rhsVector (WITH KNOWN exponents of 1st coordinate)//
	matrix<TComplex> rhsVector(mod.numTerm(), 1);
	for (unsigned rx{}; rx < mod.numTerm(); rx++)
	{
		// real part is negative d-independent wave vector exponent,
		// Imaginary part is the contribution proportional to d.
		const int expD{mod.getExpD(rx)};
		rhsVector(rx, 0) = TComplex(-mod.getExp(rx, 0), -expD);
	}
	// Get canonical dimensions//
	matrix<TComplex> E2;
	E1.Invert(E2);
//	dbgPrintMatrix("1/E1 = ", E2, true);
	matrix<TComplex> canon(mod.numTerm(), 1);
	canon = E2.multiplied(rhsVector);
//	dbgPrintMatrix("Canonical dimensions", canon);/**/
	const TComplex u(canon(mod.modelOrder() - 1, 0));
	critDim = -std::real(u) / std::imag(u);
	// Fill output variables//
	for (unsigned ix{}; ix < canon.RowNo(); ix++)
	{	// Append nontrivial canonical dimensions
		SCanDim dim;
		dim.constVal = std::real(canon(ix, 0));
		dim.dVal     = std::imag(canon(ix, 0));
		canDim.push_back(dim);
	}
	//fprintf(stderr, "%s, critDim = %f\n", mod.name().c_str(), critDim);
	if (expMatrix && invMatrix)
	{	// Optional output
		expMatrix->SetSize(mod.numTerm(), mod.numTerm());
		invMatrix->SetSize(mod.numTerm(), mod.numTerm());
		for (unsigned rx{}; rx < mod.numTerm(); rx++)
		{
			// Set output matrix
			for (unsigned cx{}; cx < mod.numTerm(); cx++)
			{
				(*expMatrix)(rx, cx)    = std::real(E1(rx, cx));
				(*invMatrix)(rx, cx) = std::real(E2(rx, cx));
				//fprintf(stderr, " %f", real(E2(rx, cx)));
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
  Determines critical dimension, independent of term selected as coupling constant.
@param critDim: [out]
@param    mod: Model to examine
@return true on success
*******************************************************************************/
bool CNumerics::determineCritDim(double& critDim, const CModelData& mod)
{
	const size_t order{mod.modelOrder()};
	critDim = INVALID_CRITDIM;
	if (mod.numTerm() < unsigned(order))
	{
		return false;
	}
	matrix<double> mtrx(order, order);
	for (size_t rx{}; rx < order; rx++)
	{
		for (unsigned cx{}; cx < order; cx++)
		{
			const int val{mod.getExp(rx, cx)};
			mtrx(rx, cx) = val;
		}
	}
	const double e0{mtrx.Det()};
	for (size_t row {}; row < order; row++)
	{	// Contribution proportional to d
		const int expD{mod.getExpD(row)};
		mtrx(row, 0) = -expD;
	}
	//dbgPrintMatrix("determineCritDim mtrx = ", mtrx);
	const double e1{mtrx.Det()};
	if (fabs(e1) > 1E-8)
	{
		critDim = e0/e1;
		return true;
	}
	return false;
}

/* METHOD *********************************************************************/
/**
  Projects exponent points onto the plane k1 = 0.
@param mtrx: [out]
@param  mod: Model to examine
*******************************************************************************/
void CNumerics::getSpanningMatrix(matrix<double>& mtrx, const CModelData& mod)
{
	const size_t order{mod.modelOrder()};
	const size_t numTerm{mtrx.RowNo()};
	for (size_t rx{}; rx < numTerm; rx++)
	{
		for (size_t cx{}; cx < order; cx++)
		{
			if (cx == 0)
			{
				mtrx(rx, cx) = 0;
			}
			else
			{
				const int val{mod.getExp(rx, cx)};
				mtrx(rx, cx) = val;
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
  Determines the rank of subsets of terms, detects redundant terms
@param  mod: Model to examine
@return Rank
*******************************************************************************/
int CNumerics::determineRank(const CModelData& mod)
{
	const size_t order{mod.modelOrder()};
	size_t numRow{mod.numTerm()};
	if (numRow > order)
	{
		numRow = order;
	}
	matrix<double> mtrx(numRow, order);
	getSpanningMatrix(mtrx, mod);
	// Rank of the modelOrder-1 rows must be maximal (=dimension of hyperplane)
	const int rank{mtrx.Rank()};
	if (rank != int(order))
	{
		//////fprintf(stderr, "Rank mismatch %d/%d %s\n", rank, order, mod.name().c_str());
		//dbgPrintMatrix("Matrix", mtrx);
	}
	return rank;
}

/* METHOD *********************************************************************/
/**
  1st row (term) will always be used.
Subtract from all other rows the 1st row.
The rows 2...modelOrder then must span the hyperplane and have maximal rank.
If this is not the case, examine the rank of 2^n row subsets to
determine rows that don't contribute to the hyperplane.
@side_effects setTermRemovable()
@param  mod: Model to examine
*******************************************************************************/
void CNumerics::determineUselessTerms(CModelData& mod)
{
	const size_t order{mod.modelOrder()};
	matrix<double> mtrx(order, order);
	getSpanningMatrix(mtrx, mod);
	// Rank of the modelOrder-1 rows must be maximal (==dimension of hyperplane)
	const int rank0{mtrx.Rank()};
	if (rank0 >= int(order))
	{
		// OK, rank is maximal
		return;
	}
	// Rank might remain the same when a term is removed. This term then is superfluous//
	for (int skip{}; skip < int(mtrx.RowNo()); skip++)
	{
		matrix<double> m1(mtrx.RowNo() - 1, mtrx.ColNo()); // One row less
		int destRow{};
		for (int rx{}; rx < int(mtrx.RowNo()); rx++)
		{
			if (rx != skip)
			{
				for (int cx{}; cx < int(mtrx.ColNo()); cx++)
				{
					m1(destRow, cx) = mtrx(rx, cx);
				}
				destRow++;
			}
		}
		if (m1.Rank() == rank0)
		{
			fprintf(stderr, "Term %d does not contribute!\n", skip + 1);
		}
	}
	/*
	It seems that there are no more cases to consider.
	If a pair of terms depends linearily, then removing one of them does not decrease the rank.
	Etc....
	*/
}


