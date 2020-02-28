#ifndef NUMERICS_H
#define NUMERICS_H

class CModelData;

namespace math
{
	template <typename T> class matrix;
}
using math::matrix;

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
struct SCanDim       // Contains a canonical dimension
{
	double constVal; // Value for d = 0
	double dVal;     // Coefficient of contribution linear in d
};

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CNumerics
{
public:
	static const double INVALID_CRITDIM;
	static bool determineCritDim(double &critDim, const CModelData&);
	static int  determineRank(const CModelData&);
	static void determineCanonicalDimensions(double& critDim, std::vector<SCanDim>&, const CModelData&, int rxInteraction,
		matrix<double>* expMatrix = 0, matrix<double>* invMatrix = 0);
private:
	static void determineUselessTerms(CModelData&);
	static void getSpanningMatrix(matrix<double>& mtrx, const CModelData&);
};

#endif

