#ifndef CTABLE_H
#define CTABLE_H

#include <cstdio>
#include <string>
#include <vector>
#include "Util.h"
#include <QStandardItemModel>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>

/* FORWARD DECLARATIONS *******************************************************/
using std::string;

/* CONSTANT DECLARATIONS ******************************************************/
#define PK_NULL 0U // Invalid
#define PK_ROOT 1U // If a root element is required

/* CLASS DECLARATIONS *********************************************************/
class QDomElement;

/* CLASS DECLARATION **********************************************************/
/**
  Template for a QAbstractTableModel. Data are in TTable.
*******************************************************************************/
template <typename TTable> class CQTableModel : public QAbstractTableModel
{
public:
  CQTableModel() : QAbstractTableModel() {}
  QVariant headerData(int col, Qt::Orientation orientation, int role) const override
  {
    return TTable::headerData(col, orientation, role);
  }
  int rowCount(const QModelIndex&) const override { return int(TTable::size()); }
  int columnCount(const QModelIndex&) const override { return TTable::columnCount(); }
  QVariant data(const QModelIndex& index, int role) const override
  {
    return TTable::data(index, role);
  }
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override
  {
    if (!TTable::empty())
    {
      beginResetModel();/**/
      if (TTable::getTableView())
      {
        for (size_t ix{}; ix < TTable::size(); ix++)
        { // Store visibility, to restore it after sort.
          TTable::at(ix).m_Hidden = TTable::getTableView()->isRowHidden(int(ix));
        }
      }
      const unsigned pk{TTable::selectedPk()};
      TTable::sort(column, order);
      if (TTable::getTableView())
      {
        for (size_t ix{}; ix < TTable::size(); ix++)
        {
          TTable::getTableView()->setRowHidden(int(ix), TTable::at(ix).m_Hidden);
        }
      }
      endResetModel();/**/
      TTable::selectRowByPk(pk); // Keep row focus
    }
  }
  void updateView()
  {
    beginResetModel();
    endResetModel();
  }
};

