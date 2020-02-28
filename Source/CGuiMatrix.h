#ifndef CGUIMATRIX_H
#define CGUIMATRIX_H

#include <vector>
#include <QObject>

class CFormula;
class CItemModel;
class CMmlWdgtRow;
class CWndMain;
class CXmlCreator;
class QBoxLayout;
class QDomElement;
class QGridLayout;
class QKeyEvent;
class QModelIndex;
class QTableView;
class QTimer;
class QWidget;

/* CLASS DECLARATION **********************************************************/
/**
  Used as a singleton.
  Contains and displays rows of terms and associated data in a QGridLayout.
  The main part of the model data (field and coordinate exponents) live here.
*******************************************************************************/
class CGuiMatrix : public QObject
{
	Q_OBJECT
	CItemModel* m_ItemModel;
	CWndMain*   m_WndMain;
	bool        m_RxInteractionSingular;
	int         m_RxInteraction;
	QTimer*     m_Timer;
	QTableView* m_TableView;
	std::vector<CMmlWdgtRow*> m_Rows;
protected:
	void keyPressEvent(QKeyEvent*);
public:
	static const unsigned s_BgColorExtra;
	static const unsigned s_BgColorNormal;
	CGuiMatrix(QBoxLayout* loOuter, CWndMain*);
	void clear();
	void determineCriticalDimension();
	void updateMml();
	void addEmptyRow();
	void addRow(CMmlWdgtRow*);
	void addRow(const CFormula&, size_t row);
	void addRow(QDomElement&, size_t row, size_t numCoord);
	void deleteCurrentRow();
	void dragDropRow(void* src, void* dst);
	void setRxInteraction(int);
	bool isDotRow(int rx) const;
	void setFocusToItem(const CMmlWdgtRow*);
	void setFocusToInteractionRow();
	int  getRxInteraction() const { return m_RxInteraction; }
	bool isRxInteractionSingular() const { return m_RxInteractionSingular; }
	int  currentRow() const;
	int  getIndexOfRow(const CMmlWdgtRow*) const;
	std::string comment(int row) const;
	void permuteFields(const std::vector<size_t>& permutation);
	void toXml(CXmlCreator&);
	 
	void deleteCoord(size_t index);
	void deleteField(size_t index);
	void editComment();
	void showContextMenu();
	size_t numTerm() const;
	int  getExp(size_t tx, size_t ixColumn) const;
	int  getExpD(size_t tx) const;
private slots:
	void onTableClick(const QModelIndex&);
	void onScroll();
};

// Global singleton
CGuiMatrix& guiMatrix();

#endif

