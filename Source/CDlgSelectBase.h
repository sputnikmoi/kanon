// Base class for dialog diplaying a selection table.
#ifndef CDLGSELECTBASE_H
#define CDLGSELECTBASE_H
#include <QtWidgets/QDialog>
#include <QtWidgets/QTableView>

/* FORWARD DECLARATIONS *******************************************************/
class CTableView;
class QAbstractTableModel;
class QBoxLayout;
class QBoxLayout;
class QCheckBox;
class QModelIndex;
class QPushButton;
class QSignalMapper;
class QTableView;

/* CONSTANT DECLARATIONS ******************************************************/
/* CLASS DECLARATIONS *********************************************************/
/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CDlgSelectBase : public QDialog
{
protected:
  QString m_Title;
  QBoxLayout* m_BoxLayout;
  QBoxLayout* m_ButtonBoxLayout;
  CTableView* m_TableView;
  unsigned m_ExecPk;
public:
  CDlgSelectBase(QWidget* parent, const QString& title, QAbstractTableModel*, unsigned bgcolor = 0);
  QPushButton* addButton(const QString& text, const char* method, const QString& toolTip="");
  QPushButton* addButton(QSignalMapper*, const QString& text, int mapVal, const QString& toolTip="");
  QCheckBox* addCheckBox(QSignalMapper*, const QString& text, int mapVal, const QString& toolTip);
  QModelIndex currentIndex() const;
  CTableView* tableView() const { return m_TableView; }
  QBoxLayout* buttonBoxLayout() const;
  void resizeColumns();
  void resizeRows();
  void updateTitle(int numRow = -1, const QString& info = "");
  unsigned execPk() const { return m_ExecPk; }
  void virtual onCurrentChanged(const QModelIndex&) {}
protected:
  virtual void updateFilter(const QString& /*i_filter*/, bool /*i_caseSensitive*/) {}
};

/* CLASS DECLARATION **********************************************************/
/**
  Calls onCurrentChanged() in the dialog.
*******************************************************************************/
class CTableView : public QTableView
{
  CDlgSelectBase* m_DlgSelect;
public:
  CTableView(CDlgSelectBase* dlgSelect) : m_DlgSelect(dlgSelect) {}
  void currentChanged(const QModelIndex& curr, const QModelIndex& prev)
  {
    QTableView::currentChanged(curr, prev);
    m_DlgSelect->onCurrentChanged(curr);
  }
};

#endif
