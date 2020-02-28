
#ifndef CMMLWDGTOPERATOR_H
#define CMMLWDGTOPERATOR_H

#include "CMmlWdgtBase.h"
#include "CGlyph.h"

/* CLASS DECLARATION **********************************************************/
/**
  Displays an operator (integral, nabla, ..) in the 'Operators' line.
*******************************************************************************/
class CMmlWdgtOperator : public CMmlWdgtBase
{
	ESymbol  m_Symbol;
	size_t m_Index;
public:
	CMmlWdgtOperator(QWidget* parent, ESymbol symbol, size_t index = 0);
	void setCoordIndex(size_t coordIndex);
	void deleteCoord(size_t coordIndex);
	void updateMml(bool = false) override;
protected:
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void timerEvent(QTimerEvent*) override;
private:
};

#endif

