
#include <cassert>
#include <cstdio>
#include <QtGui/QKeyEvent>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableView>
#include "CDlgInput.h"
#include "CGlyph.h"
#include "CGuiMatrix.h"
#include "CModelData.h"
#include "CMmlWdgtRow.h"
#include "CNumerics.h"
#include "CWndMain.h"
#include "CXmlCreator.h"
#include "strutil.h"
#include "Util.h"

using std::string;

namespace
{
	enum { colIndex, colDimension, colFormula, colComment, colCount };
	const int MaxNumTerm{40};
	static CGuiMatrix* s_GuiMatrix;
	class CWaitCursor
	{
	public:
		CWaitCursor()
		{
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		}
		~CWaitCursor()
		{
			QApplication::restoreOverrideCursor();
		}
	};
	QString toolTipExtraTerm()
	{
		return QString(
			"The canonical dimensions of coordinates,\n"
			"fields and the coupling constant are determined from\n"
			"the first " + QString::number(model().modelOrder()) + " rows.\n"
			"This extra row might be relevant, irrelevant or marginal.\n"
			"To use this term directly you might draw it upwards."
			);
	}
}

const unsigned CGuiMatrix::s_BgColorExtra {0xF0F5FF}; // Bg color of extra terms
const unsigned CGuiMatrix::s_BgColorNormal{0xFFFFED}; // Bg color of normal terms

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CItemModel : public QAbstractTableModel
{
	const std::vector<CMmlWdgtRow*>& m_Rows;
public:
	CItemModel(const std::vector<CMmlWdgtRow*>& rows)
		: QAbstractTableModel()
		, m_Rows(rows)
	{}
	int columnCount(const QModelIndex&) const override { return colCount; }
	int rowCount(const QModelIndex& = QModelIndex()) const override
	{
		return MaxNumTerm;
	}
	QVariant headerData (int col, Qt::Orientation orientation, int role) const override
	{
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		{
			return QVariant();
		}
		switch (col)
		{
		case colIndex: return "";
		case colDimension: return "[g]";
		case colFormula: return "Terms of the Lagrangian";
		case colComment: return "Comment";
		default: return "";
		}
	}
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
	{	// @return data and attributes for the table cells
		const int col{index.column()};
		const int row{index.row()};
		const int rxInteraction{guiMatrix().getRxInteraction()};
		const bool rxSingular{guiMatrix().isRxInteractionSingular()};
		const bool hasCoupConst{!guiMatrix().isDotRow(row) && (row == rxInteraction || row >= int(model().modelOrder()))};
		const bool isNormalRow{row < int(model().modelOrder())};
		switch (role)
		{
		case Qt::DisplayRole:
			if (index.isValid()) switch (col)
			{
			case colComment:
				return guiMatrix().comment(row).c_str();
			case colDimension:
				if (hasCoupConst)
				{	// Unicode!
					return model().printDimensionOfCouplingConst(row);
				}
				return row < int(guiMatrix().numTerm()) ? "0" : "";
			case colFormula: return "";
			case colIndex: return QString::number(1 + row);
			default: return "??";
			}
			break;
		case Qt::BackgroundRole:
			{
				if (col == colIndex && isNormalRow && row == rxInteraction && rxSingular)
				{
					return QColor(Qt::red);
				}
				return QColor(isNormalRow ? CGuiMatrix::s_BgColorNormal : CGuiMatrix::s_BgColorExtra);
			}
		case Qt::ToolTipRole:
			if (col == colDimension)
			{
				return ("This column displays the canonical wave vector\n"
					"dimensions of the coupling constants.\n"
					"Only one of the first " + toString(int(model().modelOrder())) + " terms\n"
					"(the selected one) has a coupling constant.\n\n"
					"Terms with a negative coupling constant dimension\n"
					"of order O(1) are 'irrelevant' in critical statics or dynamics."
					).c_str();
			}
			else if (col == colIndex)
			{
				if (row < int(model().modelOrder()))
				{
					if (row == rxInteraction)
					{
						return QString(rxSingular ? "This row as an interaction gives a singular matrix."
							: "This row is selected as interaction.");
					}
					return QString("Click to select row as interaction");
				}
				else if (!guiMatrix().isDotRow(row))
				{
					return toolTipExtraTerm();
				}
			}
			else if (col == colComment && guiMatrix().comment(row).empty())
			{
				return QString("Click to edit the row comment.");
			}
			else if (col == colFormula && !isNormalRow && !guiMatrix().isDotRow(row))
			{
				return toolTipExtraTerm();
			}
			break;
		case Qt::FontRole:
			if (col == colIndex && row == rxInteraction)
			{
				QFont fnt;
				fnt.setBold(true);
				fnt.setItalic(true);
				return fnt;
			}
			break;
		default: break;
		} // switch(role)
		return QVariant(); 
	}
	Qt::ItemFlags flags(const QModelIndex& /*index*/) const override
	{
		return Qt::ItemIsEnabled;
	}
	void updateCells()
	{	// Triggers an update of all cells.
		dataChanged(index(0, 0), index(model().numTerm() - 1, colCount - 1));
	}
};

