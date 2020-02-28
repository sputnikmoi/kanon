#include <QtWidgets/QApplication>
#include "CWndMain.h"

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	QFont font(QApplication::font());
	font.setPointSize(10);
	font.setWeight(QFont::Normal);
	QApplication::setFont(font);
	CWndMain wnd;
	wnd.show();
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}
