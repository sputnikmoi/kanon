#ifndef CGLYPH_H
#define CGLYPH_H

#include <string>

class CModelData;
class CXmlCreator;
class QDomElement;
class QPainter;

enum ESymbol
{
	none = 0,
	// Operators
	integral = 0x222B,
	delta = 0x03B4, // Delta-Function
	Delta = 0x0394,
	sum = 0x2211,
	nabla =  0x2207,
	partial = 0x2202,
	// Binary and/or neutral//
	bra = '(',
	ket = ')',
	dot = '.', //0x02D9,
	minus_ = 0x2212,
	plus_ = '+',
	otimes = 0x2297, // CIRCLED TIMES
	times  = 0x00D7, // MULTIPLICATION SIGN (cross)
	bullet = 0x2219, // BULLET OPERATOR (scalar product)
	dgamma = 0x3B3,
	qmark = '?',
	// Latin//
	a_ = 'a',
	A_ = 'A',
	b_ = 'b',
	B_ = 'B',
	c_ = 'c',
	d_ = 'd',
	f_ = 'f',
	F_ = 'F',
	j_ = 'j',
	m_ = 'm',
	M_ = 'M',
	n_ = 'n',
	N_ = 'N',
	h_ = 'h',
	I_ = 'I',
	l_ = 'l',
	p_ = 'p',
	q_ = 'q',
	r_ = 'r',
	s_ = 's',
	t_ = 't',
	u_ = 'u',
	v_ = 'v',
	w_ = 'w',
	x_ = 'x',
	y_ = 'y',
	z_ = 'z',
	// Greek//
	alpha = 0x3B1,
	beta = 0x3B2,
	chi = 0x3C7,
	epsilon = 0x3B5,
	zeta = 0x3B6,
	eta = 0x3B7,
	theta = 0x3B8,
	jota = 0x3B9,
	kappa = 0x3BA,
	lambda = 0x3BB,
	mu = 0x3BC,
	xi = 0x3BE,
	pi = 0x3C0,
	rho = 0x3C1,
	sigma = 0x3C3,
	phi = 0x3C6,
	Phi = 0x3A6,
	//ch = 0x3C7,
	psi = 0x3c8,
	Psi = 0x3A8
};

enum ECoordField { eCoord, eField };
bool hasCoordinate(ESymbol symb);

/* STRUCT DECLARATION *********************************************************/
/**
  Describes typographical attributes of a coordinate/Field.
  Usage://
  CGlyphField and CGlyphCoordinate contain m_CoordFieldIndex, which allows to get
  the coordinate/field specific attributes 'SCoordFieldAttributes' from CModelData.
*******************************************************************************/
struct SCoordFieldAttributes
{
	ESymbol m_Symb; // Unicode character to display (phi, s, x, t, ...)
	int  m_Suffix;  // >= 0, suffix displayed with coord/field.
	bool m_Bold;    // Display in bold.
	bool m_Primed;  // Display a tilde above the coordinate/field.
	bool m_Tilde;   // Display a tilde above the coordinate/field.
	SCoordFieldAttributes(ESymbol symb) : m_Symb(symb), m_Suffix(-1), m_Bold(), m_Primed(), m_Tilde() {}
};

/* CLASS DECLARATION **********************************************************/
/**
  A CGlyph defines a group of symbols inserted, edited, deleted and moved together.
  Examples are ddx, deltad(x), nabla2, phi_tilde_4. Once defined only restricted
  editing (changing exponents) is possible.
  A model term is a vector<SGlyph>.
*******************************************************************************/
class CGlyphBase
{
protected:
	void incExponent(int& exponent, int delta, int minVal = 1)
	{
		if (delta == 1) { if (exponent < 99) exponent++; }
		else if (exponent > minVal) { exponent--; }
	}
public:
	virtual ~CGlyphBase() {}
	static void initializeSymbolTable();
	virtual CGlyphBase* clone() = 0;
	virtual void paint(QPainter&, int& xPos) const = 0;
	virtual void toXml(CXmlCreator&) const = 0;
	virtual bool isNeutral() { return false; }
	virtual ESymbol symbol() const { return none; }
	virtual void incExponent(int) {}
	virtual int exponent(size_t /*ixColumn*/, const CModelData&) const { return 0; }
	virtual int exponentD() const { return 0; }
};

