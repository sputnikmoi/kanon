#ifndef CMODELDATA_H
#define CMODELDATA_H

#include <vector>
#include "CFormula.h"
#include "CGlyph.h"
#include "CNumerics.h"
#include "CTable.h"

class QWidget;
struct SCoordFieldAttributes;

/* CLASS DECLARATION **********************************************************/
/**
  Data of a model.
  A singleton instance is used for editing a model. In this case the monomial
	data are kept/edited CGuiMatrix.
  A list of models may be kept (temporarily) in the static array inhereted from
	CTable<CModelData>
*******************************************************************************/
class CModelData : public CTable<CModelData>
{
	bool m_IsSingleton;                     // Instance used for edit (monomials kept in CGuiMatrix)
	bool m_Dirty;
	bool m_Dynamics;
	bool m_ReactionDiffusion;
	bool m_Statics;
	bool m_QuantumFieldTheory;
	double m_CritDim;
	int  m_Rank;
	static bool s_LoadingFile;              // Optimization: No Gui updates as long as true
	std::string m_Comment;
	std::string m_Name;
	std::string m_Pathname;
	std::string m_References;
	std::string m_UserTag;
	std::vector<CFormula> m_Monomials;      // The monomials (As read from file. Edited data are in CGuiMatrix)
	std::vector<CGlyphCoordField> m_Coords; // Coordinates
	std::vector<CGlyphCoordField> m_Fields; // Fields
	std::vector<int> m_NormalVect;          // Canonical dimensions at crritical dimension, normalized
	std::vector<SCanDim> m_CanDim;          // Canonical dimensions of coords/fields and coupling consts
public:
	enum
	{
		colNumCoord, colNumField, colOrder, colDimension, colNormalVect, colName, colTag, colFlags
	};
	CModelData(bool isSingleton = false);
	void insertDefaultCoordField();
	bool determineCritDim(double& critDim);
	bool determineCanonicalDimensions(size_t rxOfCoupling);
	bool dirty() const { return m_Dirty; }
	bool isDynamics() const { return m_Dynamics; }
	bool isQuantumFieldTheory() const { return m_QuantumFieldTheory; }
	bool isReactionDiffusion() const { return m_ReactionDiffusion; }
	bool isStatics() const { return m_Statics; }
	bool isResponseField(size_t fx) const;
	void setDynamics(bool val) { m_Dynamics = val; }
	void setQuantumFieldTheory(bool val) { m_QuantumFieldTheory = val; }
	void setReactionDiffusion(bool val) { m_ReactionDiffusion = val; }
	void setStatics(bool val) { m_Statics = val; }
	 
	void determineNormalVector();
	string printSortedNormalVector() const;
	string printCanonicalDimension(int fx) const;
	string printCriticalDimension() const;
	QString printDimensionOfCouplingConst(size_t tx) const;
	string printHtmlDimensionOfCouplingConst(size_t tx) const;
	string printColumnCaption(size_t cx) const;
	string printRowCaption(size_t tx) const;
	string printRelevanceExtra(size_t tx, bool formula) const;
	 
	bool makeClean();
	double critDim() const { return m_CritDim; }
	void setDirty();
	void loadData(const std::string& pathname, std::string& errMsg);
	bool saveData(QWidget* = nullptr, const std::string& pathname = "");
	std::string name() const { return m_Name; }
	size_t numTerm() const;
	size_t modelOrder() const;
	size_t numCoord() const { return m_Coords.size(); }
	size_t numField() const { return m_Fields.size(); }
	void removeCoord(size_t);
	void removeField(size_t);
	int getExp(size_t tx, size_t fx) const;
	int getExpD(size_t tx) const;
	 
	double getDimensionAtCritDim(size_t cx) const;
	void setName(const string& name) { m_Name = name; }
	 
	void dump() const; // Debug
	 
	std::string category() const { return m_UserTag; }
	std::string comment() const { return m_Comment; }
	std::string flags() const;
	std::string pathname() const { return m_Pathname; }
	std::string getCoordsFieldsList() const;
	 
	void setComment(const std::string&);
	void setTag(const std::string&);
	std::string references() const { return m_References; }
	void setReferences(const std::string&);
	void setPathname(const std::string& pathname) { m_Pathname = pathname; }
	bool permuteFields(const std::vector<size_t>& permutation);
	 
	const CGlyphCoordField& glyphCoord(size_t cx) const;
	const CGlyphCoordField& glyphField(size_t cx) const;
	 
	const CGlyphCoordField& glyphCoordField(size_t cx, ECoordField type) const;
	void setGlyphCoordField(size_t cx, ECoordField type, const CGlyphCoordField&);
	 
	static QVariant headerData(int col, Qt::Orientation orientation, int role)
	{	// QAbstractTableModel override
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		{
			return QVariant();
		}
		return columnCaption(col);
	}
	static QVariant data(const QModelIndex &index, int role);
	static void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
	static bool isLoadingFile() { return s_LoadingFile; }
private:
	double getDimensionOfCouplingConst(double* dCanDim, size_t tx) const;
};

// Singleton for editor
CModelData& model();
	 
#endif

