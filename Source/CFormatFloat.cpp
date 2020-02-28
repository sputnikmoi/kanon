// Formats float value, determines width
#include <math.h>
#include <cstring>
#include <cstdio>
#include <QString>
#include "CFormatFloat.h"
#include "CGlyph.h"

using std::string;

namespace
{
	const double TINY{1e-6f};
}

/* METHOD *********************************************************************/
/**
  Puts value in string format into m_Buf.
  Determines maximal length required to display a set of values, eliminating
  common trailing '0' characters in fractional part.
*******************************************************************************/
void CFormatFloat::simplify(double val)
{
	if (fabs(val) < TINY)
	{
		val = 0.0;  // Rounding error
	}
	sprintf(m_Buf, m_Format.c_str(), val);
	size_t len1{strlen(m_Buf)};
	for (; len1 > 0;)
	{
		const int ch{m_Buf[len1 - 1]};
		if (ch == '0')
		{
			len1--;
		}
		else if (ch == '.' || ch == ',')
		{
			len1--;
			break;
		}
		else break;
	}
	if (len1 > m_Len)
	{
		m_Len = len1;
	}
}

/* METHOD *********************************************************************/
/**
@return Formatted value
*******************************************************************************/
const char* CFormatFloat::get(double val)
{
	simplify(val);
	m_Buf[m_Len] = 0;
	return m_Buf;
}

/* METHOD *********************************************************************/
/**
  Creates string representation of factor*<epsilon>, omitting a factor of
absolute value 1.
@param  factor: The factor
@param   unary: Only display negative sign, with no spaces.
@return expression as QString
*******************************************************************************/
QString CFormatFloat::epsilonFactor(double factor, bool unary, EFormat format)
{
	const bool asHtml(format == eFormatHtml);
	QString ret;
	const double absFact(fabs(factor));
	if (absFact < TINY)
	{
		return unary ? "0" : "";
	}
	else if (fabs(absFact -1.0) < TINY)
	{	// Omit factor 1
		ret = asHtml ? QString("&epsilon;") : QString(QChar(epsilon));
	}
	else
	{
		CFormatFloat flt("%2.3f");
		flt.simplify(absFact);
		ret = QString(flt.get(absFact));
		if (asHtml)
		{
			ret += " &times; &epsilon;";
		}
		else
		{
			ret += " * " + QString(QChar(epsilon));
		}
	}
	if (unary)
	{
		if (factor < 0.0)
		{
			ret = "-" + ret;
		}
	}
	else
	{
		ret = (factor < 0.0 ? " - " : " + ") + ret;
	}
	return ret;
}

/* METHOD *********************************************************************/
/**
  Creates string representation of cst + factor*<epsilon>, omits cst of value
0.0 and omits factor of absolute value 1.0.
@param constVal: Constant
@param   factor: Factor for epsilon
@return expression
*******************************************************************************/
QString CFormatFloat::bilinear(double constVal, double factor, EFormat format)
{
	if (fabs(constVal) < TINY)
	{
		return epsilonFactor(factor, true, format);
	}
	CFormatFloat flt("%2.3f");
	flt.simplify(constVal);
	if (fabs(factor) < TINY)
	{
		return flt.get(constVal);
	}
	return flt.get(constVal) + epsilonFactor(factor, false, format);
}

