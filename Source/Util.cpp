#ifdef __linux__
#	include <unistd.h>
#else
#	include <cstdlib>
#	include <ctime>
#endif
#include <stdexcept>
#include <string>
#include <QtCore/QFile>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QAction>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtXml/QDomNode>
#include <QUuid>
#include "Util.h"
#include "strutil.h"

using std::runtime_error;
using std::string;

namespace
{
	const unsigned s_ModuleVerMajor{3}; // Written to XML file
	const unsigned s_ModuleVerMinor{1}; // Written to XML file
	const unsigned s_ModuleVerRel{0};   // Written to XML file
}

const QString MimeFormat::Coord("kanon_coord");
const QString MimeFormat::Field("kanon_field");
const QString MimeFormat::Operator("kanon_operator");
const QString MimeFormat::Row("kanon_row");

/* FUNCTION *******************************************************************/
/**
*******************************************************************************/
QString appName()
{
	return "Kanon";
}

/* FUNCTION *******************************************************************/
/**
  Creates and connects a button.
@return QPushButton*
*******************************************************************************/
QPushButton* addButton(QWidget* parent, QBoxLayout* lb, QSignalMapper* signMap,
  const QString& name, int mapVal, const QString& toolTip)
{
  QPushButton* btn{new QPushButton(name, parent)};
  lb->addWidget(btn);
  btn->setToolTip(toolTip);
  signMap->setMapping(btn, mapVal);
  QObject::connect(btn, SIGNAL(clicked()), signMap, SLOT(map()));
  return btn;
}

/* FUNCTION *******************************************************************/
/**
	Creates adds and connects a menu action.
*******************************************************************************/
QAction* addMenuAction(QWidget* parent, QMenu* menu, const QString& name, const QString& shortcut, QSignalMapper* signMap, int mapVal)
{
	QAction* act{new QAction(name, parent)};
	act->setShortcut(shortcut);
	signMap->setMapping(act, mapVal);
	QObject::connect(act, SIGNAL(triggered()), signMap, SLOT(map()));
	menu->addAction(act);
	return act;
}

/* FUNCTION *******************************************************************/
/**
@return Random primary key, always > 1.
*******************************************************************************/
unsigned createRandomKey()
{
  for (;;)
  {
    unsigned key{qHash(QUuid::createUuid())};
    if (key != 0)
    {
      return key;
    }
  }
}

/* FUNCTION *******************************************************************/
/**
@return version string
*******************************************************************************/
string getVersionString()
{
  string ret(toString("%u.%u.%u ", s_ModuleVerMajor, s_ModuleVerMinor, s_ModuleVerRel));
  ret += string("\n") + __DATE__;
  return ret;
}

