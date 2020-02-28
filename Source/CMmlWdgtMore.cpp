#include <cstdio>
#include <QtGui/QKeyEvent>
#include "CDlgCoordFieldSymbol.h"
#include "CFormula.h"
#include "CGlyph.h"
#include "CMmlWdgtMore.h"


/* METHOD *********************************************************************/
/**
  Ctor. Creates a glyph for the 'Operators' line.
*******************************************************************************/
CMmlWdgtMore::CMmlWdgtMore(QWidget* parent, ECoordField type)
	: CMmlWdgtBase(parent)
	, m_Type(type)
{
	m_Formula.add(new CGlyphNeutral(dot, true));
	m_Formula.add(new CGlyphNeutral(dot, true));
	m_Formula.add(new CGlyphNeutral(dot, true));
	updateMml();
}

/* METHOD *********************************************************************/
/**
  Xfers Mml text from m_Formula to m_WdgtMml.
*******************************************************************************/
void CMmlWdgtMore::updateMml(bool)
{
	update();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtMore::keyPressEvent(QKeyEvent* ev)
{
	if (Qt::Key_Return == ev->key())
	{
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtMore::mousePressEvent(QMouseEvent*)
{
	CDlgCoordFieldSymbol dlg(this, m_Type, -1);
	if (QDialog::Accepted == dlg.exec())
	{
	}
}