/* CLASS DECLARATION **********************************************************/
/**
  Template base class for tables.
  Contains a vector with row data, provides access methods.
*******************************************************************************/
template <typename TRow> class CTable
{
  static CQTableModel<TRow>* s_TableModel;
  static QTableView* s_TableView;
  static QString s_ColumnCaptions; // Separated with '|'
protected:
  static std::vector<TRow> s_Array;// Table data
  static string s_Name;            // Table name
public:
  unsigned Pk;      // Primary key
  unsigned m_OrgPk; // Used to tag records for export and similiar purposes.
  bool m_Tag;       // Available for various purposes.
  bool m_Hidden;
  static int s_SortColumn; // Column index
  static QAbstractTableModel* getTableModel()
  {
    return s_TableModel;
  }
  static void updateView()
  {
    return s_TableModel->updateView();
  }
  static void setTableView(QTableView* view) { s_TableView = view; }
  static QTableView* getTableView() { return s_TableView; }
  static void setRowHidden(size_t row, bool hidden)
  {
    getTableView()->setRowHidden(int(row), at(row).m_Hidden = hidden);
  }
  static void selectRowByPk(unsigned pk)
  {
    if (s_TableView)
    {
      size_t index;
      if (get(pk, &index))
      {
        s_TableView->selectRow(int(index));
      }
    }
  }
  static TRow* current()
  {
    if (s_TableView && s_TableView->currentIndex().isValid())
    {
      const QModelIndex index(s_TableView->currentIndex());
      return &at(index.row());
    }
    return nullptr;
  }
  static unsigned selectedPk()
  {
    if (s_TableView && s_TableView->currentIndex().isValid())
    {
      return at(s_TableView->currentIndex().row()).Pk;
    }
    return PK_NULL;
  }
  static int columnCount() { return s_ColumnCaptions.count("|") + 1; }
  static QString columnCaption(int col)
  {
    return s_ColumnCaptions.section('|', col, col);
  }
  CTable(): Pk(PK_NULL), m_OrgPk(PK_NULL), m_Tag(), m_Hidden() {}
  virtual bool okToImport() { return true; }
  static int sortColumn(int val = -1) { if (val >= 0) s_SortColumn = val; return s_SortColumn; }
  static size_t size()
  {
    return s_Array.size();
  }
  static int isize() { return int(s_Array.size()); }
  static bool empty() { return s_Array.empty(); }
  static bool push_back(const TRow& row)
  {
    if (row.Pk == PK_NULL)
    {
      return false;
    }
    for (const auto& elem : s_Array)
    {
      if (elem.Pk == row.Pk)
      {
        return false;
      }
    }
    s_Array.push_back(row);
    return true;
  }
  static TRow* get(unsigned pk, size_t* index = nullptr)
  {
    if (pk > 0) for (size_t ix{}; ix < s_Array.size(); ix++) if (s_Array[ix].Pk == pk)
    {
      if (index)
      {
        *index = ix;
      }
      return &s_Array[ix];
    }
    if (index)
    {
      *index = size();
    }
    return nullptr;
  }
  static TRow& at(size_t ix) { return s_Array[ix]; }
  static void erase(size_t ix)
  {
    if (ix < s_Array.size())
    {
      s_Array.erase(begin(s_Array) + ix);
    }
  }
  static bool del(unsigned pk)
  {
    for (size_t ix{}; ix < s_Array.size(); ix++) if (s_Array[ix].Pk == pk)
    {
      s_Array.erase(begin(s_Array) + ix);
      return true;
    }
    return false;
  }
  static void clear()
  {
    s_Array.clear();
  }
  static size_t count(unsigned fk)
  {
    size_t val{};
    for (size_t ix{}; ix < s_Array.size(); ix++) if (s_Array[ix].fk() == fk)
    {
      val++;
    }
    return val;
  }
  static unsigned createPk(const std::vector<TRow>& vect)
  {
    unsigned pk{PK_NULL};
    for (;;)
    {
      pk = createRandomKey();
      size_t ix{};
      for (; ix < vect.size(); ix++)
      {
        if (pk == vect.at(ix).Pk)
        {
          break; // In use, retry
        }
      }
      if (ix >= vect.size())
      {
        break;
      }
    }
    return pk;
  }
  static unsigned create(TRow& row)
  { // row only is a copy after push_back()!
    row.Pk = createPk(s_Array);
    s_Array.push_back(row);
    return row.Pk;
  }
  static unsigned prev(unsigned pk)
  {
    size_t index;
    if (0 != get(pk, &index))
    {
      if (index < size())
      {
        return s_Array[index - 1].Pk;
      }
    }
    return PK_NULL;
  }
  static void getNewFokusPk(unsigned& fokusPk, int row) // BEFORE erase(row). Qt4 sets RowHidden to false during update!
  {
    for (size_t hx{}; hx < size(); hx++)
    {
      at(hx).m_Hidden = getTableView()->isRowHidden(int(hx));
    }
    int rx(row + 1);
    for (; rx < isize() && at(rx).m_Hidden; rx++) {}
    if (rx >= isize())
    {
      rx = row - 1;
      for (; rx >= 0 && at(rx).m_Hidden; rx--) {}
    }
    if (rx >= 0 && rx < isize())
    {
      fokusPk = at(rx).Pk;
    }
  }
  static void clearTags()
  {
    for (auto& elem : s_Array)
    {
      elem.m_Tag = false;
    }
  }
  static std::vector<TRow>& array() { return s_Array; }
  void setTag()
  {
    m_Tag = true;
  }
};

// Use this makro to define a table by initializing the static CTable members.
// Column-caption separator is '|'. The number of captions also determines the number of columns.
#define DECLARE_TABLE(className, tableName, columnCaptions) \
  template <> std::vector<className> CTable<className>::s_Array{}; \
  template <> string CTable<className>::s_Name(tableName); \
  template <> QTableView* CTable<className>::s_TableView{}; \
  template <> QString CTable<className>::s_ColumnCaptions{columnCaptions}; \
  template <> int CTable<className>::s_SortColumn(-1); \
  template <> CQTableModel<className>* CTable<className>::s_TableModel{new CQTableModel<className>};

#endif
