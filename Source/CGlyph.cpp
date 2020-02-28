
#include <cassert>
#include <cstdio>
#include <map>
#include <string>
#include <QtXml/QDomNode>
#include <QPainter>
#include "strutil.h"
#include "CGlyph.h"
#include "CModelData.h"
#include "CXmlCreator.h"
#include "Util.h"

using std::map;
using std::string;

namespace
{
	typedef std::map<ESymbol, const char*> TSymbMap;
	TSymbMap g_Symbol2Text;
	string symbol2string(ESymbol symb)
	{
		TSymbMap::iterator iter(g_Symbol2Text.find(symb));
		if (iter == g_Symbol2Text.end())
		{
			throwAssert("Unknown symbol", false);
		}
		return iter->second;
	}
	ESymbol string2symbol(const string& text)
	{
		for (TSymbMap::iterator it(g_Symbol2Text.begin()); it != g_Symbol2Text.end(); it++)
		{
			if (it->second == text)
			{
				return it->first;
			}
		}
		return none;
	}
	 
	QFont getFont(QPainter& p)
	{
		QFont font(p.font());
		font.setPointSize(13);
		return font;
	}
	 
	/* METHOD *********************************************************************/
	/**
	  Paints symbol with optional exponent
	@param        p:
	@param     xPos:
	@param     symb:
	@param exponent: optionally empty
	*******************************************************************************/
	void paintSymbol(QPainter& p, int& xPos, ESymbol symb, const QString& exponent, const QChar& chSuff,
		bool bold, bool tilde, bool primed)
	{
		const QFontMetrics fmGlobal(getFont(p));
		const int x0{xPos};
		CGlyphNeutral n1(symb, bold);
		n1.paint(p, xPos);
		const int prevTop{fmGlobal.boundingRect(QChar(symb)).top()};
		if (tilde)
		{
			p.save();
			p.setFont(getFont(p));
			const QChar chTilde('~');
			const QRect rectTilde(fmGlobal.boundingRect(chTilde));
			const int vGap{3}; // Between tilde and symbol
			p.drawText((x0 + xPos - rectTilde.width())/2, prevTop - rectTilde.bottom() - vGap, chTilde);
			p.restore();
		}
		if (!chSuff.isNull() || !exponent.isEmpty())
		{
			p.save();
			QFont font(getFont(p));
			font.setPointSize(int(font.pointSize()*0.7));
			p.setFont(font);
			const QFontMetrics fmSmall(getFont(p));
			int x1{xPos}, x2{xPos};
			if (!chSuff.isNull())
			{
				p.drawText(xPos, fmGlobal.descent(), chSuff);
				x1 += fmSmall.horizontalAdvance(chSuff);
			}
			if (!exponent.isEmpty())
			{
				const QRect rect(fmSmall.boundingRect(QChar('1')));
				p.drawText(xPos, fmGlobal.descent() - 1.2*rect.height(), exponent);
				x2 += fmSmall.horizontalAdvance(exponent);
			}
			xPos = qMax(x1, x2);
			p.restore();
		}
		if (primed)
		{
			p.setFont(getFont(p));
			CGlyphNeutral n1(ESymbol('\''));
			n1.paint(p, xPos);
		}
	}
}

