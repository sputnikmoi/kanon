#ifndef UTIL_H
#define UTIL_H
#include <QString>

/* FORWARD DECLARATIONS *******************************************************/
class QAction;
class QBoxLayout;
class QDomDocument;
class QDomElement;
class QDomNode;
class QMenu;
class QPushButton;
class QSignalMapper;
class QWidget;

enum ECase
{
  eCaseSensitive, eCaseInsensitive
};

struct MimeFormat
{
	static const QString Coord;
	static const QString Field;
	static const QString Operator;
	static const QString Row;
};

/* FUNCTIONS ******************************************************************/
QString appName();
QPushButton* addButton(QWidget* parent, QBoxLayout*, QSignalMapper*, const QString& name, int mapVal, const QString& toolTip="");
QAction* addMenuAction(QWidget* parent, QMenu*, const QString& name, const QString& shortcut, QSignalMapper*, int mapVal);
unsigned createRandomKey();
std::string getVersionString();
void msgBox(QWidget* parent, const QString& text);
bool msgBoxCritical(const std::string& text, QWidget* parent = nullptr);
bool yesNo(QWidget* parent, const std::string& text, const std::string& title = "");
int yesNoCancel(QWidget* parent, const std::string& text, const std::string& title = "");

QString fromUtf8(const std::string& text);
std::string  toUtf8(const QString& text);

std::string pathToData();
bool stringMatchesFilter(const std::string& filter, const std::string& text, ECase caseSensitive);

void throwError(const std::string& text);
void throwError(const QString& text);
void throwAssert(const std::string& text, bool val);

bool        xmlGetBool(QDomNode&, const QString& name);
bool        xmlHasTagName(QDomNode&, const QString& name);
std::string xmlGetAttr(QDomNode&, const QString& name, bool mandatory = false);
std::string xmlGetPcData(QDomElement&);
std::string xmlGetTextData(QDomElement&);
std::string xmlGetTextData2(QDomNode&, const QString& name);

std::string xmlRequireAttr(QDomNode&, const QString& name);
void        xmlOpen(QDomDocument&, const QString& filename);

#endif

