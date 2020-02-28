
#ifndef CMMLWDGTCOORDFIELD_H
#define CMMLWDGTCOORDFIELD_H
#include "CMmlWdgtBase.h"

class CWndMain;

/* CLASS DECLARATION **********************************************************/
/**
  Displays a coordinate/field (within the list of coords/fields).
*******************************************************************************/
class CMmlWdgtCoordField : public CMmlWdgtBase
{
	CWndMain* m_WndMain;
	ECoordField m_Type;
	size_t m_IndexInModel;
public:
	CMmlWdgtCoordField(CWndMain* wndMain, ECoordField type, size_t indexInModel);
	bool isCoord() const;
	QString strType() const;
	void updateFormula();
	void updateVisibility();
	bool canPermuteFields() const;
protected:
	void mouseMoveEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent*) override;
	void timerEvent(QTimerEvent*) override;
private:
	bool isMore() const;
};

#endif