/* FUNCTION *******************************************************************/
/**
*******************************************************************************/
void msgBox(QWidget* parent, const QString& text)
{
	if (!text.isEmpty())
	{
		QMessageBox::information(parent, "tstQt", text);
	}
}
bool msgBoxCritical(const string& text, QWidget* parent)
{
	QMessageBox::critical(parent, appName(), text.c_str());
	return false;
}
bool yesNo(QWidget* parent, const string& text, const string& title)
{
	QMessageBox msgBox(parent);
	msgBox.setWindowTitle(title.empty() ? appName() : title.c_str());
	msgBox.setText(text.c_str());
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	msgBox.setIcon(QMessageBox::Question);
	const int ret{msgBox.exec()};
	return QMessageBox::Yes == ret;
}
int yesNoCancel(QWidget* parent, const string& text, const string& title)
{
	QMessageBox msgBox(parent);
	msgBox.setWindowTitle(title.empty() ? appName() : title.c_str());
	msgBox.setText(text.c_str());
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setIcon(QMessageBox::Question);
	return msgBox.exec();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
QString fromUtf8(const string& text)
{
  return QString::fromUtf8(text.c_str());
}
string toUtf8(const QString& text)
{
  return text.trimmed().toUtf8().constData();
}

/* FUNCTION *******************************************************************/
/**
	Throws a runtime_error
*******************************************************************************/
void throwError(const string& text)
{
	throw runtime_error(text);
}
void throwError(const QString& text)
{
	throw runtime_error(qPrintable(text));
}
void throwAssert(const string& text, bool val)
{
	if (!val)
	{
		throw runtime_error(text);
	}
}

/* FUNCTION *******************************************************************/
/**
@return Directory of predefined *.kxm files (models).
*******************************************************************************/
string pathToData()
{
#ifdef __linux__
	return "./Data";
#else
	return "../Data";
#endif
}

/* FUNCTION *******************************************************************/
/**
Use ";" instead of "|" for SCPI commands (which may contain "|").
Example: "Dis & aster | Accident". Space is same as '&'.
@return true when name matches filter. A leading '!' character in filter negates.
*******************************************************************************/
bool stringMatchesFilter(const string& filter, const string& text, ECase caseSensitive)
{
	for (size_t index{};;)
	{
		bool matched{true};
		for (;;)
		{	// All tokens to next '|' or EOT must match
			string token(getToken(filter, index, " &|", "", true));
			if (token.empty())
			{
				return matched;
			}
			else if (token == " " || token == "&")
			{
				continue;
			}
			else if (token == "|")
			{
				if (matched)
				{
					return true;
				}
				matched = true;
				continue;
			}
			const bool negate(token[0] == '!');
			if (negate)
			{
				token = token.substr(1);
			}
			if (caseSensitive == eCaseSensitive)
			{
				if (negate != (string::npos == text.find(token)))
				{
					matched = false;
				}
			}
			else
			{
				if (negate != (string::npos == toLower(text).find(toLower(token))))
				{
					matched = false;
				}
			}
		}
	}
}

/* FUNCTION *******************************************************************/
/**
	Opens an XML document.
*
@param      doc: [in/out]
@param filename:
@exception :
*******************************************************************************/
void xmlOpen(QDomDocument& doc, const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		throwError("Cannot open " + filename);
	}
	const QString text(file.readAll());
	QString errMsg;
	int errLine, errCol;
	if (!doc.setContent(text, false, &errMsg, &errLine, &errCol))
	{
		throwError("Error in " + filename + ", line " + QString::number(errLine) + ", col " + QString::number(errCol)
			+ "\n" + errMsg);
	}
}

/* FUNCTION *******************************************************************/
/**
	Reads attribute
@param      node:
@param      name:
@param mandatory:
@return attribute
@exception : runtime_error if mandatory
*******************************************************************************/
string xmlGetAttr(QDomNode& node, const QString& name, bool mandatory)
{
	QDomElement e(node.toElement());
	if (e.isNull())
	{
		throwError(QString("Not a QDomElement, '") +	name + "' at line " + QString::number(node.lineNumber()));
	}
	if (mandatory && !e.hasAttribute(name))
	{
		throwError(QString("Missing attr	'") +	name + "' at line " + QString::number(node.lineNumber()));
	}
	return e.attribute(name).toStdString();
}
string xmlRequireAttr(QDomNode& node, const QString& name)
{
	return xmlGetAttr(node, name, true);
}

bool xmlGetBool(QDomNode& node, const QString& name)
{
	string text(xmlGetAttr(node, name, false));
	if (text == "false" || text.empty())
	{
		text = "0";
	}
	else if (text == "true")
	{
		text = "1";
	}
	const bool ret(text == "1");
	if (!ret && !text.empty() && text != "0")
	{
		throwError("Invalid bool value (" + text + ")	in line " + toString(node.lineNumber()));
	}
	return ret;
}
bool xmlHasTagName(QDomNode& node, const QString& name)
{
	QDomElement e(node.toElement());
	return e.isNull() ? false : e.tagName() == name;
}

string xmlGetPcData(QDomElement& elem)
{
	QDomNode child(elem.firstChild());
	if (child.isCDATASection())
	{
		return child.toCDATASection().data().toStdString();
	}
	return "";
}

string xmlGetTextData(QDomElement& elem)
{
	QDomNode child(elem.firstChild());
	if (child.isText())
	{
		return child.toText().data().toStdString();
	}
	return "";
}

string xmlGetTextData2(QDomNode& node, const QString& name)
{
	for (QDomNode subnode(node.firstChild()); !subnode.isNull(); subnode = subnode.nextSibling())
	{
		if (xmlHasTagName(subnode, name))
		{
			QDomNode child(subnode.firstChild());
			if (child.isText())
			{
				return child.toText().data().toStdString();
			}
			 
		}
	}
	return "";
}

