
#include <cstdio>
#include <QtGui/QKeyEvent>
#include <QtGui/QDrag>
#include <QMimeData>
#include "CFormula.h"
#include "CGlyph.h"
#include "CMmlWdgtOperator.h"
#include "Util.h"


/* METHOD *********************************************************************/
/**
  Ctor. Creates a glyph for the 'Operators' line.
*******************************************************************************/
CMmlWdgtOperator::CMmlWdgtOperator(QWidget* parent, ESymbol symbol, size_t index)
	: CMmlWdgtBase(parent)
	, m_Symbol(symbol)
	, m_Index(index)
{
	setAcceptDrops(true);
	if (hasCoordinate(m_Symbol))
	{
		m_Formula.add(new CGlyphCoordinate(index, symbol));
	}
	else
	{
		m_Formula.add(new CGlyphNeutral(symbol));
	}
	if (symbol == integral || symbol == nabla || symbol == partial || symbol == delta)
	{
		setToolTip("- Add the operator to a term of the Lagrangian below\n  by drag and drop.\n"
			"- You might first modify the operator by\n  dropping a coordinate onto it.");
	}
	else
	{
		setToolTip("Add the symbol to a term of\nthe Lagrangian below by drag and drop.\nThis symbol is dimensionless\n(doesn't change dimensions).");
	}
	updateMml();
}

/* METHOD *********************************************************************/
/**
  Configures the operator for the coordinate (makes sense for integral, delta,
  nabla, partial).
@param coordIndex: Index within CModelData.
*******************************************************************************/
void CMmlWdgtOperator::setCoordIndex(size_t coordIndex)
{
	if (hasCoordinate(m_Symbol))
	{
		m_Index = coordIndex;
		const CGlyphCoordinate glyph(coordIndex, m_Symbol);
		m_Formula.clear();
		m_Formula.add(new CGlyphCoordinate(glyph));
	}
}

/* METHOD *********************************************************************/
/**
  Replaces reference to coordinate with reference to default coordinate.
@param coordIndex: 0-based
*******************************************************************************/
void CMmlWdgtOperator::deleteCoord(size_t coordIndex)
{
	if (hasCoordinate(m_Symbol))
	{
		m_Formula.replaceCoord(coordIndex, 0);
	}
}

/* METHOD *********************************************************************/
/**
  Xfers Mml text from m_Formula to m_WdgtMml.
*******************************************************************************/
void CMmlWdgtOperator::updateMml(bool)
{
	update();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtOperator::mouseMoveEvent(QMouseEvent*)
{
	killTimer(m_TimerId);
	m_TimerId = 0;
	// Start drag if there was substantial move
	QByteArray dataThis;
	dataThis.setNum(unsigned (100*m_Symbol + m_Index));
	QMimeData* mimeData{new QMimeData};
	mimeData->setData(MimeFormat::Operator, dataThis);
	QDrag* drag{new QDrag(this)};
	drag->setMimeData(mimeData);
	QPixmap pxmp;
	getPixmap(pxmp);
	drag->setPixmap(pxmp);
	drag->exec();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtOperator::mousePressEvent(QMouseEvent* ev)
{
	if (ev->button() == Qt::RightButton)
	{
		return;
	}
	m_TimerId = startTimer(200);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtOperator::dragEnterEvent(QDragEnterEvent* ev)
{
	if (hasCoordinate(m_Symbol) && ev->mimeData()->hasFormat(MimeFormat::Coord))
	{
		ev->acceptProposedAction();
	}
}

/* METHOD *********************************************************************/
/**
  Dropping a coordinate onto an operator (integral, nabla, partia, delta)
  converts the operator accordingly.
*******************************************************************************/
void CMmlWdgtOperator::dropEvent(QDropEvent* ev)
{
	if (ev->mimeData()->hasFormat(MimeFormat::Coord))
	{
		bool ok;
		const int coordIndex{ev->mimeData()->data(MimeFormat::Coord).toInt(&ok)};
		if (ok)
		{
			setCoordIndex(coordIndex);
			ev->acceptProposedAction();
			updateMml();
		}
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtOperator::timerEvent(QTimerEvent*)
{
	killTimer(m_TimerId);
	m_TimerId = 0;
}

