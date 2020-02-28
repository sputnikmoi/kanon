#ifndef FORMATFLOAT_H
#define FORMATFLOAT_H

#include <string>

class QString;

enum EFormat
{
	eFormatHtml, eFormatText
};

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CFormatFloat
{
	size_t m_Len;
	char m_Buf[128];
	std::string m_Format;
public:
	CFormatFloat(const std::string& format = " % 2.3f") : m_Len(), m_Format(format) {}
	void simplify(double);
	const char* get(double);
	static QString epsilonFactor(double factor, bool unary = true, EFormat format = eFormatText);
	static QString bilinear(double cst, double factor, EFormat format = eFormatText);
};

#endif

