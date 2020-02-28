/*******************************************************************************
*******************************************************************************/
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>

class CGuiMatrix;
class CMmlWdgtOperator;
class CMmlWdgtRow;
class QAction;
class QBoxLayout;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSignalMapper;

/* CLASS DECLARATION **********************************************************/
/**
  QMainWindow
*******************************************************************************/
class CWndMain : public QMainWindow
{
    Q_OBJECT
public:
    CWndMain(QWidget* parent = 0);
	~CWndMain();
	void displayCritDim(bool valid, double critDim, int rank);
	void removeCoordFromOperator(unsigned coordIndex);
	void updateMml();
	void permuteFields();
protected:
    void keyPressEvent(QKeyEvent*) override;
	QSize sizeHint() const override;
	void closeEvent(QCloseEvent*) override;
private:
	enum {eNumMaxRecentFile = 12};
	QAction* m_ActFileClose;
	QAction* m_ActRecentFile[eNumMaxRecentFile];
	QAction* m_ActSeparator;
	QLabel*    m_LblCritDim;
	QLineEdit* m_TxtCritDim;
	QLineEdit* m_TxtName;
	std::vector<CMmlWdgtOperator*> m_Operators;
	QGridLayout* m_LoCoordField;
	QPushButton* m_BtnResult;

	bool makeClean();
	QLabel* createLabel(const QString&, QWidget* buddy);
	void clear();
	void fileLoad(const std::string& pathname);
	void fileSave(const std::string& pathNameToUse);
	void createMenus(QSignalMapper*);
	void startHelpProcess();
	void updateCoordFields();
	void updateRecentFileActions();
	void updateTitle();
private slots:
	void onOpenRecent();
	void onSignMap(int);
	void onNameEdited(const QString&);
};

#endif