/* METHOD *********************************************************************/
/**
@return true if symbol is associated with some coordinate (carries dimension).
*******************************************************************************/
bool hasCoordinate(ESymbol symb)
{
	return symb==integral || symb==nabla || symb==partial || symb==delta;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGlyphBase::initializeSymbolTable()
{
	g_Symbol2Text[none] = "";
	g_Symbol2Text[a_] = "a";
	g_Symbol2Text[A_] = "A";
	g_Symbol2Text[b_] = "b";
	g_Symbol2Text[B_] = "B";
	g_Symbol2Text[c_] = "c";
	g_Symbol2Text[d_] = "d";
	g_Symbol2Text[f_] = "f";
	g_Symbol2Text[F_] = "F";
	g_Symbol2Text[h_] = "h";
	g_Symbol2Text[I_] = "I";
	g_Symbol2Text[j_] = "j";
	g_Symbol2Text[l_] = "l";
	g_Symbol2Text[m_] = "m";
	g_Symbol2Text[M_] = "M";
	g_Symbol2Text[n_] = "n";
	g_Symbol2Text[N_] = "N";
	g_Symbol2Text[p_] = "p";
	g_Symbol2Text[q_] = "q";
	g_Symbol2Text[r_] = "r";
	g_Symbol2Text[s_] = "s";
	g_Symbol2Text[t_] = "t";
	g_Symbol2Text[u_] = "u";
	g_Symbol2Text[v_] = "v";
	g_Symbol2Text[w_] = "w";
	g_Symbol2Text[x_] = "x";
	g_Symbol2Text[y_] = "y";
	g_Symbol2Text[z_] = "z";
	g_Symbol2Text[integral] = "Integral";
	g_Symbol2Text[delta] = "delta";
	g_Symbol2Text[Delta] = "Delta";
	g_Symbol2Text[sum] = "sum";
	g_Symbol2Text[nabla] = "nabla";
	g_Symbol2Text[partial] = "part";
	// Binary and/or neutral//
	g_Symbol2Text[bra] = "(";
	g_Symbol2Text[ket] = ")";
	g_Symbol2Text[dot] = ".";
	g_Symbol2Text[minus_] = "minus";
	g_Symbol2Text[dgamma] = "gamma";
	g_Symbol2Text[plus_] = "plus";
	g_Symbol2Text[otimes] = "otimes";
	g_Symbol2Text[times] = "times";
	g_Symbol2Text[bullet] = "bullet";
	// Greek
	g_Symbol2Text[alpha] = "alpha";
	g_Symbol2Text[beta] = "beta";
	g_Symbol2Text[chi] = "chi";
	g_Symbol2Text[epsilon] = "eps";
	g_Symbol2Text[zeta] = "zeta";
	g_Symbol2Text[eta] = "eta";
	g_Symbol2Text[theta] = "theta";
	g_Symbol2Text[kappa] = "kappa";
	g_Symbol2Text[lambda] = "lambda";
	g_Symbol2Text[mu] = "mu";
	g_Symbol2Text[xi] = "xi";
	g_Symbol2Text[pi] = "pi";
	g_Symbol2Text[rho] = "rho";
	g_Symbol2Text[sigma] = "sigma";
	g_Symbol2Text[phi] = "phi";
	g_Symbol2Text[Phi] = "Phi";
	g_Symbol2Text[psi] = "psi";
	g_Symbol2Text[Psi] = "Psi";
}

/* METHOD *********************************************************************/
/**
@param coordIndex: Specifies the coordinate symbol proper (via indey in model).
@param       symb: Special additional symbol (integral, nabla, partial, delta)
@param   exponent: Exponent
*******************************************************************************/
CGlyphCoordinate::CGlyphCoordinate(int coordIndex, ESymbol symb, int exponent)
	: CGlyphBase()
	, m_CoordIndex(coordIndex)
	, m_Symb(symb)
	, m_Exponent(exponent)
{
}
/* METHOD *********************************************************************/
/**
*******************************************************************************/
CGlyphCoordinate::CGlyphCoordinate(QDomElement& elem)
	: CGlyphBase()
	, m_CoordIndex()
	, m_Symb(none)
	, m_Exponent(1)
{
	string text(xmlRequireAttr(elem, "index"));
	throwAssert("Invalid field index ", 1 == sscanf(text.c_str(), "%u", &m_CoordIndex));
	text = xmlGetAttr(elem, "symbol");
	if (!text.empty())
	{
		m_Symb = string2symbol(text);
		throwAssert("Unknown symbol '" + text + "'", m_Symb != none);
	}
	text = xmlGetAttr(elem, "exponent");
	if (!text.empty())
	{
		sscanf(text.c_str(), "%d", &m_Exponent);
	}
}

/* METHOD *********************************************************************/
/**
  For calculations, not for display.
@param ixColumn: Matrix column
*
@return Coordinate exponent, without contributions proportional to d.
*******************************************************************************/
int CGlyphCoordinate::exponent(size_t ixColumn, const CModelData& mod) const
{
	if (ixColumn >= mod.numCoord() || m_CoordIndex != int(ixColumn))
	{
		return 0;
	}
	if (m_Symb == integral)
	{
		if (ixColumn != 0)
		{	// The integral over the 1st coordinate is d-dimensional
			return -1;
		}
	}
	else if (m_Symb == delta)
	{
		if (ixColumn != 0)
		{	// A delta-function with 1st coordinate is d-dimensional
			return 1;
		}
	}
	else if (m_Symb == nabla || m_Symb == partial)
	{
		return m_Exponent;
	}
	return 0;
}

/* METHOD *********************************************************************/
/**
@return Coordinate exponent: Contribution proportional to d.
*******************************************************************************/
int CGlyphCoordinate::exponentD() const
{
	if (m_CoordIndex != 0)
	{	// Not 1st (d-dimensional) coordinate
		return 0;
	}
	if (m_Symb == integral)
	{
		return -1;
	}
	else if (m_Symb == delta)
	{
		return 1;
	}
	return 0;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGlyphCoordinate::paint(QPainter& p, int& xPos) const
{
	const QFontMetrics fmGlobal(getFont(p));
	const SCoordFieldAttributes attrs(model().glyphCoord(m_CoordIndex).attrs());
	switch (m_Symb)
	{
	case integral:
		{
			p.save();
			QFont font(getFont(p));
			font.setWeight(QFont::Light);
			font.setPointSize(int(font.pointSize()*1.2));
			p.setFont(font);
			CGlyphNeutral n1(integral, false);
 			n1.paint(p, xPos);
			p.restore();
			xPos -= 3;
			const QString exponent(m_CoordIndex == 0 ? "d" : "");
			paintSymbol(p, xPos, d_, exponent, QChar(), false, false, false);
			const QChar chSuff(attrs.m_Suffix < 0 ? 0 : attrs.m_Suffix + '0');
			paintSymbol(p, xPos, attrs.m_Symb, "", chSuff, false, attrs.m_Tilde, attrs.m_Primed);
			xPos += qMax(2, fmGlobal.horizontalAdvance(QChar(d_))/6);
		}
		break;
	case delta:
		{
			const QString exponent(m_CoordIndex == 0 ? "d" : "");
			paintSymbol(p, xPos, delta, exponent, QChar(), false, false, false);
			CGlyphNeutral n1(bra, false);
 			n1.paint(p, xPos);
			const QChar chSuff(attrs.m_Suffix < 0 ? 0 : attrs.m_Suffix + '0');
			paintSymbol(p, xPos, attrs.m_Symb, "", chSuff, false, attrs.m_Tilde, attrs.m_Primed);
			CGlyphNeutral n2(ket, false);
 			n2.paint(p, xPos);
		}
		break;
	case nabla:
	case partial:
		{
			const QString exponent(m_Exponent == 1 ? "" : QString::number(m_Exponent));
			QChar chSuff(m_CoordIndex == 0 ? 0 : attrs.m_Symb);
			paintSymbol(p, xPos, m_Symb, exponent, chSuff, false, false, attrs.m_Primed);
		}
		break;
	default: break;
	}
}

/* METHOD *********************************************************************/
/**
@return Coordinate attributes (as a monomial factor).
*******************************************************************************/
void CGlyphCoordinate::toXml(CXmlCreator& xml) const
{
	xml.addAttrib("type", "coord");
	xml.addAttrib("index", toString(m_CoordIndex));
	xml.addAttrib("symbol", symbol2string(m_Symb));
	xml.addAttribSkipEmpty("exponent", (m_Exponent == 0 || m_Exponent == 1) ? "" : toString(m_Exponent));
	xml.createClose("Factor");
	// m_ExponentCh;
	// m_Suffix;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CGlyphField::CGlyphField(int fieldIndex, int exponent)
	: CGlyphBase()
	, m_FieldIndex(fieldIndex)
	, m_Exponent(exponent)
{}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CGlyphField::CGlyphField(QDomElement& elem)
	: CGlyphBase()
	, m_FieldIndex(0)
	, m_Exponent(1)
{
	string text(xmlRequireAttr(elem, "index"));
	throwAssert("Invalid field index ", 1 == sscanf(text.c_str(), "%u", &m_FieldIndex));
	text = xmlGetAttr(elem, "exponent");
	if (!text.empty())
	{
		sscanf(text.c_str(), "%d", &m_Exponent);
	}
	//const string symbol(xmlGetAttr(fact, "symbol"));
	//const string exponent(xmlGetAttr(fact, "exponent"));
}

/* METHOD *********************************************************************/
/**
  For calculations, not for display.
@param ixColumn: Matrix column
*
@return Field exponent
*******************************************************************************/
int CGlyphField::exponent(size_t ixColumn, const CModelData& mod) const
{
	if (ixColumn < mod.numCoord())
	{
		return 0;
	}
	const int ixField{int(ixColumn - mod.numCoord())};
	if (m_FieldIndex == ixField)
	{
		return m_Exponent;
	}
	return 0;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGlyphField::paint(QPainter& p, int& xPos) const
{
	const SCoordFieldAttributes attrs(model().glyphField(m_FieldIndex).attrs());
	const QString exponent(m_Exponent == 1 ? "" : QString::number(m_Exponent));
	const QChar chSuff(attrs.m_Suffix < 0 ? 0 : attrs.m_Suffix + '0');
	paintSymbol(p, xPos, attrs.m_Symb, exponent, chSuff, attrs.m_Bold, attrs.m_Tilde, attrs.m_Primed);
}

/* METHOD *********************************************************************/
/**
@return Field attributes (as a monomial factor).
*******************************************************************************/
void CGlyphField::toXml(CXmlCreator& xml) const
{
	xml.addAttrib("type", "field");
	xml.addAttrib("index", toString(m_FieldIndex));
	xml.addAttribSkipEmpty("exponent", m_Exponent == 1 ? "" : toString(m_Exponent));
	xml.createClose("Factor");
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CGlyphCoordField::CGlyphCoordField(ESymbol symb, const string& comment)
	: CGlyphBase()
	, m_Comment(comment)
	, m_Attributes(symb)
{}

/* METHOD *********************************************************************/
/**
  Creates instance from serialized text (see CGlyphCoordField::toXml().
@param elem: Xml node.
*******************************************************************************/
CGlyphCoordField::CGlyphCoordField(QDomElement& elem)
	: CGlyphBase()
	, m_Comment(xmlGetAttr(elem, "comment"))
	, m_Attributes(x_)
{
	m_Attributes.m_Symb = string2symbol(xmlGetAttr(elem, "symbol"));
	m_Attributes.m_Bold = xmlGetBool(elem, "bold");
	m_Attributes.m_Tilde = xmlGetBool(elem, "tilde");
	m_Attributes.m_Primed = xmlGetBool(elem, "prime");
	const string suffix(xmlGetAttr(elem, "suffix"));
	int ival;
	if (!suffix.empty() && 1 == sscanf(suffix.c_str(), "%d", &ival))
	{
		m_Attributes.m_Suffix = ival;
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGlyphCoordField::paint(QPainter& p, int& xPos) const
{
	const QChar chSuff(m_Attributes.m_Suffix < 0 ? 0 : m_Attributes.m_Suffix + '0');
	paintSymbol(p, xPos, m_Attributes.m_Symb, "", chSuff, m_Attributes.m_Bold, m_Attributes.m_Tilde, m_Attributes.m_Primed);
}

/* METHOD *********************************************************************/
/**
  Serializes coordinate/field definition
@param xml: Destination
*******************************************************************************/
void CGlyphCoordField::toXml(CXmlCreator& xml, const string& tag) const
{
	xml.addAttrib("symbol", symbol2string(m_Attributes.m_Symb));
	xml.addAttribSkipEmpty("bold", m_Attributes.m_Bold ? "true" : "");
	xml.addAttribSkipEmpty("tilde", m_Attributes.m_Tilde ? "true" : "");
	xml.addAttribSkipEmpty("prime", m_Attributes.m_Primed ? "true" : "");
	xml.addAttribSkipEmpty("suffix", m_Attributes.m_Suffix >= 0 ? toString(m_Attributes.m_Suffix) : "");
	xml.addAttribSkipEmpty("comment", m_Comment);
	xml.createClose(tag);
}

/* METHOD *********************************************************************/
/**
@return string for debug
*******************************************************************************/
string CGlyphCoordField::toStr() const
{
	string str(symbol2string(m_Attributes.m_Symb));
	str += toString("_%d", m_Attributes.m_Suffix);
	if (m_Attributes.m_Tilde)
	{
		str += "~";
	}
	return str;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CGlyphNeutral::CGlyphNeutral(ESymbol symb, bool bold)
	: CGlyphBase()
	, m_Symb(symb)
	, m_Bold(bold)
{}
/* METHOD *********************************************************************/
/**
*******************************************************************************/
CGlyphNeutral::CGlyphNeutral(QDomElement& elem)
	: CGlyphBase()
	, m_Symb(string2symbol(xmlGetAttr(elem, "symbol")))
	, m_Bold(xmlGetBool(elem, "bold"))
{}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGlyphNeutral::paint(QPainter& p, int& xPos) const
{
	p.save();
	QFont font(getFont(p));
	p.setFont(font);
	const QFontMetrics fmGlobal(font);
	if (m_Bold)
	{
		font.setWeight(QFont::DemiBold);
		p.setFont(font);
	}
	if (m_Symb == nabla)
	{	// Workaround for windows 10, QTBUG-48945
		xPos++;
		const int top{fmGlobal.boundingRect(QChar('x')).top()};
		const int w1{int(fmGlobal.horizontalAdvance('x')*0.9)};
		const int x1{xPos + w1/2};
		QPen pen(p.pen());
		pen.setWidth(2);
		p.setPen(pen);
		p.drawLine(x1, -1, xPos, top);
		p.drawLine(x1, -1, xPos + w1, top);
		p.drawLine(xPos, top, xPos + w1, top);
		xPos += w1 + 1;
	}
#if 0
	else if (m_Symb == otimes)
	{	// Workaround for windows 10, QTBUG-48945
		const QChar ch(static_cast<ushort>(times));
		xPos++;
		p.drawText(xPos,0, ch);
		const QRect rect(fmGlobal.boundingRect(QChar(times)));
		p.drawEllipse(rect.translated(xPos-1, 0));
		xPos += fmGlobal.horizontalAdvance(ch);
	}
#endif
	else
	{
		const QChar ch(static_cast<ushort>(m_Symb));
		p.drawText(xPos,0, ch);
		xPos += fmGlobal.horizontalAdvance(ch);
		if (m_Bold)
		{
			xPos += 2;
		}
	}
	p.restore();
}

/* METHOD *********************************************************************/
/**
@return TeX for the glyph
*******************************************************************************/
void CGlyphNeutral::toXml(CXmlCreator& xml) const
{
	xml.addAttrib("type", "neutral");
	xml.addAttrib("symbol", symbol2string(m_Symb));
	xml.addAttribSkipEmpty("bold", m_Bold ? "true" : "");
	xml.createClose("Factor");
}

