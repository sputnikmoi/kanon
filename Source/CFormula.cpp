
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtXml/QDomNode>
#include "CFormula.h"
#include "CGlyph.h"
#include "CXmlCreator.h"
#include "Util.h"

using std::string;

/* METHOD *********************************************************************/
/**
  Ctor
*******************************************************************************/
CFormula::CFormula()
	: m_CanHaveCursor()
	, m_CsrBlinkState(true)
	, m_HasFocus()
	, m_CsrPos(1)
	, m_Formula()
	, m_Comment()
{
}

/* METHOD *********************************************************************/
/**
  Dtor
*******************************************************************************/
CFormula::~CFormula()
{
	clear();
}

/* METHOD *********************************************************************/
/**
  Copy ctor
*******************************************************************************/
CFormula::CFormula(const CFormula& rhs)
	: m_CanHaveCursor(rhs.m_CanHaveCursor)
	, m_CsrBlinkState(true)
	, m_HasFocus()
	, m_CsrPos(1)
	, m_Formula()
	, m_Comment(rhs.m_Comment)
{
	for (size_t ix{}; ix < rhs.m_Formula.size(); ix++)
	{
		m_Formula.push_back(rhs.m_Formula[ix]->clone());
	}
}

/* METHOD *********************************************************************/
/**
  Assignment operator
*******************************************************************************/
CFormula& CFormula::operator=(const CFormula& rhs)
{
	if (this != &rhs)
	{
		m_CanHaveCursor = rhs.m_CanHaveCursor;
		m_CsrBlinkState = rhs.m_CsrBlinkState;
		m_HasFocus = rhs.m_HasFocus;
		m_CsrPos = rhs.m_CsrPos;
		m_Comment = rhs.m_Comment;
		m_Formula.clear();
		for (size_t ix{}; ix < rhs.m_Formula.size(); ix++)
		{
			m_Formula.push_back(rhs.m_Formula[ix]->clone());
		}
	}
	return *this;
}

/* METHOD *********************************************************************/
/**
  Deserialization of a model monomial.
@param elem:
*******************************************************************************/
void CFormula::fromXml(QDomElement& elem)
{
	clear();
	m_Comment = xmlGetAttr(elem, "comment");
	for (QDomNode node(elem.firstChild()); !node.isNull(); node = node.nextSibling())
	{
		if (node.isElement())
		{
			QDomElement fact(node.toElement());
			if (fact.tagName() == "Factor")
			{
				const string type(xmlRequireAttr(fact, "type"));
				if (type == "neutral")
				{
					add(new CGlyphNeutral(fact));
				}
				else if (type == "coord")
				{
					add(new CGlyphCoordinate(fact));
				}
				else if (type == "field")
				{
					add(new CGlyphField(fact));
				}
				else
				{
					throwAssert("Invalid Factor type " +  type, false);
				}
			}
		}
	}
	allowCursor(true); // If it has focus
}

/* METHOD *********************************************************************/
/**
  Converts monomial to Xml.
@param	   xml: Destination
@param comment: Optional comment
*******************************************************************************/
void CFormula::toXml(CXmlCreator& xml) const
{
	xml.addAttribSkipEmpty("comment", m_Comment);
	xml.createTag("Monomial");
	for (size_t ix{}; ix < m_Formula.size(); ix++)
	{
		m_Formula[ix]->toXml(xml);
	}
	xml.closeTag("Monomial");
}

/* METHOD *********************************************************************/
/**
@param toggleCsrState: Set to true to toggle cursor (->blinking cursor).
*******************************************************************************/
void CFormula::toggleCsrState(bool toggleCsrState)
{
	if (toggleCsrState)
	{
		m_CsrBlinkState = !m_CsrBlinkState;
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CFormula::add(CGlyphBase* glyph)
{
	m_Formula.push_back(glyph);
}
void CFormula::clear()
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		delete m_Formula.at(gx);
	}
	m_Formula.clear();
	m_CsrPos = 1;
}

/* METHOD *********************************************************************/
/**
  When field or operator was dropped onto the line.
*******************************************************************************/
void CFormula::setCsrToEnd()
{
	m_CsrPos = m_Formula.size();
}

/* METHOD *********************************************************************/
/**
@param p:
*******************************************************************************/
int CFormula::paint(QPainter& p)
{
	const QFontMetrics fmGlobal(p.font());
	int xPos{};
	for (size_t ix{}; ix < m_Formula.size(); ix++)
	{
		const bool bCsr{m_CanHaveCursor && ix+1 == m_CsrPos && m_HasFocus};
		if (bCsr)
		{
			p.setPen(Qt::red);
		}
		m_Formula[ix]->paint(p, xPos);
		if (bCsr)
		{
			p.setPen(Qt::DashLine);
			if (m_CsrBlinkState)
			{
				p.drawLine(xPos, fmGlobal.descent(), xPos, -fmGlobal.ascent());
			}
			xPos++;
		}
		p.setPen(Qt::black);
	}
	return xPos;
}

