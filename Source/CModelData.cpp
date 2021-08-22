#include <algorithm>
#include <cmath>
#include <cstdio>
#include <QtWidgets/QMessageBox>
#include <QtXml/QDomNode>
#include "CFormatFloat.h"
#include "CFormula.h"
#include "CGlyph.h"
#include "CGuiMatrix.h"
#include "CModelData.h"
#include "CModelData.h"
#include "CNumerics.h"
#include "CXmlCreator.h"
#include "strutil.h"
#include "Util.h"

using std::string;

namespace
{
	class CGuiOptimizationInfo
	{
		bool& m_Loading;
	public:
		CGuiOptimizationInfo(bool& loading) : m_Loading(loading) { m_Loading = true; }
		~CGuiOptimizationInfo() { m_Loading = false; }
	};
	 
	const string g_FileVersionKanon("4");
	bool g_Ascending{};
	 
	bool ltName(const CModelData& x, const CModelData& y) {
		return (g_Ascending && 0 > cmpstri(x.name(), y.name()))
			|| (!g_Ascending && 0 < cmpstri(x.name(), y.name()));
	}
	bool ltNumCoord(const CModelData& x, const CModelData& y) {
		return (g_Ascending && x.numCoord() < y.numCoord())
			|| (!g_Ascending && x.numCoord() > y.numCoord());
	}
	bool ltDimension(const CModelData& x, const CModelData& y) {
		return (g_Ascending && x.critDim() < y.critDim())
			 || (!g_Ascending && x.critDim() > y.critDim());
	}
	bool ltNumField(const CModelData& x, const CModelData& y) {
		return (g_Ascending && x.numField() < y.numField())
			 || (!g_Ascending && x.numField() > y.numField());
	}
	bool ltOrder(const CModelData& x, const CModelData& y) {
		return (g_Ascending && x.modelOrder() < y.modelOrder())
			 || (!g_Ascending && x.modelOrder() > y.modelOrder());
	}
	bool ltNormalVect(const CModelData& x, const CModelData& y) {
		return (g_Ascending && 0 > cmpstri(x.printSortedNormalVector(), y.printSortedNormalVector()))
			 || (!g_Ascending && 0 < cmpstri(x.printSortedNormalVector(), y.printSortedNormalVector()));
	}
	bool ltTag(const CModelData& x, const CModelData& y) {
		return (g_Ascending && 0 > cmpstri(x.category(), y.category()))
		 || (!g_Ascending && 0 < cmpstri(x.category(), y.category()));
	}
	bool ltFlags(const CModelData& x, const CModelData& y) {
		return (g_Ascending && 0 > cmpstri(x.flags(), y.flags()))
			|| (!g_Ascending && 0 < cmpstri(x.flags(), y.flags()));
	}
}

bool CModelData::s_LoadingFile(false);
// Define column names and number of columns to display
DECLARE_TABLE(CModelData, "Models", " #Coord | #Field | Order | Crit. dim. | Normal vect. | Name | Tag | Category")

/* FUNCTION *******************************************************************/
/**
@return Singleton instance
*******************************************************************************/
CModelData& model()
{
	static CModelData s_Model(true);
	return s_Model;
}

/* METHOD *********************************************************************/
/**
  Ctor
*******************************************************************************/
CModelData::CModelData(bool isSingleton)
	: m_IsSingleton(isSingleton)
	, m_Dirty()
	, m_Dynamics()
	, m_ReactionDiffusion()
	, m_Statics()
	, m_QuantumFieldTheory()
	, m_CritDim(CNumerics::INVALID_CRITDIM)
	, m_Rank(-1)
	, m_Comment()
	, m_Name()
	, m_Pathname()
	, m_References()
	, m_UserTag()
	, m_Monomials()
	, m_Coords()
	, m_Fields()
	, m_NormalVect()
	, m_CanDim()
{
	insertDefaultCoordField();
}

