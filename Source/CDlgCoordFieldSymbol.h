#ifndef CDLGCOORDFIELDSYMBOL_H
#define CDLGCOORDFIELDSYMBOL_H
#include <QtWidgets/QDialog>
#include "CGlyph.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QTextEdit;

/* CLASS DECLARATION **********************************************************/
/**
  Dialog to define coordinate or field attributes.
*******************************************************************************/
class CDlgCoordFieldSymbol : public QDialog
{
	Q_OBJECT
	enum { colSymbol, colSuffix, colTilde, colBold};
	bool m_IsCoordinate;
	const QString m_StrType;
	size_t m_IndexInModel;
	 
	CGlyphCoordField m_Glyph;
	QTextEdit* m_TxtComment;
	QComboBox* m_CmbSymbol;
	QComboBox* m_CmbSuffix;
	QCheckBox* m_ChkBold;
	QCheckBox* m_ChkPrimed;
	QCheckBox* m_ChkTilde;
	QPushButton* m_BtnOk;
	QPushButton* m_BtnCancel;
	 
	QLabel* createLabel(const QString& text, QWidget* buddy);
	void addToCmb(int ch);
private slots:
	void onCancel();
	void onOk();
protected:
	QSize sizeHint() const
	{
		return QSize(450, -1);
	}
public:
	CDlgCoordFieldSymbol(QWidget* parent, ECoordField type, size_t indexInModel);
};

#endif

