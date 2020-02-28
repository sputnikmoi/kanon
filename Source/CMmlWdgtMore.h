
#ifndef CMMLWDGTMORE_H
#define CMMLWDGTMORE_H

#include "CMmlWdgtBase.h"
#include "CGlyph.h"

/* CLASS DECLARATION **********************************************************/
/**
  Displays an operator (integral, nabla, ..) as a symbol in the 'Operators' line.
*******************************************************************************/
class CMmlWdgtMore : public CMmlWdgtBase
{
	ECoordField m_Type;
public:
	CMmlWdgtMore(QWidget* parent, ECoordField type);
protected:
	void updateMml(bool = false) override;
	void keyPressEvent(QKeyEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
private:
};

#endif