/* CLASS DECLARATION **********************************************************/
/**
  To get currentChanged() events.
*******************************************************************************/
class CMatrixTableView : public QTableView
{
	QTimer* m_Timer;
public:
	CMatrixTableView(QWidget* parent, QTimer* timer) : QTableView(parent), m_Timer(timer)
	{
	}
protected:
	void currentChanged(const QModelIndex& current, const QModelIndex& prev)
	{
		QTableView::currentChanged(current, prev);
		guiMatrix().setRxInteraction(current.row());
		guiMatrix().determineCriticalDimension();
		guiMatrix().updateMml();
	}
	void wheelEvent(QWheelEvent* ev) override
	{
		QTableView::wheelEvent(ev);
		m_Timer->start(std::chrono::milliseconds(1000));
	}
};

/* FUNCTION *******************************************************************/
/**
@return Singleton instance
*******************************************************************************/
CGuiMatrix& guiMatrix()
{
	return *s_GuiMatrix;
}

/* METHOD *********************************************************************/
/**
  Ctor
@param  parent:
@param loOuter:
*******************************************************************************/
CGuiMatrix::CGuiMatrix(QBoxLayout* loOuter, CWndMain* wndMain)
	: m_ItemModel(new CItemModel(m_Rows))
	, m_WndMain(wndMain)
	, m_RxInteractionSingular()
	, m_RxInteraction()
	, m_Timer(new QTimer(this))
	, m_TableView(new CMatrixTableView(wndMain, m_Timer))
	, m_Rows()
{
	throwAssert("CGuiMatrix singleton", s_GuiMatrix == 0);
	s_GuiMatrix = this;
	QGridLayout* loGrid{new QGridLayout};
	loOuter->addLayout(loGrid);
	loGrid->addWidget(m_TableView);
	m_TableView->setModel(m_ItemModel);
	m_TableView->setColumnWidth(colIndex, 30);
	m_Timer->setSingleShot(true);
	clear();
	addEmptyRow();
	updateMml();
	connect(m_TableView, SIGNAL(pressed(const QModelIndex&)), this, SLOT(onTableClick(const QModelIndex&)));
	connect(m_TableView->verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(onScroll()));
	connect(m_Timer, SIGNAL(timeout()), this, SLOT(onScroll()));
}

/* METHOD *********************************************************************/
/**
  Clears all rows.
*******************************************************************************/
void CGuiMatrix::clear()
{
	for (size_t ix{}; ix < m_Rows.size(); ix++)
	{	// Do not delete: widgets remain in the QTableView.
		m_Rows[ix]->clear();
	}
	m_Rows.clear();
	for (int rx{}; rx < m_ItemModel->rowCount(); rx++)
	{
		m_TableView->setIndexWidget(m_TableView->model()->index(rx, colFormula), NULL);
		m_TableView->setRowHidden(rx, true);
	}
}

