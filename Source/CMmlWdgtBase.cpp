/******************************************************************************/
/**
@file         CMmlWdgtBase.cpp
@copyright
*
@description  The widget displays a CFormula.
*******************************************************************************/
#include <cstdio>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLayout>
#include "CFormula.h"
#include "CMmlWdgtBase.h"
#include "Util.h"

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CMmlWdgtBase::CMmlWdgtBase(QWidget* parent)
	: QWidget(parent)
	, m_Formula()
	, m_BaseFontPointSize(16)
	, m_TimerId()
	, m_WidthFromPaint()
	, m_WidthMin(m_BaseFontPointSize * 3.5)
{
	setFocusPolicy(Qt::WheelFocus);
	setMinimumWidth(m_WidthFromPaint);
	setMinimumHeight(m_BaseFontPointSize + 14);
}

int CMmlWdgtBase::baseFontPointSize() const
{
	return m_BaseFontPointSize;
}

CMmlWdgtBase::~CMmlWdgtBase()
{
	clearFormula();
}

QSize CMmlWdgtBase::sizeHint() const
{
	return QSize(m_WidthMin + m_WidthFromPaint, -1);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtBase::clearFormula()
{
	m_Formula.clear();
}

void CMmlWdgtBase::paintEvent(QPaintEvent* ev)
{
	QPainter p(this);
    const QRect rct(ev->rect());
    const int h0{rct.height()};
	QFont font(p.font());
	font.setPointSize(m_BaseFontPointSize);
	p.setFont(font);
    const QFontMetrics fm(p.fontMetrics());
    p.translate(0, h0/2 + fm.ascent()/3);
	m_WidthFromPaint = m_Formula.paint(p);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtBase::updateMml(bool)
{
	update();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtBase::setFocus(bool /*state*/)
{
#if 0
	m_Formula.setFocus(state);
	updateMml();
#endif
}

/* METHOD *********************************************************************/
/**
  Event handler
*******************************************************************************/
void CMmlWdgtBase::focusInEvent(QFocusEvent* ev)
{
	m_Formula.setFocus(true);
	updateMml();
	if (m_Formula.withCursor())
	{
		m_TimerId = startTimer(500);
	}
	if (ev)
	{
		QWidget::focusInEvent(ev);
	}
}

/* METHOD *********************************************************************/
/**
  Event handler
*******************************************************************************/
void CMmlWdgtBase::focusOutEvent(QFocusEvent* ev)
{
	m_Formula.setFocus(false);
	updateMml();
	if (m_Formula.withCursor())
	{
		killTimer(m_TimerId);
		m_TimerId = 0;
	}
	QWidget::focusOutEvent(ev);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtBase::getPixmap(QPixmap& pxmp, double factor)
{
	const bool withCursor{m_Formula.withCursor()};
	m_Formula.allowCursor(false);
	updateMml();
	pxmp = grab();
	m_Formula.allowCursor(withCursor);
	pxmp = pxmp.scaledToHeight(int(pxmp.height()*factor));
	pxmp.setMask(pxmp.createHeuristicMask());
}

