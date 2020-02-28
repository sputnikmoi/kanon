
#ifndef CMMLWDGTBASE_H
#define CMMLWDGTBASE_H

#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>
#include "CFormula.h"


class CFormula;

/* CLASS DECLARATION **********************************************************/
/**
 The widget displays a CFormula.
*******************************************************************************/
class CMmlWdgtBase : public QWidget
{
protected:
	CFormula m_Formula;
	int m_BaseFontPointSize;
	int m_TimerId;
	int m_WidthFromPaint;
	const int m_WidthMin;
	QSize sizeHint() const override;
	void focusInEvent(QFocusEvent*) override;
	void focusOutEvent(QFocusEvent*) override;
	void paintEvent(QPaintEvent*) override;
public:
	CMmlWdgtBase(QWidget* parent);
	virtual ~CMmlWdgtBase();
	const CFormula& formula() const { return m_Formula; }
	int baseFontPointSize() const;
	virtual void updateMml(bool toggleCsrState = false);
	void clearFormula();
	void getPixmap(QPixmap&, double factor = 1.0);
	void setFocus(bool state);
};

#endif