/* CLASS DECLARATION **********************************************************/
/**
  This glyph isn't directly associated to a coordinate/field.
  Binary operators and brackets...
*******************************************************************************/
class CGlyphNeutral : public CGlyphBase
{
	ESymbol m_Symb; // Symbol to display (otimes, minus, plus, ...)
	bool m_Bold;    // Display in bold
public:
	CGlyphNeutral(ESymbol symb, bool bold = false);
	CGlyphNeutral(QDomElement&);
	CGlyphBase* clone() override { return new CGlyphNeutral(*this); }
	void paint(QPainter&, int& xPos) const override;
	void toXml(CXmlCreator&) const override;
	bool isNeutral() override { return true; }
	ESymbol symbol() const override { return m_Symb; }
};

/* CLASS DECLARATION **********************************************************/
/**
  Represents a field within a row (with optional exponent).
  Binary operators and brackets...
*******************************************************************************/
class CGlyphField : public CGlyphBase
{
	int m_FieldIndex; // Field index from CModelData
	int m_Exponent;   // Exponent displayed with field
	std::string getExponentString() const;
public:
	CGlyphField(int fieldIndex, int exponent = 1);
	CGlyphField(QDomElement&);
	CGlyphBase* clone() override { return new CGlyphField(*this); }
	int  fieldIndex() const { return m_FieldIndex; }
	int  exponent(size_t ixColumn, const CModelData&) const override;
	void paint(QPainter&, int& xPos) const override;
	void toXml(CXmlCreator&) const override;
	void incExponent(int delta) override { CGlyphBase::incExponent(m_Exponent, delta, -1); }
	void decIndex() { m_FieldIndex--; }
	void setIndex(int fx) { m_FieldIndex = fx; }
};

/* CLASS DECLARATION **********************************************************/
/**
  Represents a coordinate+operator within a row.
*******************************************************************************/
class CGlyphCoordinate : public CGlyphBase
{
	int  m_CoordIndex; // Coordinate index from CModelData (attribute to m_Symb)
	ESymbol m_Symb;    // EXTRA symbol to display (integrate, nabla, delta, ...)
	int  m_Exponent;   // Exponent
	char m_ExponentCh; // Overwrites m_Exponent if != 0
	int  m_Suffix;     // >= 0, suffix (or character, e.g. 't') displayed with coord/field
	std::string getExponentString() const;
public:
	CGlyphCoordinate(int coordIndex, ESymbol symb, int exponent = 1);
	CGlyphCoordinate(QDomElement&);
	CGlyphBase* clone() override { return new CGlyphCoordinate(*this); }
	int  coordIndex() const { return m_CoordIndex; }
	void paint(QPainter&, int& xPos) const override;
	void toXml(CXmlCreator&) const override;
	void incExponent(int delta) override { CGlyphBase::incExponent(m_Exponent, delta, -4); }
	void decIndex() { m_CoordIndex--; }
	void setIndexToDefault() { m_CoordIndex = 0; }
	int  exponent(size_t ixColumn, const CModelData&) const override;
	int  exponentD() const override;
};

/* CLASS DECLARATION **********************************************************/
/**
  Defines the attributes of a coord/field (see SCoordFieldAttributes),
  and allows to display the coordinates and fields in toolboxes.
  DON'T use in a row (=model term) !
*******************************************************************************/
class CGlyphCoordField : public CGlyphBase
{
	std::string m_Comment;
	SCoordFieldAttributes m_Attributes;
public:
	CGlyphCoordField(ESymbol symb = qmark, const std::string& comment = "");
	CGlyphCoordField(QDomElement&);
	CGlyphBase* clone() override { return new CGlyphCoordField(*this); }
	void paint(QPainter&, int& xPos) const override;
	void toXml(CXmlCreator&) const override { return; }
	void toXml(CXmlCreator&, const std::string& tag) const;
	std::string toStr() const;
	std::string comment() const { return m_Comment; }
	void setSymbol(ESymbol symb) { m_Attributes.m_Symb = symb; }
	void setBold(bool bold) { m_Attributes.m_Bold = bold; }
	void setComment(const std::string& text) { m_Comment = text; }
	void setSuffix(int suffix) { m_Attributes.m_Suffix = suffix; }
	void setTilde(bool tilde) { m_Attributes.m_Tilde = tilde; }
	void setPrimed(bool primed) { m_Attributes.m_Primed = primed; }
	const SCoordFieldAttributes& attrs() const { return m_Attributes; }
	bool hasTilde() const { return m_Attributes.m_Tilde; }
	bool hasPrime() const { return m_Attributes.m_Primed; }
};

#endif