/* METHOD *********************************************************************/
/**
  Performs calculations, displays results.
Uses m_RxInteraction as interaction term.
*******************************************************************************/
void CGuiMatrix::determineCriticalDimension()
{
	double critDim{-1};
	int rank{-1};
	const bool isCritical{model().determineCritDim(critDim)};
	m_RxInteractionSingular = false;
	if (isCritical)
	{
		try
		{
			model().determineCanonicalDimensions(m_RxInteraction);
		}
		catch (const std::exception&)
		{
			m_RxInteractionSingular = true;
		}
	}
	else
	{
		rank = CNumerics::determineRank(model());
	}
	if (m_WndMain)
	{
		m_WndMain->displayCritDim(isCritical, critDim, rank);
	}
	updateMml();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGuiMatrix::updateMml()
{
	if (CModelData::isLoadingFile())
	{	// Optimization. updateMml() to be called when done.
		return;
	}
	bool haveMore{};
	for (size_t rx{}; rx < m_Rows.size(); rx++)
	{
		if (!haveMore)
		{
			m_Rows.at(rx)->updateMml();
			m_Rows.at(rx)->setVisible(true);
			haveMore = m_Rows.at(rx)->isDotRow();
		}
		else
		{
			m_Rows.at(rx)->setVisible(false);
		}
		m_Rows.at(rx)->updateToolTip();
	}
	m_ItemModel->updateCells();
	m_TableView->resizeColumnToContents(colFormula);
}

/* METHOD *********************************************************************/
/**
  Adds a CMmlWdgtRow to the layout (takes ownership).
@param mmlWdgt:
*******************************************************************************/
void CGuiMatrix::addRow(CMmlWdgtRow* mmlWdgtRow)
{
	if (m_Rows.size() >= size_t(MaxNumTerm))
	{
		delete mmlWdgtRow;
		return;
	}
	m_Rows.push_back(mmlWdgtRow);
	const int row{int(m_Rows.size() - 1)};
	m_TableView->setRowHeight(row, 2.7*mmlWdgtRow->baseFontPointSize()); // ToDo: Font
	m_TableView->setIndexWidget(m_TableView->model()->index(row, colFormula), mmlWdgtRow);
	m_TableView->horizontalHeader()->setStretchLastSection(true);
	m_TableView->setRowHidden(row, false);
}

/* METHOD *********************************************************************/
/**
  Adds a (deserialized) CMmlWdgtRow/monomial to the model.
@param formula:
@param     row:
*******************************************************************************/
void CGuiMatrix::addRow(const CFormula& formula, size_t row)
{
	throwAssert(toString("Invalid matrix row %u", row), row < m_Rows.size());
	CWaitCursor hourGlass;
	m_Rows[row]->setFormula(formula);
}

/* METHOD *********************************************************************/
/**
  The last row displays "..." (==allows adding a new row)
*******************************************************************************/
void CGuiMatrix::addEmptyRow()
{
	if (m_Rows.size() < size_t(MaxNumTerm))
	{
		for (size_t rx{}; rx < m_Rows.size(); rx++)
		{
			if (m_Rows[rx]->isDotRow())
			{
				return;
			}
		}
		CMmlWdgtRow* row{new CMmlWdgtRow(m_TableView)};
		for (size_t ix{}; ix < 3; ix++)
		{
			row->addToFormula(new CGlyphNeutral(dot, true));
		}
		row->setIsDotRow();
		addRow(row);
	}
}

/* METHOD *********************************************************************/
/**
@param mmlWdgtRow: To delete
*******************************************************************************/
void CGuiMatrix::deleteCurrentRow()
{
	const size_t row{static_cast<size_t>(currentRow())};
	if (row < numTerm())
	{
		for (size_t jx{row}; jx + 1 < m_Rows.size(); jx++)
		{
			m_Rows[jx]->assign(m_Rows[jx + 1]);
		}
		// Delete invalid last row.
		const size_t ixLast{m_Rows.size() - 1};
		m_TableView->setIndexWidget(m_TableView->model()->index(ixLast, colFormula), NULL);
		m_TableView->setRowHidden(ixLast, true);
		m_Rows.erase(m_Rows.begin() + ixLast);
		addEmptyRow();
		model().setDirty();
		updateMml();
		determineCriticalDimension();
	}
}

/* METHOD *********************************************************************/
/**
@param rx:
*******************************************************************************/
void CGuiMatrix::setRxInteraction(int rx)
{
	if (rx < int(model().modelOrder()))
	{
		m_RxInteraction = rx;
	}
}

/* METHOD *********************************************************************/
/**
@return true for empty row
*******************************************************************************/
bool CGuiMatrix::isDotRow(int rx) const
{
	return rx >= 0 && rx < int(m_Rows.size()) && m_Rows[rx]->isDotRow();
}

/* METHOD *********************************************************************/
/**
  Passes focus from QtMmlWidget to item in QAbstractItemView
  (what somehow isn't done by Qt).
*******************************************************************************/
void CGuiMatrix::setFocusToItem(const CMmlWdgtRow* wdgt)
{
	const int rx{getIndexOfRow(wdgt)};
	if (rx >= 0 && m_TableView)
	{
		m_TableView->setCurrentIndex(m_ItemModel->index(rx, colFormula));
	}
}

/* METHOD *********************************************************************/
/**
  Called after loading a model.
*******************************************************************************/
void CGuiMatrix::setFocusToInteractionRow()
{
	if (m_TableView)
	{
		const size_t rxInteraction{model().modelOrder() - 1};
		if (rxInteraction < model().numTerm())
		{
			m_TableView->setCurrentIndex(m_ItemModel->index(int(rxInteraction), colFormula));
		}
	}
}

/* METHOD *********************************************************************/
/**
  Permutes rows according to drag/drop action.
@param src, dst: Rows
*******************************************************************************/
void CGuiMatrix::dragDropRow(void* src, void* dst)
{
	if (src == dst)
	{
		return;
	}
	size_t ixDst{m_Rows.size()};
	size_t ixSrc{m_Rows.size()};
	for (size_t rx{}; rx < m_Rows.size(); rx++)
	{
		if (m_Rows[rx] == src)
		{
			ixSrc = rx;
		}
		if (m_Rows[rx] == dst)
		{
			ixDst = rx;
		}
	}
	const bool ok{ixSrc < m_Rows.size() && ixDst < m_Rows.size()};
	if (ok && ixDst != ixSrc)
	{	// The CMmlWdgtRow(s) are already in the layout -> exchange formula.//
		model().setDirty();
		const int step{ixDst < ixSrc ? -1 : 1};
		CMmlWdgtRow keep(nullptr);
		keep.assign(m_Rows[ixSrc]);
		for (size_t ix{ixSrc}; ix != ixDst; ix += step)
		{
			m_Rows[ix]->assign(m_Rows[ix + step]);
		}
		m_Rows[ixDst]->assign(&keep);
		for (size_t jx{}; jx < m_Rows.size(); jx++)
		{
			//m_Rows[jx]->setFocus(jx == ixDst);
			m_Rows[jx]->updateMml();
			m_Rows[jx]->updateToolTip();
		}
	}
	m_ItemModel->updateCells();
	determineCriticalDimension();
}

/* METHOD *********************************************************************/
/**
@return Index of current row
*******************************************************************************/
int CGuiMatrix::currentRow() const
{
	return m_TableView->currentIndex().row();
}

/* METHOD *********************************************************************/
/**
@return Index of row in table.
*******************************************************************************/
int CGuiMatrix::getIndexOfRow(const CMmlWdgtRow* row) const
{
	for (size_t jx{}; jx < m_Rows.size(); jx++)
	{
		if (m_Rows[jx] == row)
		{
			return int(jx);
		}
	}
	return -1;
}

std::string CGuiMatrix::comment(int row) const
{
	return size_t(row) < m_Rows.size() ? m_Rows[row]->comment() : "";
}

/* METHOD *********************************************************************/
/**
  After drag/drop...
*******************************************************************************/
void CGuiMatrix::permuteFields(const vector<size_t>& permutation)
{
	for (size_t rx{}; rx < m_Rows.size() && !m_Rows[rx]->isDotRow(); rx++)
	{
		m_Rows[rx]->permuteFields(permutation);
	}
}

/* METHOD *********************************************************************/
/**
  Writes expressions for the monomials to the Xml file.
@param xml: Destination
*******************************************************************************/
void CGuiMatrix::toXml(CXmlCreator& xml)
{
	for (size_t rx{}; rx < m_Rows.size() && !m_Rows[rx]->isDotRow(); rx++)
	{
		m_Rows[rx]->toXml(xml);
	}
}

/* METHOD *********************************************************************/
/**
@param index: 0-based
*******************************************************************************/
void CGuiMatrix::deleteCoord(size_t index)
{
	for (const auto& wdgtRow : m_Rows)
	{
		if (wdgtRow->formula().containsCoord(index))
		{
			if (yesNo(m_TableView, "The coordinate still is used in the Lagrangian.\n"
				"Delete it and remove all references?", "Delete coordinate"))
			{
				break;
			}
			return;
		}
	}
	for (auto& wdgtRow : m_Rows)
	{	// Remove references.
		wdgtRow->removeCoord(index);
	}
	m_WndMain->removeCoordFromOperator(index);
	model().removeCoord(index);
	m_WndMain->updateMml();
	determineCriticalDimension();
}

/* METHOD *********************************************************************/
/**
@param index: 0-based
*******************************************************************************/
void CGuiMatrix::deleteField(size_t index)
{
	for (const auto& wdgRow : m_Rows)
	{
		if (wdgRow->formula().containsField(index))
		{
			if (yesNo(m_TableView, "Field still used in the Lagrangian.\n"
				"Delete and remove all references?", "Delete field"))
			{
				break;
			}
			return;
		}
	}
	for (auto& wdgtRow : m_Rows)
	{	// Remove references.
		wdgtRow->removeField(index);
	}
	model().removeField(index);
	m_WndMain->updateMml();
	determineCriticalDimension();
}

size_t CGuiMatrix::numTerm() const
{
	if (m_Rows.empty())
	{
		return 0;
	}
	if (m_Rows[m_Rows.size() - 1]->isDotRow())
	{
		return m_Rows.size() - 1;
	}
	return m_Rows.size();
}

int CGuiMatrix::getExp(size_t tx, size_t ixColumn) const
{
	throwAssert("getExp(tx, ix)", tx < numTerm() && ixColumn < model().modelOrder());
	return m_Rows[tx]->getExp(ixColumn);
}

int CGuiMatrix::getExpD(size_t tx) const
{
	throwAssert("getExpD(tx", tx < numTerm());
	return m_Rows[tx]->getExpD();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGuiMatrix::showContextMenu()
{
	const int row{currentRow()};
	if (row < int(numTerm()))
	{
		enum { idDeleteRow, idEditComment };
		QMenu menu(m_TableView);
		menu.addAction("Edit comment")->setData(idEditComment);
		menu.addAction("Delete this row")->setData(idDeleteRow);
		if (QAction* act{menu.exec(QCursor::pos())}) switch (act->data().toInt())
		{
		case idDeleteRow:
			deleteCurrentRow();
			break;
		case idEditComment:
			editComment();
			break;
		default: break;
		}
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGuiMatrix::editComment()
{
	const int row{currentRow()};
	if (row < int(numTerm()))
	{
		string text(comment(row));
		CDlgInput dlg("Kanon: Comment for row " + QString::number(1 + row), m_TableView, 620);
		dlg.addTextField("Comment", &text, true);
		if (QDialog::Accepted == dlg.exec())
		{
			if (dlg.id() == 0)
			{
				model().setDirty();
				dlg.dump();
				m_Rows[row]->setComment(text);
				m_TableView->update();
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGuiMatrix::onTableClick(const QModelIndex& index)
{
	if (index.column() == colComment)
	{
		editComment();
	}
	else if (Qt::RightButton == QApplication::mouseButtons())
	{
		showContextMenu();
	}
}

void CGuiMatrix::onScroll()
{
	// After scroll some rows still need a repaint!
	updateMml();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CGuiMatrix::keyPressEvent(QKeyEvent* ev)
{
	const int key{ev->key()};
	if (key == Qt::Key_F1)
	{
		//fprintf(stderr, "f1\n");/**/
	}
	else
	{
		ev->ignore();
	}
}

