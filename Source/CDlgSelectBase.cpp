// Base class for dialog diplaying a selection table.
#include <QSignalMapper>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include "CDlgSelectBase.h"

#define PK_NULL 0U // Invalid


/* METHOD *********************************************************************/
/**
  Ctor.
*******************************************************************************/
CDlgSelectBase::CDlgSelectBase(QWidget* parent, const QString& title, QAbstractTableModel* tableModel, unsigned bgcolor)
  : QDialog(parent)
  , m_Title(title)
  , m_BoxLayout(new QHBoxLayout)
  , m_ButtonBoxLayout(new QHBoxLayout)
  , m_TableView()
  , m_ExecPk(PK_NULL)
{
  setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint); // No effect !
  QBoxLayout* lv{new QVBoxLayout(this)};
  lv->addLayout(m_BoxLayout);
  m_TableView = new CTableView(this);
  m_TableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_TableView->setSortingEnabled(true);
  m_TableView->setSelectionMode(QAbstractItemView::SingleSelection);
  if (bgcolor != 0)
  {
    QPalette pal(m_TableView->palette());
    pal.setColor(QPalette::Base, bgcolor);
    m_TableView->setPalette(pal);
  }
  m_BoxLayout->addWidget(m_TableView);
  lv->addLayout(m_ButtonBoxLayout);
  m_TableView->setModel(tableModel);
  updateTitle();
}

/* METHOD *********************************************************************/
/**
  Creates button and adds it to the m_ButtonBoxLayout, connects signal.
*******************************************************************************/
QPushButton* CDlgSelectBase::addButton(const QString& text, const char* method, const QString& toolTip)
{
  if (m_ButtonBoxLayout)
  {
    QPushButton* btn{new QPushButton(text, this)};
    btn->setToolTip(toolTip);
    m_ButtonBoxLayout->addWidget(btn);
    if (method)
    {
      QObject::connect(btn, SIGNAL(clicked()), this, method);
    }
    return btn;
  }
  return 0;
}

/* METHOD *********************************************************************/
/**
  Creates and connects a button.
@return QPushButton*
*******************************************************************************/
QPushButton* CDlgSelectBase::addButton(QSignalMapper* signMap, const QString& text, int mapVal, const QString& toolTip)
{
  QPushButton* btn{};
  if (m_ButtonBoxLayout)
  {
    btn = new QPushButton(text, this);
    btn->setToolTip(toolTip);
    m_ButtonBoxLayout->addWidget(btn);
    signMap->setMapping(btn, mapVal);
    QObject::connect(btn, SIGNAL(clicked()), signMap, SLOT(map()));
  }
  return btn;
}

/* METHOD *********************************************************************/
/**
  Creates and connects a checkbox.
@return QCheckBox*
*******************************************************************************/
QCheckBox* CDlgSelectBase::addCheckBox(QSignalMapper* signMap, const QString& text, int mapVal, const QString& toolTip)
{
  QCheckBox* chk{};
  if (m_ButtonBoxLayout)
  {
    chk = new QCheckBox(text, this);
    chk->setToolTip(toolTip);
    m_ButtonBoxLayout->addWidget(chk);
    signMap->setMapping(chk, mapVal);
    QObject::connect(chk, SIGNAL(toggled(bool)), signMap, SLOT(map()));
  }
  return chk;
}

QBoxLayout* CDlgSelectBase::buttonBoxLayout() const
{
  return m_ButtonBoxLayout;
}

QModelIndex CDlgSelectBase::currentIndex() const
{
  return m_TableView->currentIndex();
}

void CDlgSelectBase::resizeColumns()
{
  for (int cx{}; cx < m_TableView->model()->columnCount(); cx++)
  {
    m_TableView->resizeColumnToContents(cx);
  }
}
void CDlgSelectBase::resizeRows()
{
  for (int rx{}; rx < m_TableView->model()->rowCount(); rx++)
  {
    m_TableView->setRowHeight(rx, 19);
  }
}

void CDlgSelectBase::updateTitle(int numRow, const QString& info)
{
  if (numRow >= 0)
  {
    setWindowTitle(m_Title + " (" + QString::number(numRow) + ")" + info);
  }
  else
  {
    setWindowTitle(m_Title + " (" + QString::number(m_TableView->model()->rowCount()) + ")");
  }
}