void CModelData::insertDefaultCoordField()
{
#if 1
	if (m_Coords.empty())
	{
		m_Coords.push_back(CGlyphCoordField(x_, "D-dimensional coordinate"));
	}
	if (m_Fields.empty())
	{
		m_Fields.push_back(CGlyphCoordField(phi));
	}
#endif
}

/* METHOD *********************************************************************/
/**
  QAbstractTableModel method.
@param index:
@param  role:
*
@return Cell data for the list of modules.
*******************************************************************************/ 
QVariant CModelData::data(const QModelIndex& index, int role)
{
	if (index.isValid() && role == Qt::DisplayRole)
	{
		const CModelData& mod(at(index.row()));
		switch (index.column())
		{
		case colNumCoord:   return QString::number(mod.m_Coords.size());
		case colNumField:   return QString::number(mod.m_Fields.size());
		case colOrder:      return QString::number(mod.modelOrder());
		case colDimension:  return QString::number(mod.m_CritDim);
		case colNormalVect: return mod.printSortedNormalVector().c_str();
		case colName:       return mod.m_Name.c_str();
		case colTag:        return mod.m_UserTag.c_str();
		case colFlags:      return mod.flags().c_str();
		default:            return "??";
		}
	}
	return QVariant();
}

/* METHOD *********************************************************************/
/**
  Called from Qt.
*
@param :
*******************************************************************************/
void CModelData::sort(int column, Qt::SortOrder order)
{
	g_Ascending = order == Qt::AscendingOrder;
	if (column == -1)
	{
		column = s_SortColumn;
	}
	else
	{
		s_SortColumn = column;
	}
	std::vector <CModelData>::iterator it0(array().begin()), itn(array().end());
	switch (column)
	{
	case colDimension:  std::stable_sort(it0, itn, ltDimension); break;
	case colFlags:      std::stable_sort(it0, itn, ltFlags); break;
	case colName:       std::stable_sort(it0, itn, ltName); break;
	case colNormalVect: std::stable_sort(it0, itn, ltNormalVect); break;
	case colNumCoord:   std::stable_sort(it0, itn, ltNumCoord); break;
	case colNumField:   std::stable_sort(it0, itn, ltNumField); break;
	case colOrder:      std::stable_sort(it0, itn, ltOrder); break;
	case colTag:        std::stable_sort(it0, itn, ltTag); break;
	default: break;
  }
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CModelData::setDirty()
{
	m_Dirty = true;
}

size_t CModelData::modelOrder() const
{
	return m_Coords.size() + m_Fields.size();
}

size_t CModelData::numTerm() const
{
	if (m_IsSingleton)
	{
		return guiMatrix().numTerm();
	}
	return m_Monomials.size();
}

void CModelData::removeCoord(size_t cx)
{
	throwAssert("removeCoord()", cx < m_Coords.size());
	m_Coords.erase(m_Coords.begin() + cx);
	m_Dirty = true;
}
void CModelData::removeField(size_t fx)
{
	throwAssert("removeField()", fx < m_Fields.size());
	m_Fields.erase(m_Fields.begin() + fx);
	m_Dirty = true;
}

/* METHOD *********************************************************************/
/**
@return true for fields with a tilde.
*******************************************************************************/
bool CModelData::isResponseField(size_t cx) const
{
	throwAssert("isResponseField()", cx < modelOrder());
	return cx >= numCoord() && m_Fields[cx - numCoord()].hasTilde();
}

/* METHOD *********************************************************************/
/**
@param tx: Term index, counted from 0
@param cx: Coord-/Fieldindex, counted from 0
*
@return Exponent, in the cx == 0 case WITHOUT contribution proportinal to d
*******************************************************************************/
int CModelData::getExp(size_t tx, size_t cx) const
{
	if (m_IsSingleton)
	{	// Data may be edited
		return guiMatrix().getExp(tx, cx);
	}
	throwAssert("getExp()", tx < numTerm() && cx < modelOrder());
	return m_Monomials[tx].getExp(cx, *this);
}

int CModelData::getExpD(size_t tx) const
{	// cx == 0 case: contribution proportinal to d
	if (m_IsSingleton)
	{
		return guiMatrix().getExpD(tx);
	}
	throwAssert("getExpD()", tx < numTerm());
	return m_Monomials[tx].getExpD();
}

string CModelData::getCoordsFieldsList() const
{
	const string txtCoord(toString("coordinate%s", numCoord() == 1 ? "" : "s"));
	const string txtField(toString("field%s", numField() == 1 ? "" : "s"));
	 
	string ret("(" + txtCoord + "&nbsp;");
	for (size_t cx{1}; cx <= m_Coords.size(); cx++)
	{
		ret += toString("C%d", int(cx));
		ret += cx < m_Coords.size() ? "," : "";
	}
	ret += "&nbsp;, " + txtField + "&nbsp;";
	for (size_t fx{1}; fx <= m_Fields.size(); fx++)
	{
		ret += toString("F%d", int(fx));
		ret += fx < m_Fields.size() ? "," : ")";
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
@return Text identifying flags/attributes (only for display in list)
*******************************************************************************/
string CModelData::flags() const
{
	string ret;
	if (isStatics())
	{
		ret += "S";
	}
	if (isDynamics())
	{
		ret += "D";
	}
	if (isReactionDiffusion())
	{
		ret += "R";
	}
	if (isQuantumFieldTheory())
	{
		ret += "Q";
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
  Debug only
*******************************************************************************/
void CModelData::dump() const
{
	//fprintf(stderr, "--dump()--\n");
	//	for (const auto& coord : m_Coords.size()) { coord.dump(); }
	//	fprintf(stderr, "\n");
	for (size_t tx{}; tx < numTerm(); tx++)
	{
		for (size_t cx{}; cx < modelOrder(); cx++)
		{
			fprintf(stderr, "%2d ", getExp(tx, cx));
		}
		fprintf(stderr, "\n");
	}
}

/*******************************************************************************
  Returns canonical dimension of the coupling constant of a term.
@precondition determineCanonicalDimensions() called
@param dCanDim: [out] Factor for contribution linear in d
@param      tx: 0..numTerm() - 1, index of term.
                Use tx < modelOrder() for explicitely selected constant.
@return constCanDim
*******************************************************************************/
double CModelData::getDimensionOfCouplingConst(double* dCanDim, size_t tx) const
{
	*dCanDim = -1.0;
	if (m_CanDim.size() != 1 + numTerm())
	{	// m_CanDim[] must contain dimensions of coords/fields, then coupling constants.
		return -1.0;
	}
	if (tx < modelOrder())
	{	// Use explicitely selected coupling constant.
		tx = modelOrder();
	}
	else
	{	// Extra terms
		tx++;
	}
	*dCanDim = m_CanDim[tx].dVal;
	return m_CanDim[tx].constVal;
}

/*******************************************************************************
  Returns canonical dimension of the coupling constant of a term.
@precondition determineCanonicalDimensions() called
@param dCanDim: [out] Factor for contribution linear in d
@param      tx: 0..numTerm() - 1, index of term
@return Dimension as (unicode!) QString
*******************************************************************************/
QString CModelData::printDimensionOfCouplingConst(size_t tx) const
{
	if (m_CritDim <= CNumerics::INVALID_CRITDIM)
	{
		return "undefined";
	}
	double dCanDim;
	const double constCanDim(getDimensionOfCouplingConst(&dCanDim, tx));
	// Use: dim = constCanDim + (m_CritDim-eps)*dCanDim.
	const double dim(constCanDim + m_CritDim*dCanDim);
	return CFormatFloat::bilinear(dim, -dCanDim);
}
string CModelData::printHtmlDimensionOfCouplingConst(size_t tx) const
{
	if (m_CritDim <= CNumerics::INVALID_CRITDIM)
	{
		return "undefined";
	}
	double dCanDim;
	const double constCanDim(getDimensionOfCouplingConst(&dCanDim, tx));
	// Use: dim = constCanDim + (m_CritDim-eps)*dCanDim.
	const double dim(constCanDim + m_CritDim*dCanDim);
	return CFormatFloat::bilinear(dim, -dCanDim, eFormatHtml).toStdString();
}

/* METHOD *********************************************************************/
/**
@precondition determineCritDim() and determineCanonicalDimensions().
@param cx: Coord/field index. 0 corresponds to wave vector.
*
@return Canonical dimension of coord/field/coupling constant at critical dimension.
*******************************************************************************/
double CModelData::getDimensionAtCritDim(size_t cx) const
{
	throwAssert("getDimensionAtCritDim(cx)", cx < m_CanDim.size());
	return m_CanDim[cx].constVal + m_CanDim[cx].dVal * m_CritDim;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CModelData::setComment(const std::string& text)
{
	if (m_Comment != text)
	{
		m_Dirty = true;
		m_Comment = text;
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CModelData::setTag(const std::string& text)
{
	if (m_UserTag != text)
	{
		m_Dirty = true;
		m_UserTag = text;
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CModelData::setReferences(const std::string& text)
{
	if (m_References != text)
	{
		m_Dirty = true;
		m_References = text;
	}
}

/* METHOD *********************************************************************/
/**
  Permutes fields.
@param permutation: Destination in m_Fields.
*******************************************************************************/
bool CModelData::permuteFields(const vector<size_t>& permutation)
{
	vector<CGlyphCoordField> fieldsOld(m_Fields);
	throwAssert("permuteFields", permutation.size() == fieldsOld.size());
	for (size_t ix{}; ix < permutation.size(); ix++)
	{
		m_Fields[ix] = fieldsOld[permutation[ix]];
	}
#if 0
	fprintf(stderr, "Fields now are:\n");
	for (const auto& field : m_Fields)
	{
		fprintf(stderr, "\t%s\n", field.toStr().c_str());
	}
#endif
	// m_Monomials are original data, edited data are in CGuMatrix.
	guiMatrix().permuteFields(permutation);
	// Nothing to do in list of fields in GUI.
	return true;
}

/* METHOD *********************************************************************/
/**
@return Glyph for coordinate (including comment).
*******************************************************************************/
const CGlyphCoordField& CModelData::glyphCoord(size_t cx) const
{
	throwAssert("glyphCoord()", cx < m_Coords.size());
	return m_Coords[cx];
}

/* METHOD *********************************************************************/
/**
@return Glyph for field (including comment).
*******************************************************************************/
const CGlyphCoordField& CModelData::glyphField(size_t cx) const
{
	throwAssert("glyphField()", cx < m_Fields.size());
	return m_Fields[cx];
}

/* METHOD *********************************************************************/
/**
@return Glyph for coordinate or field (including comment).
*******************************************************************************/
const CGlyphCoordField& CModelData::glyphCoordField(size_t index, ECoordField type) const
{
	return type == eCoord ? glyphCoord(index) : glyphField(index);
}

/* METHOD *********************************************************************/
/**
  Accepts coordinate or field data.
*******************************************************************************/
void CModelData::setGlyphCoordField(size_t cx, ECoordField type, const CGlyphCoordField& glyph)
{
	if (type == eCoord)
	{
		if (cx < m_Coords.size())
		{
			m_Coords[cx] = glyph;
		}
		else
		{
			m_Coords.push_back(glyph);
		}
	}
	else
	{
		if (cx < m_Fields.size())
		{
			m_Fields[cx] = glyph;
		}
		else
		{
			m_Fields.push_back(glyph);
		}
	}
}

/* METHOD *********************************************************************/
/**
  Determines m_CritDim.
@return true when OK
*******************************************************************************/
bool CModelData::determineCritDim(double& critDim)
{
	if (CNumerics::determineCritDim(m_CritDim, *this))
	{
		critDim = m_CritDim;
		return true;
	}
	return false;
}

/* METHOD *********************************************************************/
/**
  Determines canonical dimensions of coordinates and fields and coupling constants.
@side_effects m_CritDim, m_CanDim updated;
@param  rxOfCoupling: Row index of interaction term
@return true on success
*******************************************************************************/
bool CModelData::determineCanonicalDimensions(size_t rxOfCoupling)
{
	m_CanDim.clear();
	// Determine rank first: evaluate() may throw.
	m_Rank = CNumerics::determineRank(*this);
	if (CNumerics::determineCritDim(m_CritDim, *this))
	{
		CNumerics::determineCanonicalDimensions(m_CritDim, m_CanDim, *this, rxOfCoupling);
		return true;
	}
	return false;
}

/* METHOD *********************************************************************/
/**
  Determines the normal vector (=Signature). The normal vector is a modelOrder()-
  dimensional vector, the components of which are the dimensions of the coordinates
  and fields (Multiplied with a common factor to get integer values).
  The 1st component denotes D-dimensional space.
@precondition determineCanonicalDimensions()
@side_effects m_NormalVect
*******************************************************************************/
void CModelData::determineNormalVector()
{
	m_NormalVect.clear();
	throwAssert("determineNormalVector() failed.\n"
		"You might select another term of the first " + toString(int(modelOrder())) + "\n"
		"terms of the Lagrangian as interaction and try again.",
		m_CanDim.size() == 1 + numTerm());
	double common{};
	double val{};
	for (size_t fx{}; fx < modelOrder(); fx++)
	{
		const double EPS{0.01};
		if (val > EPS)
		{
			common = val;
		}
		val = getDimensionAtCritDim(fx);
		if (fx == 0)
		{
			common = val;
		}
		else 
		{
			if (val > common + EPS)
			{
				for (; val > common + EPS && common > EPS;)
				{
					val -= common;
				}
				common = val;
			}
			else if (common > val + EPS && val > EPS)
			{
				for (; common > val + EPS && val > EPS;)
				{
					common -= val;
				}
			}
		}
	}
	for (size_t fx{}; fx < modelOrder(); fx++)
	{
		const double val{floor(0.49 + getDimensionAtCritDim(fx) / common)};
		m_NormalVect.push_back(int(val));
//		ret += toString("%d", int(val));
//		if (fx + 1 == numCoord())
//		{
//			ret += "; ";
//		}
//		else ret += fx + 1 < modelOrder() ? ", " : ")\r\n";
	}
}

/* METHOD *********************************************************************/
/**
@precondition determineNormalVector().
@return Normal vector as string, with sorted field components.
*******************************************************************************/
string CModelData::printSortedNormalVector() const
{
	string ret;
	for (size_t cx{}; cx < m_NormalVect.size(); cx++)
	{
		ret += toString(m_NormalVect[cx]);
		if (cx + 1 == numCoord())
		{
			ret += "; ";
		}
		else if (cx + 1 < modelOrder())
		{
			ret += ", ";
		}
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
  Returns formula and value for canonical dimension of coordinates/fields or of
the coupling constant.
@param fx: 0...modelOrder()-1.
@return Expression for canonical wave vector dimension.
*******************************************************************************/
string CModelData::printCanonicalDimension(int fx) const
{
	if (fx == 0)
	{
		return "1  (by definition)";
	}
	if (m_CanDim.size() != 1 + numTerm())
	{
		return "?";
	}
	const double DCanDim{m_CanDim[fx].dVal};
	double constCanDim{m_CanDim[fx].constVal};
	if (fabs(constCanDim) < 0.00001)
	{
		constCanDim = 0.0; // Rounding error
	}
	const double dim{constCanDim + m_CritDim * DCanDim};
	string ret;
	ret = CFormatFloat::bilinear(dim, -DCanDim, eFormatHtml).toStdString();
	return ret;
}

/* METHOD *********************************************************************/
/**
@return Critical dimension
*******************************************************************************/
string CModelData::printCriticalDimension() const
{
	if (m_CritDim <= CNumerics::INVALID_CRITDIM)
	{
		return "undefined";
	}
	CFormatFloat val("%2.3f");
	return val.get(m_CritDim);
}

/* METHOD *********************************************************************/
/**
@return Column caption
*******************************************************************************/
string CModelData::printColumnCaption(size_t cx) const
{
	throwAssert("printColumnCaption()", cx < modelOrder() && m_IsSingleton);
	if (cx < m_Coords.size())
	{
		return m_Coords[cx].comment();
	}
	return m_Fields[cx - m_Coords.size()].comment();
}

/* METHOD *********************************************************************/
/**
@return Row comment
*******************************************************************************/
string CModelData::printRowCaption(size_t tx) const
{
	throwAssert("printRowCaption()", tx < numTerm() && m_IsSingleton);
	return guiMatrix().comment(tx);
}

/* METHOD *********************************************************************/
/**
  Returns a string specifying whether the EXTRA term is relevant
@param tx: Zero-based index of (extra) term
@param formula: Display formula for canonical dimension
@return
*******************************************************************************/
string CModelData::printRelevanceExtra(size_t tx, bool formula) const
{
	double DCanDim;
	const double constCanDim{getDimensionOfCouplingConst(&DCanDim, tx)};
	const double dim{constCanDim + m_CritDim*DCanDim};
	string ret;
	if (formula)
	{
		return CFormatFloat::bilinear(dim, -DCanDim, eFormatHtml).toStdString();
	}
	else
	{
		if (dim > 0.001)
		{
			ret += "   relevant";
		}
		else if (dim < -0.001)
		{
			ret += "   irrelevant";
		}
		else ret += "   marginal";
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
  Saves the data.
@precondition m_Pathname is a valid pathname
@return true when OK
*******************************************************************************/
bool CModelData::makeClean()
{
	m_Dirty = false;
	if (m_IsSingleton)
	{
		guiMatrix().clear();
	}
	m_Pathname.clear();
	m_Monomials.clear();
	m_CritDim = CNumerics::INVALID_CRITDIM;
	m_Rank = -1;
	m_ReactionDiffusion = m_Statics = m_Dynamics = false;
	m_Comment.clear();
	m_Name.clear();
	m_References.clear();
	m_CanDim.clear();
	m_Fields.clear();
	if (m_IsSingleton && !m_Coords.empty())
	{	// Keep d-dimensional coordinate (gets removed before load)
		m_Coords.erase(m_Coords.begin() + 1, m_Coords.end());
	}
	else
	{
		m_Coords.clear();
	}
	return true;
}

/* METHOD *********************************************************************/
/**
  Loads data from file.
@param   pathname: Source
@param     errMsg: Error message or empty (Qt crashes when a messageBox is opened in an exception handler) !
*******************************************************************************/
void CModelData::loadData(const string& pathname, string& errMsg)
{
	try
	{
		errMsg.clear();
		CGuiOptimizationInfo loadGuard(s_LoadingFile);
		QDomDocument doc;
		xmlOpen(doc, pathname.c_str());
		QDomElement docElem(doc.documentElement());
		if (docElem.tagName() != "Kanon")
		{
			throwError("This is another XML file type, cannot be loaded.\n" + pathname);
		}
		m_Pathname = pathname;
		m_Coords.clear();
		m_Fields.clear();
		m_Monomials.clear();
		m_UserTag = xmlGetAttr(docElem, "userTag");
		m_Dynamics = xmlGetBool(docElem, "dynamics");
		m_ReactionDiffusion = xmlGetBool(docElem, "reactionDiffusion");
		m_Statics = xmlGetBool(docElem, "statics");
		m_QuantumFieldTheory = xmlGetBool(docElem, "qmField");
		 
		const string version(xmlRequireAttr(docElem, "version"));
		unsigned uFileVersionKanon{}, uFileVersionFile{};
		if (1 == sscanf(g_FileVersionKanon.c_str(), "%u", &uFileVersionKanon)
			&& 1 == sscanf(version.c_str(), "%u", &uFileVersionFile))
		{
			if (uFileVersionFile > uFileVersionKanon)
			{	// Throwing exceptions from an event handler is not supported in Qt (Qt5?). !
				// You must not let any exception whatsoever propagate through Qt code. !
				insertDefaultCoordField(); // Saves the day.
				const string errText("This Kanon version supports file version " + g_FileVersionKanon + ".\n"
					"File has version " + version + ".\n" + pathname);
				errMsg = errText;
			}
		}
		size_t numMonomial{};
		for (QDomNode node(docElem.firstChild()); !node.isNull(); node = node.nextSibling())
		{
			if (node.isElement())
			{
				QDomElement elem(node.toElement());
				const QString tagName(elem.tagName());
				if (tagName == "Name")
				{
					m_Name = xmlGetTextData(elem);
				}
				else if (tagName == "Comment")
				{
					m_Comment = xmlGetPcData(elem);
					//fprintf(stderr, "Comment %s\n", m_Comment.c_str());
				}
				else if (tagName == "Coordinate")
				{
					CGlyphCoordField coord(elem);
					m_Coords.push_back(coord);
				}
				else if (tagName == "Field")
				{
					CGlyphCoordField field(elem);
					m_Fields.push_back(field);
				}
				else if (tagName == "Monomial")
				{
					CFormula formula;
					formula.fromXml(elem);
					errMsg = formula.validate(*this);
					if (errMsg.empty())
					{
						m_Monomials.push_back(formula);
						if (m_IsSingleton)
						{	// Monomials kept in CGuiMatrix for edit
							guiMatrix().addRow(formula, numMonomial++);
						}
					}
				}
				else if (tagName == "References")
				{
					m_References = xmlGetPcData(elem);
					//fprintf(stderr, "References %s\n", m_References.c_str());
				}
			}
		}
		if (!m_IsSingleton)
		{	// Fill model list.
			Pk = createPk(array());
			push_back(*this);
		}
		insertDefaultCoordField();
	}
	catch (const std::exception& e)
	{
		insertDefaultCoordField();
		errMsg = string(e.what()) + ", " + pathname;
		fprintf(stderr, "ERR %s\n\n", e.what());/**/
	}
}

/* METHOD *********************************************************************/
/**
  Writes data to file.
@param    parent:
@param  pathname: Destination
*******************************************************************************/
bool CModelData::saveData(QWidget* parent, const string& pathname)
{
	if (!pathname.empty())
	{
		m_Pathname = pathname;
	}
	CXmlCreator xml(m_Pathname);
	if (!xml.isOpen())
	{
		msgBoxCritical("Cannot write to\n" + m_Pathname, parent);
		return false;
	}
	xml.write("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>");
	xml.addComment("Kanon");
	xml.addAttribSkipEmpty("userTag", m_UserTag);
	if (m_Statics)
	{
		xml.addAttrib("statics", CXmlCreator::bool2string(m_Statics));
	}
	if (m_Dynamics)
	{
		xml.addAttrib("dynamics", CXmlCreator::bool2string(m_Dynamics));
	}
	if (m_ReactionDiffusion)
	{
		xml.addAttrib("reactionDiffusion", CXmlCreator::bool2string(m_ReactionDiffusion));
	}
	if (m_QuantumFieldTheory)
	{
		xml.addAttrib("qmField", CXmlCreator::bool2string(m_QuantumFieldTheory));
	}
	xml.addAttrib("version", g_FileVersionKanon);
	xml.createTag("Kanon");
	xml.createChild("Name", m_Name);
	xml.createChildPcData("Comment", m_Comment);
	xml.createChildPcData("References", m_References);
	for (auto& coord : m_Coords)
	{
		coord.toXml(xml, "Coordinate");
	}
	for (auto& field : m_Fields)
	{
		field.toXml(xml, "Field");
	}
	if (m_IsSingleton)
	{	// From edit buffer
		guiMatrix().toXml(xml);
	}
	else
	{
		throwAssert("!m_IsSingleton", false);
		for (auto& monomial :  m_Monomials)
		{
			monomial.toXml(xml);
		}
	}
	xml.closeTag("Kanon");
	m_Dirty = false;
	return true;
}

