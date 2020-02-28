#include <cassert>
#include <QtGui/QKeyEvent>
#include <QDrag>
#include <QMimeData>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include "CGlyph.h"
#include "CGuiMatrix.h"
#include "CHelp.h"
#include "CMmlWdgtRow.h"
#include "CModelData.h"
#include "strutil.h"
#include "Util.h"

using std::string;
using std::vector;

CMmlWdgtRow* CMmlWdgtRow::s_DragCandidate;

/* METHOD *********************************************************************/
/**
  Ctor
*******************************************************************************/
CMmlWdgtRow::CMmlWdgtRow(QWidget* parent)
	: CMmlWdgtBase(parent)
	, m_IsDotRow(false)
{
	m_Formula.allowCursor(!m_IsDotRow);
	setAcceptDrops(true);
}

/* METHOD *********************************************************************/
/**
  Assigns internal state, widget remains unchanged.
*******************************************************************************/
void CMmlWdgtRow::assign(const CMmlWdgtRow* rhs)
{
	if (rhs)
	{
		m_IsDotRow = rhs->m_IsDotRow;
		m_Formula = rhs->formula();
		updateToolTip();
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtRow::clear()
{
	m_IsDotRow = false;
	m_Formula.clear();
}

void CMmlWdgtRow::removeCoord(size_t index)
{
	m_Formula.removeCoord(index);
}
void CMmlWdgtRow::removeField(size_t index)
{
	m_Formula.removeField(index);
}

/* METHOD *********************************************************************/
/**
  Row displays "..."
*******************************************************************************/
void CMmlWdgtRow::setIsDotRow()
{
	m_IsDotRow = true;
	m_Formula.allowCursor(false);
	updateToolTip();
}

void CMmlWdgtRow::setComment(const std::string& text)
{
	m_Formula.setComment(text);
}

std::string CMmlWdgtRow::comment() const
{
	return m_Formula.comment();
}

/* METHOD *********************************************************************/
/**
@param fx: Matrix index (coordinates, then fields)
*
@return Exponent
*******************************************************************************/
int CMmlWdgtRow::getExp(size_t ixColumn) const
{
	assert(ixColumn < model().modelOrder());
	return m_Formula.getExp(ixColumn, model());
}

int CMmlWdgtRow::getExpD() const
{
	return m_Formula.getExpD();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtRow::updateToolTip()
{
	if (m_IsDotRow)
	{
		QString text("Drag operators or fields onto\n"
			"the '...' to create another term.");
		if (model().numTerm() < model().modelOrder())
		{
			text += "\nAt least " + QString::number(model().modelOrder()) + " terms are required.";
		}
		setToolTip(text);
	}
	else
	{
		setToolTip("");
	}
}

/* METHOD *********************************************************************/
/**
  Appends glyph to formula.
@param glyph:
*******************************************************************************/
void CMmlWdgtRow::addToFormula(CGlyphBase* glyph)
{
	const bool wasDotRow{m_IsDotRow};
	if (m_IsDotRow)
	{
		m_IsDotRow = false;
		m_Formula.clear();
	}
	updateToolTip();
	m_Formula.allowCursor(!m_IsDotRow);
	m_Formula.add(glyph);
	if (wasDotRow)
	{	// Make new "..." row visible
		guiMatrix().updateMml();
		guiMatrix().addEmptyRow();
		guiMatrix().updateMml();
	}
}

/* METHOD *********************************************************************/
/**
  Appends (deserialized) glyphs to formula.
@param text:
*******************************************************************************/
void CMmlWdgtRow::setFormula(const CFormula& formula)
{
	const bool wasDotRow{m_IsDotRow};
	m_IsDotRow = false;
	m_Formula = formula;
	if (wasDotRow)
	{	// Make new "..." row visible
		guiMatrix().updateMml();
		guiMatrix().addEmptyRow();
		guiMatrix().updateMml();
	}
}

void CMmlWdgtRow::permuteFields(const vector<size_t>& permutation)
{	// After drag/drop
	m_Formula.permuteFields(permutation);
}

/* METHOD *********************************************************************/
/**
@param toggleCsrState: Set to true to toggle cursor (->blinking cursor).
*******************************************************************************/
void CMmlWdgtRow::updateMml(bool toggleCsrState)
{
	m_Formula.toggleCsrState(toggleCsrState);
	update();
}

/* METHOD *********************************************************************/
/**
  Event handler: Blinking cursor.
*******************************************************************************/
void CMmlWdgtRow::timerEvent(QTimerEvent*)
{
	updateMml(true);
}

/* METHOD *********************************************************************/
/**
  Event handler.
*******************************************************************************/
void CMmlWdgtRow::keyPressEvent(QKeyEvent* ke)
{
	const bool ctl{0 != (ke->modifiers() & Qt::ControlModifier)};
	const int key{ke->key()};
	if (!m_IsDotRow)
	{
		bool dirty{};
		if (key == Qt::Key_F1)
		{
			help().show("EditLagrangian.htm");
		}
		else if (!m_Formula.consumeKey(dirty, key, ctl))
		{
			CMmlWdgtBase::keyPressEvent(ke);
		}
		if (dirty)
		{
			model().setDirty();
			guiMatrix().determineCriticalDimension();
		}
	}
	if ((key == Qt::Key_Down || key == Qt::Key_Up
		|| key == Qt::Key_PageDown || key == Qt::Key_PageUp) && !ctl)
	{
		CMmlWdgtBase::keyPressEvent(ke);
	}
	updateMml();
}

/* METHOD *********************************************************************/
/**
  Event handler
*******************************************************************************/
void CMmlWdgtRow::mousePressEvent(QMouseEvent* ev)
{
	if (!m_IsDotRow)
	{
		if (ev->button() == Qt::RightButton)
		{
			guiMatrix().showContextMenu();
		}
		else
		{
			s_DragCandidate = this;
		}
	}
	CMmlWdgtBase::mousePressEvent(ev);
}

/* METHOD *********************************************************************/
/**
  Event handler
*******************************************************************************/
void CMmlWdgtRow::mouseMoveEvent(QMouseEvent*)
{
	if (this == s_DragCandidate)
	{
		QByteArray dataThis;
		dataThis.append(QString::asprintf("%p", this));
		QMimeData* mimeData{new QMimeData};
		mimeData->setData(MimeFormat::Row, dataThis);
		QDrag* drag{new QDrag(this)};
		drag->setMimeData(mimeData);
		QPixmap pxmp;
		getPixmap(pxmp);
		drag->setPixmap(pxmp);
		drag->exec();
	}
	s_DragCandidate = nullptr;
}

/* METHOD *********************************************************************/
/**
  Pass focus to item in QAbstractItemView (which somehow isn't done by Qt).
*******************************************************************************/
void CMmlWdgtRow::focusInEvent(QFocusEvent* ev)
{
	CMmlWdgtBase::focusInEvent(ev);
	guiMatrix().setFocusToItem(this);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtRow::dragEnterEvent(QDragEnterEvent* ev)
{
	const QMimeData* md{ev->mimeData()};
	if (md->hasFormat(MimeFormat::Field) || md->hasFormat(MimeFormat::Operator)
		|| (md->hasFormat(MimeFormat::Row) && !m_IsDotRow))
	{
		ev->acceptProposedAction();
	}
}

/* METHOD *********************************************************************/
/**
  Accepts the drop.
@param ev: Event
*******************************************************************************/
void CMmlWdgtRow::dropEvent(QDropEvent* ev)
{
	if (ev->mimeData()->hasFormat(MimeFormat::Row) && !m_IsDotRow)
	{	// Shift rows up/down.
		const string text(ev->mimeData()->data(MimeFormat::Row).toStdString());
		void* src{};
		if (1 == sscanf(text.c_str(), "%p", &src))
		{
			ev->acceptProposedAction();
			guiMatrix().dragDropRow(src, this);
			guiMatrix().setFocusToItem(this);
		}
	}
	else if (ev->mimeData()->hasFormat(MimeFormat::Field))
	{	// Append/insert field.
		bool ok;
		const int fieldIndex{ev->mimeData()->data(MimeFormat::Field).toInt(&ok)};
		if (ok)
		{
			ev->acceptProposedAction();
			addToFormula(new CGlyphField(fieldIndex));
			updateMml();
			model().setDirty();
			guiMatrix().determineCriticalDimension();
			//=======
			m_Formula.setCsrToEnd();
			CMmlWdgtBase::setFocus(true);
			guiMatrix().setFocusToItem(this);
			setFocus(true);
		}
	}
	else if (ev->mimeData()->hasFormat(MimeFormat::Operator))
	{	// Append/insert operator
		bool ok;
		const int iVal{ev->mimeData()->data(MimeFormat::Operator).toInt(&ok)};
		if (ok)
		{
			const int index{iVal % 100};
			const ESymbol symbol{static_cast<ESymbol>(iVal / 100)};
			ev->acceptProposedAction();
			if (hasCoordinate(symbol))
			{
				addToFormula(new CGlyphCoordinate(index, symbol));
			}
			else
			{
				addToFormula(new CGlyphNeutral(symbol));
			}
			updateMml();
			model().setDirty();
			guiMatrix().determineCriticalDimension();
			//=======
			m_Formula.setCsrToEnd();
			CMmlWdgtBase::setFocus(true);
			guiMatrix().setFocusToItem(this);
			setFocus(true);
		}
	}
	if (m_IsDotRow)
	{
		fprintf(stderr, "ToDo: Convert to normal row\n"); // !
		fprintf(stderr, "ToDo: Add a new row\n"); // !
	}
}

/* METHOD *********************************************************************/
/**
  Inserts test formula.
*******************************************************************************/
void CMmlWdgtRow::addTestGlyphs(int)
{
#if 0
	addToFormula(new CGlyphNeutral(bra));// .
	addToFormula(new CGlyphField(2, 2));
	addToFormula(new CGlyphNeutral(plus));// ?
	 
	addToFormula(new CGlyphField(2));
	addToFormula(new CGlyphCoordinate(1, partial, 1));
	addToFormula(new CGlyphField(3));
	addToFormula(new CGlyphNeutral(plus));// ?
	 
	addToFormula(new CGlyphField(2));
	addToFormula(new CGlyphCoordinate(0, nabla, 2));
	addToFormula(new CGlyphField(3));
	addToFormula(new CGlyphNeutral(plus));// ?
	 
	addToFormula(new CGlyphField(2));
	addToFormula(new CGlyphField(3, 3));
	addToFormula(new CGlyphNeutral(ket));// .
	 
	//--------
	addToFormula(new CGlyphNeutral(plus));// ?
	addToFormula(new CGlyphCoordinate(0, delta));
	addToFormula(new CGlyphCoordinate(1, delta));
	//addToFormula(new CGlyphNeutral(otimes));
	//addToFormula(new CGlyphNeutral(minus));
	updateMml();
#endif
}

/* METHOD *********************************************************************/
/**
  Serialializes the formula for "save in file".
@param xml: Destination
*******************************************************************************/
void CMmlWdgtRow::toXml(CXmlCreator& xml) const
{
	if (!m_IsDotRow)
	{
		m_Formula.toXml(xml);
	}
}

