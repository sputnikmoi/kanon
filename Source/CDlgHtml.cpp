#include <cstdio>
#include <QKeyEvent>
#include <QPainter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextBrowser>
#include "CDlgHtml.h"
#include "Util.h"

/* METHOD *********************************************************************/
/**
  Creates a CDlgHtml window and shows the given page.
We set the Qt::WA_GroupLeader attribute because we want to pop up CDlgHtml windows from modal
dialogs in addition to the main window. Modal dialogs normally prevent the user from interacting
with any other window in the application. However, after requesting help, the user must obviously
be allowed to interact with both the modal dialog and the help browser. Setting the Qt::WA_GroupLeader
attribute makes this interaction possible.
*******************************************************************************/
CDlgHtml::CDlgHtml(const QString& html, QWidget* parent)
  : QDialog(parent)
  , m_TextBrowser()
{
  setWindowTitle("Kanon ouput: Dimensions and exponent matrix");
  QVBoxLayout* loMain{new QVBoxLayout};
  setLayout(loMain);
  m_TextBrowser = new QTextBrowser;
  loMain->addWidget(m_TextBrowser);
   
  QHBoxLayout* loButtons{new QHBoxLayout()};
  loMain->addLayout(loButtons);
   
  QPushButton* btnPrint{new QPushButton("&Print")};
  loButtons->addWidget(btnPrint);
  connect(btnPrint, SIGNAL(clicked()), this, SLOT(onPrint()));
   
  QPushButton* btnClose{new QPushButton("&Close")};
  btnClose->setShortcut(tr("Esc"));
  loButtons->addWidget(btnClose);
   
  connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
  m_TextBrowser->setText(html);
   
  QFont fnt(m_TextBrowser->font());
  fnt.setPointSize(11);
  //fnt.setWeight(99);
  m_TextBrowser->setFont(fnt);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgHtml::keyPressEvent(QKeyEvent* e)
{
  const bool ctl{0 != (e->modifiers() & Qt::ControlModifier)};
  const int ascii{e->text().isEmpty() ? 0 : e->text()[0].toLatin1()};
  if (ctl && ascii == 'F' - '@')
  {
  }
  else e->ignore();
}

/* METHOD *********************************************************************/
/**
  Slot
*******************************************************************************/
void CDlgHtml::onPrint()
{
  QPrinter printer;
  if (!printer.isValid())
  {
    return;
  }
  QPrintDialog printDialog(&printer, this);
  if (printDialog.exec() == QDialog::Accepted)
  {
    if (printer.isValid())
    {
      m_TextBrowser->print(&printer);
    }
    else
    {
      msgBox(this, "No printer available");
    }
  }
}