/* METHOD *********************************************************************/
/**
@param ixColumn: Matrix column (Coordinates, then fields)
*
@return Exponent at (row, ixColumn), where row corresponds to formula.
*******************************************************************************/
int CFormula::getExp(size_t ixColumn, const CModelData& mod) const
{
	int ret{};
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		ret += m_Formula[gx]->exponent(ixColumn, mod);
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
@return coefficient of the contribution proportional to d of the exponent
  of the 1st coordinate (Integrals). In most cases -1 because of the
  d-dimensional integral.
*******************************************************************************/
int CFormula::getExpD() const
{
	int ret{};
	for (auto& formula : m_Formula)
	{
		ret += formula->exponentD();
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
  Event handler.
@param dirty: [out] Formula changed
@param   key: Event
@param   ctl: CTL pressed
*******************************************************************************/
bool CFormula::consumeKey(bool& dirty, int key, bool ctl)
{
	dirty = false;
	bool consumed(true);
	if (m_Formula.empty())
	{
		return false;
	}
	if (m_CsrPos > m_Formula.size())
	{
		m_CsrPos = m_Formula.size();
	}
	if (ctl)
	{
		if (key == Qt::Key_Up)
		{
			dirty = true;
			m_Formula.at(m_CsrPos - 1)->incExponent(1);
		}
		else if (key == Qt::Key_Down)
		{
			dirty = true;
			m_Formula.at(m_CsrPos - 1)->incExponent(-1);
		}
		else if (key == Qt::Key_Left && m_CsrPos >= 2)
		{
			dirty = true;
			m_CsrPos--;
			std::swap(m_Formula[m_CsrPos], m_Formula[m_CsrPos - 1]);
		}
		else if (key == Qt::Key_Right && m_CsrPos < m_Formula.size())
		{
			dirty = true;
			std::swap(m_Formula[m_CsrPos - 1], m_Formula[m_CsrPos]);
			m_CsrPos++;
		}
	}
	else
	{
		if (key == Qt::Key_Left)
		{
			consumed = m_CsrPos >= 1;
			if (consumed)
			{
				m_CsrPos--;
			}
		}
		else if (key == Qt::Key_Right)
		{
			consumed = m_CsrPos < m_Formula.size();
			if (consumed)
			{
				m_CsrPos++;
			}
		}
		if (key == Qt::Key_Home)
		{
			m_CsrPos = 1;
		}
		if (key == Qt::Key_End)
		{
			m_CsrPos = m_Formula.size();
		}
		else if (key == Qt::Key_Backspace)
		{
			if (m_CsrPos > 0)
			{
				dirty = true;
				m_Formula.erase(m_Formula.begin() + m_CsrPos - 1);
				if (m_CsrPos >= 1)
				{
					m_CsrPos--;
				}
			}
		}
		else if (key == Qt::Key_Delete)
		{
			if (m_CsrPos < m_Formula.size())
			{
				dirty = true;
				m_Formula.erase(m_Formula.begin() + m_CsrPos);
				if (m_CsrPos > m_Formula.size())
				{
					m_CsrPos = m_Formula.size();
				}
			}
		}
	}
	return consumed;
}

/* METHOD *********************************************************************/
/**
@return true if coordinate with given index found
*******************************************************************************/
bool CFormula::containsCoord(size_t index) const
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		if (const CGlyphCoordinate* glyph{dynamic_cast<const CGlyphCoordinate*>(m_Formula[gx])})
		{
			if (glyph->coordIndex() == int(index))
			{
				return true;
			}
		}
	}
	return false;
}

/* METHOD *********************************************************************/
/**
@return true if coordinate with given index found
*******************************************************************************/
bool CFormula::containsField(size_t index) const
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		if (const CGlyphField* glyph{dynamic_cast<const CGlyphField*>(m_Formula[gx])})
		{
			if (glyph->fieldIndex() == int(index))
			{
				return true;
			}
		}
	}
	return false;
}

/* METHOD *********************************************************************/
/**
@param index: 0-based
*******************************************************************************/
void CFormula::removeCoord(size_t index)
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		if (CGlyphCoordinate* glyph{dynamic_cast<CGlyphCoordinate*>(m_Formula[gx])})
		{
			const int cx{glyph->coordIndex()};
			if (cx == int(index))
			{
				delete m_Formula.at(gx);
				m_Formula.erase(m_Formula.begin() + gx);
				gx--;
			}
			else if (cx > int(index))
			{
				glyph->decIndex();
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CFormula::replaceCoord(size_t ixOld, size_t /*ixNew*/)
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{	// For operators: Replace reference to coordinate with default coordinate.
		if (CGlyphCoordinate* glyph{dynamic_cast<CGlyphCoordinate*>(m_Formula[gx])})
		{
			if (int(ixOld) == glyph->coordIndex())
			{
				glyph->setIndexToDefault();
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
@param index: 0-based
*******************************************************************************/
void CFormula::removeField(size_t index)
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		if (CGlyphField* glyph{dynamic_cast<CGlyphField*>(m_Formula[gx])})
		{
			const int fx{glyph->fieldIndex()};
			if (fx == int(index))
			{
				delete m_Formula.at(gx);
				m_Formula.erase(m_Formula.begin() + gx);
				gx--;
			}
			else if (fx > int(index))
			{
				glyph->decIndex();
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
  After drag/drop
*******************************************************************************/
void CFormula::permuteFields(const vector<size_t>& permutation)
{
	for (size_t gx{}; gx < m_Formula.size(); gx++)
	{
		if (CGlyphField* glyph{dynamic_cast<CGlyphField*>(m_Formula[gx])})
		{
			const int fx{glyph->fieldIndex()};
			for (size_t px{}; px < permutation.size(); px++)
			{
				if (int(permutation[px]) == fx)
				{
					glyph->setIndex(px);
					break;
				}
			}
		}
	}
}

