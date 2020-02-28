
#ifndef CDLGHTML_H
#define CDLGHTML_H

#include <QtWidgets/QDialog>

class QTextBrowser;

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CDlgHtml : public QDialog
{
	Q_OBJECT
public:
  CDlgHtml(const QString& html, QWidget* parent = nullptr);
private:
  QTextBrowser* m_TextBrowser;
  QSize sizeHint() const { return QSize(750, 600); }
  void keyPressEvent(QKeyEvent*) override;
private slots:
  void onPrint();
};

#endif

