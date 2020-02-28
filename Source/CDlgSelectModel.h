#ifndef CDLGSELECTMODEL_H
#define CDLGSELECTMODEL_H

#include "CDlgSelectBase.h"

/* FORWARD DECLARATIONS *******************************************************/
class QModelIndex;

/* CONSTANT DECLARATIONS ******************************************************/
/* CLASS DECLARATIONS *********************************************************/

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CDlgSelectModel : public CDlgSelectBase
{
	Q_OBJECT
private:
	bool m_UsePathnameToFocus;
	static std::string s_FilterDynamics;
	static std::string s_FilterName;
	static std::string s_FilterQm;
	static std::string s_FilterReactionDiffusion;
	static std::string s_FilterStatics;
	static std::string s_FilterTag;
	std::string m_PathnameToEdit;
	std::string m_PathnameToFocus;
	 
	void applyFilter();
	void onFilter();
	void readModelFiles();
public:
	CDlgSelectModel(QWidget* parent, const std::string& pathnameToFocus);
	std::string pathnameSelected() const { return m_PathnameToEdit; }
	QSize sizeHint() const override;
	void keyPressEvent(QKeyEvent*) override;

private slots:
	void onSignMap(int);
	void onEdit(const QModelIndex&);
};

#endif
