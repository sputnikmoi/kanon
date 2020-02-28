#ifndef CMMLWDGTROW_H
#define CMMLWDGTROW_H

#include "CFormula.h"
#include "CMmlWdgtBase.h"

class CGuiMatrix;
class CXmlCreator;
class QDomElement;
class QtMmlWidget;

/* CLASS DECLARATION **********************************************************/
/**
 GUI-counterpart of a model term.
 Displayed in CGuiMatrix column.
*******************************************************************************/
class CMmlWdgtRow : public CMmlWdgtBase
{
	bool   m_IsDotRow;
	static CMmlWdgtRow* s_DragCandidate;
	static CMmlWdgtRow* s_DraggedRow;
public:
	CMmlWdgtRow(QWidget* parent);
	void assign(const CMmlWdgtRow*);
	void clear();
	void setComment(const std::string&);
	std::string comment() const;
	void updateToolTip();
	void setIsDotRow();
	bool isDotRow() const { return m_IsDotRow; }
	int  getExp(size_t ixColumn) const;
	int  getExpD() const;
	 
	void addToFormula(CGlyphBase*);
	void setFormula(const CFormula&);
	void permuteFields(const std::vector<size_t>& permutation);
	void updateMml(bool toggleCsrState = false) override;
	void addTestGlyphs(int ix);
	const CFormula& formula() const { return m_Formula; }
	void removeCoord(size_t index);
	void removeField(size_t index);
	 
	void toXml(CXmlCreator&) const;
protected:
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent*) override;
	void focusInEvent(QFocusEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void timerEvent(QTimerEvent*) override;
};

#endif

