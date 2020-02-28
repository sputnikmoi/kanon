/******************************************************************************/
/**
@file         HtmlOutput.cpp
@copyright
*
@language     C++
*
@description  Minimal html output formatting
********************************************************************************
*******************************************************************************/
#include <cmath>
#include "CModelData.h"
#include "HtmlOutput.h"
#include "strutil.h"

using std::string;

namespace
{
	int NUM_REP{9999};
	string toHtml(const string& str)
	{	// Some ad-hoc HTML formating
		string ret(str);
		// Must be done first, else inserted tags get removed
		ret = replace(ret, "<", "&lt;", NUM_REP);
		ret = replace(ret, ">", "&gt;", NUM_REP);
		//---
		// These combinations might occur in normal text, replace only complete words
		ret = replace(ret, "chi", "&chi;", NUM_REP, true);
		ret = replace(ret, "Chi", "&Chi;", NUM_REP, true);
		 
		ret = replace(ret, "phi", "&phi;", NUM_REP, true);
		ret = replace(ret, "Phi", "&Phi;", NUM_REP, true);
		ret = replace(ret, "psi", "&psi;", NUM_REP, true);
		ret = replace(ret, "Psi", "&Psi;", NUM_REP, true);
		 
		ret = replace(ret, "^2", "<sup>2</sup>", NUM_REP);
		ret = replace(ret, "^3", "<sup>3</sup>", NUM_REP);
		ret = replace(ret, "^4", "<sup>4</sup>", NUM_REP);
		ret = replace(ret, "^5", "<sup>5</sup>", NUM_REP);
		ret = replace(ret, "^6", "<sup>6</sup>", NUM_REP);
		ret = replace(ret, "^8", "<sup>8</sup>", NUM_REP);
		ret = replace(ret, "ä", "&auml;", NUM_REP);
		ret = replace(ret, "ö", "&ouml;", NUM_REP);
		ret = replace(ret, "ü", "&uuml;", NUM_REP);
		return ret;
	}
	string addI(const string str)
	{
		return "<i>" + str + "</i>";
	}
}

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CHtml
{
	string m_Html;
public:
	CHtml(const string& title,
		const string& headTags = "", const string& bodyAttr = "",
		const string& prefix = "", const string& htmlAttrs = "")
		: m_Html()
	{	// Creates HTML head and body tags.
		if (!prefix.empty())
		{
			m_Html += prefix + "\r\n";
		}
		tag("html", htmlAttrs);
		tag("head");
		m_Html += "<title>" + title + "</title>\r\n" + headTags + "</head>\r\n\n"
			"<body " + bodyAttr + ">\r\r\n";
	}
	string text() const
	{
		return m_Html;
	}
	void close()
	{
		m_Html += "\r\n</body>\r\n</html>\r\n";
	}
	void h1(const string& text)
	{
		m_Html += "<h1>" + text + "</h1>\r\n";
	}
	void h2(const string& text)
	{
		m_Html += "<h2>" + text + "</h2>\r\n";
	}
	void h3(const string& text)
	{
		m_Html += "<h3>" + text + "</h3>\r\n";
	}
	void indent(int depth = 1)
	{
		for (; depth > 0; depth--)
		{
			m_Html += '\t';
		}
	}
	void tag(const string& name, const string& attrs = "")
	{
		m_Html += "<" + name + insSpace(attrs) + ">\r\n";
	}
	void end(const string& name, int indentVal = 0)
	{
		indent(indentVal);
		m_Html += "</" + name + ">\r\n";
	}
	void tag1(int indentVal, const string& tag, const string& text, const string& attrs = "")
	{	// Creates a text tag, attributes are optional
		indent(indentVal);
		m_Html += '<' + tag + insSpace(attrs) + '>' + text;
		end(tag);
	}
	void tableCell(const string& text, const string& attrs = "")
	{
		tag1(2, "td", text, attrs);
	}
	void rowspan(bool firstRow, int numRow, const string& text, const string& attrs = "")
	{
		if (firstRow)
		{
			tableCell(text, toString("rowspan=\"%d\"", numRow) + insSpace(attrs));
		}
	}
	void matrixBracket(bool firstRow, int numRow)
	{
		rowspan(firstRow, numRow, "", CHtml::bg("black") + " width = \"1\"");
	}
	void para(const string& text)
	{
		m_Html += "<p>" + text;
		end("p");
	}
	static string bg(const string& color)
	{
		return "bgcolor=" + quote(color);
	}
private:
	string insSpace(const string attrs)
	{
		return attrs.empty() ? "" : (" " + attrs);
	}
};

/* FUNCTION *******************************************************************/
/**
  Creates a HTML file with results.
@precondition getModel() != 0.
@param rxInteraction
*******************************************************************************/
string htmlModelOutput(size_t rxInteraction)
{
	const bool haveDim{true};
	const bool xhtml{};
	CModelData& mod{model()};
	const auto modelOrder{mod.modelOrder()};
	const char* Css{
		"<style type=\"text/css\" media=\"all | print | screen\">\r\n"
#if 0
		"	body{font-size: 15; font-family: Arial, Helvetica, Sans-Serif; background: #FFFFFF;}\r\n"
		"	h1{font-weight: bold; font-family: Times; font-size: 24pt; background-color: #E8E8D8;}\r\n"
		"	h2{font-weight: bold; font-family: Times; font-size: 20pt; background-color: #E8E8D8;}\r\n"
		"	h3{font-weight: bold; font-family: Times; font-size: 14pt; }\r\n"
#endif
		"</style>\r\n"};
	const string prefix(xhtml ? "<?xml version=\"1.0\"?>\r\n<!DOCTYPE html SYSTEM \"mathml.dtd\">" : "");
	const string htmlAttrs(xhtml ? "xmlns=\"http://www.w3.org/1999/xhtml\" "
		"xmlns:math=\"http://www.w3.org/1998/Math/MathML\" "
		"xmlns:xlink=\"http://www.w3.org/1999/xlink\""
		: "");

	CHtml html("Kanon-Output", Css, "", prefix, htmlAttrs);
	if (!haveDim)
	{
		return html.text();
	}
	string name(trimWhite(mod.name()));
	if (name.empty())
	{
		name = "Name?";
	}
	html.h1(toHtml(name));
	mod.determineNormalVector();
	html.para(toString("Critical dimension <i>d<sub>c</sub> = %s</i>, <i>model order = %d</i>, "
		"normal vector <i>(%s)</i>.",
		mod.printCriticalDimension().c_str(), modelOrder, mod.printSortedNormalVector().c_str()));
	// Exponent Matrix//
	string coordFieldWarning(". ");
	html.tag("table", "border = \"1\"");
	html.tag1(1, "caption", "<b>Exponent matrix &nbsp;" + mod.getCoordsFieldsList() + "</b>");
	for (size_t tx{}; tx <= mod.numTerm() + 1; tx++)
	{	// tx=0 is the column header row, then follow exponents -
		html.indent();
		html.tag("tr");
		if (tx == 0)
		{
			html.tableCell("&nbsp;", CHtml::bg("lightcyan"));
			html.tableCell("Comment", CHtml::bg("lightcyan"));
		}
		else if (tx <= mod.numTerm())
		{
			html.tableCell(toString("%d", tx), CHtml::bg("lightcyan"));
			const string caption(mod.printRowCaption(tx - 1)/*.substr(0, 50)*/);
			html.tableCell(toHtml(caption), CHtml::bg("whitesmoke"));
		}
		else
		{
			html.tableCell("&nbsp;", CHtml::bg("lightcyan"));
			html.tableCell("Dimension of coordinate/field at <i>d<sub>c</sub></i>&nbsp;",
				"align=\"center\" " + CHtml::bg("lightcyan"));
		}
		for (size_t cx{}; cx < modelOrder; cx++)
		{	// Exponent columns
			string text;
			string attrs;
			if (tx == 0)
			{	// Column header for coordinate/field cx
				if (mod.isResponseField(cx))
				{
					text += "~";
				}
				else text += " ";
				text += cx < mod.numCoord() ? "C" : "F";
				text += toString(int(1 + (cx < mod.numCoord() ? cx : cx - mod.numCoord())));
				attrs = "align=\"center\" " + CHtml::bg("lightcyan");
			}
			else if (tx <= mod.numTerm())
			{	// Display exponents
				if (cx == 0)
				{	// Exponent of d-dimensional coordinate
					const int expD{mod.getExpD(cx)};
					const int expC{mod.getExp(tx - 1, cx)};
					string str;
					if (expC != 0)
					{
						str = toString("%-d", expC);
						if (expD > 0)
						{
							str += "+";
						}
					}
					if (expD < 0)
					{
						str += "-";
					}
					if (expD != 0)
					{
						int expDAbs{abs(expD)};
						if (expDAbs != 1)
						{
							str += toString(abs(expDAbs));
						}
						str += "d";
					}
					text += addI(str);
				}
				else
				{
					text += addI(toString("%-d", mod.getExp(tx - 1, cx)));
				}
				attrs = "align=\"center\" ";
				if (tx <= modelOrder)
				{
					attrs += CHtml::bg("cornsilk");
				}
			}
			else
			{	// Last row displays coordinate/field dimensions
				const double dimAtCritDim(mod.getDimensionAtCritDim(cx));
				text = addI(toString("%2.2f", dimAtCritDim));
				if (fabs(dimAtCritDim) < 0.001)
				{
					text = "<font color=\"red\">" + text + "</font>";
					coordFieldWarning = " (a zero value in principle signifies infinitely many marginal terms). ";
				}
				attrs += CHtml::bg("lightcyan");
			}
			html.tableCell(text, attrs);
		}
		// Additional columns//
		string text;
		if (tx == 0)
		{
			html.tableCell("Type", "align=\"center\" " + CHtml::bg("lightcyan"));
			html.tableCell(addI("[g]"), "align=\"center\" " + CHtml::bg("lightcyan"));
		}
		else if (tx <= mod.numTerm())
		{
			if (tx == rxInteraction + 1)
			{	// Currently selected row, coupling constant
#ifdef __linux__
				text = "&lArr;&nbsp;coupling";
#else
				text = "&lt;&lt;&nbsp;coupling";
#endif
				html.tableCell(text, "align=\"center\"");
				const string text(mod.printHtmlDimensionOfCouplingConst(0));
				html.tableCell(addI(text), "align=\"center\" " + CHtml::bg("lightcyan"));
			}
			else if (tx <= modelOrder)
			{	// Other normal terms
				text = "   kept fixed";
				html.tableCell(addI(text), "align=\"center\"");
				html.tableCell(addI("0"), "align=\"center\" " + CHtml::bg("lightcyan"));
			}
			else
			{	// Extra term
				text = mod.printRelevanceExtra(tx - 1, false);
				html.tableCell(addI(text), "align=\"center\"");
				html.tableCell(addI(mod.printRelevanceExtra(tx - 1, true)),
					"align=\"center\" " + CHtml::bg("lightcyan"));
			}
		}
		else
		{
			html.tableCell("&nbsp;", CHtml::bg("lightcyan"));
			html.tableCell("&nbsp;", CHtml::bg("lightcyan"));
		}
		html.end("tr", 1);
	}
	html.end("table");
	html.para("The last column contains the dimensions <i>[g]</i> of the coupling constants "
		"<i>(&epsilon; = d<sub>c</sub> - d)</i>.<br/>\r\n"
		"The last row contains the canonical wave vector dimensions <i>[f]</i> of the coordinates/fields <i>f</i> at "
		"the critical dimension" + coordFieldWarning +
		"The general expressions are listed in the table:");
	// General expressions for canonical dimensions of coordinates/fields//
	html.tag("table", "border = \"1\"");
	html.tag1(1, "caption", "<b>Canonical wave vector dimensions of coordinates and fields</b>");
	html.indent(1);
	html.tag("tr");
	html.tableCell("Name", CHtml::bg("lightcyan"));
	html.tableCell("Comment", CHtml::bg("lightcyan"));
	html.tableCell("Canonical dimension", "align=\"center\" " + CHtml::bg("lightcyan"));
	html.end("tr", 1);
	for (size_t fx{}; fx < modelOrder; fx++)
	{
		html.indent(1);
		html.tag("tr");
		string text;
		if (fx < mod.numCoord())
		{
			text = toString("\t[C%d]", fx + 1);
		}
		else
		{
			text = toString("\t[F%d]", fx + 1 - mod.numCoord());
		}
		html.tableCell(text, CHtml::bg("lightcyan"));
		html.tableCell(mod.printColumnCaption(fx).substr(0, 50),
			CHtml::bg("whitesmoke"));
		text = mod.printCanonicalDimension(fx);
		// ToDo: Use color in case of non-positive value !
		html.tableCell(addI(text), "align=\"center\"");
		html.end("tr", 1);
	}
	html.end("table");
	// Comment, References//
	html.h2("Comments and references");
	if (!mod.comment().empty())
	{
		html.para(replace(toHtml(mod.comment()), "\n", "<br/>", NUM_REP));
	}
	if (!mod.references().empty())
	{
		html.para(replace(toHtml(mod.references()), "\n", "<br/>", NUM_REP));
	}
	html.close();
	return html.text();
}

