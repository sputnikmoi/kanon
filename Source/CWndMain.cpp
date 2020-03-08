/******************************************************************************* 
*******************************************************************************/ 
#include <cstdlib>
#include <cstdio>
#include <string>
#include <QtGui/QKeyEvent> 
#include <QSettings>
#include <QSignalMapper>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include "CDlgHtml.h" 
#include "CDlgInput.h" 
#include "CDlgSelectModel.h" 
#include "CFormula.h" 
#include "CGlyph.h" 
#include "CGuiMatrix.h" 
#include "CHelp.h"
#include "CMmlWdgtCoordField.h" 
#include "CMmlWdgtMore.h" 
#include "CMmlWdgtOperator.h" 
#include "CMmlWdgtRow.h" 
#include "CModelData.h" 
#include "CWndMain.h" 
#include "HtmlOutput.h" 
#include "strutil.h" 
#include "Util.h" 
 
using std::string; 
using std::vector; 
 
namespace 
{
	const char* g_Settings0{"rid"};
	const char* g_Settings1{"Kanon"};
	const char* g_Settings2{"RecentFileList"};
	const char* g_StrCritDim{"Crit.\ndim."};
	const unsigned MaxNumCoordField{10};
	enum
	{
		idBtnCategory,
		idBtnComment,
		idBtnHelp,
		idBtnReferences,
		idBtnResult,
		idBtnTag,
		idFileClose,
		idFileModelList,
		idFileNew,
		idFileOpen,
		idFilePrint,
		idFileSave,
		idFileSaveAs,
		idHelpAbout,
		idHelpHelp,
	};
	static QString s_DefaultDirectory("./Data");
	void addSeparator(QBoxLayout* lo)
	{
		QFrame* frm{new QFrame};
		frm->setFrameStyle(QFrame::HLine);
		lo->addWidget(frm);
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
CWndMain::CWndMain(QWidget *parent) 
	: QMainWindow(parent)
	, m_ActFileClose()
	, m_ActRecentFile()
	, m_ActSeparator()
	, m_LblCritDim()
	, m_TxtCritDim()
	, m_TxtName()
	, m_Operators()
	, m_LoCoordField()
	, m_BtnResult()
{ 
	setWindowTitle("Kanon");
	setMinimumWidth(800);
	s_DefaultDirectory = pathToData().c_str();
#ifdef __linux__
	setWindowIcon(QIcon(":/images/Kanon32.png"));
#endif
	QSignalMapper* signMap{new QSignalMapper(this)};
	CGlyphBase::initializeSymbolTable();
	CModelData::sortColumn(CModelData::colName);
	QWidget* center{new QWidget(this)};
	setCentralWidget(center);
	QBoxLayout* loV{new QVBoxLayout(center)};
	// Name & flags//
	QBoxLayout* loName{new QHBoxLayout};
	loV->addLayout(loName);
	m_TxtName = new QLineEdit(this);
	m_TxtName->setToolTip("The name of the model.");
	loName->addWidget(createLabel("&Name:", m_TxtName));
	loName->addWidget(m_TxtName);
	connect(m_TxtName, SIGNAL(textEdited(const QString&)), this, SLOT(onNameEdited(const QString&)));
	addButton(this, loName, signMap, "Category...", idBtnCategory, "Allows to classify\nthe model.");
	// Comment, References, Dimension//
	QBoxLayout* loComment{new QHBoxLayout};
	loV->addLayout(loComment);
	addButton(this, loComment, signMap, "&Comment..", idBtnComment, "Edit the comment\nfor the model.");
	addButton(this, loComment, signMap, "&References..", idBtnReferences, "Edit references.");
	addButton(this, loComment, signMap, "&Tag..", idBtnTag, "Edit user tag\n"
		"The text gets displayed in the list of models (CTL-L),\n"
		"and allows to sort or select models in way\ndefined by the user");
	m_BtnResult = addButton(this, loComment, signMap, "&Result..", idBtnResult,
		"Displays the exponent matrix and\n"
		"the canonical wave vector dimensions\n"
		"of coupling constants, coordinates and fields.");
	m_BtnResult->setEnabled(false);
	// Critical dimension//
	QFormLayout* lf{new QFormLayout};
	loComment->addLayout(lf);
	lf->addRow(m_LblCritDim = new QLabel(g_StrCritDim), m_TxtCritDim = new QLineEdit(this));
	m_TxtCritDim->setToolTip("Displays the\ncritical dimension.");
	m_TxtCritDim->setReadOnly(true);
	// Help
	addButton(this, loComment, signMap, "&Help...", idBtnHelp);
	// Coordinates + Fields//
	m_LoCoordField = new QGridLayout;
	loV->addLayout(m_LoCoordField);
	m_LoCoordField->addWidget(new QLabel("Coordinates:"), 0, 0);
	m_LoCoordField->addWidget(new QLabel("Fields:"), 1, 0);
	for (unsigned cx{}; cx < MaxNumCoordField; cx++)
	{
		m_LoCoordField->addWidget(new CMmlWdgtCoordField(this, eCoord, cx), 0, 1 + cx);
	}
	for (unsigned fx{}; fx < MaxNumCoordField; fx++)
	{
		m_LoCoordField->addWidget(new CMmlWdgtCoordField(this, eField, fx), 1, 1 + fx);
	}
	updateCoordFields();
	addSeparator(loV);
	// Operators//
	QBoxLayout* loOperators{new QHBoxLayout};
	loV->addLayout(loOperators);
	loOperators->addWidget(new QLabel("Operators:"));
	m_Operators.push_back(new CMmlWdgtOperator(this, integral));
	m_Operators.push_back(new CMmlWdgtOperator(this, nabla));
	m_Operators.push_back(new CMmlWdgtOperator(this, partial));
	m_Operators.push_back(new CMmlWdgtOperator(this, delta));
	m_Operators.push_back(new CMmlWdgtOperator(this, dgamma));
	m_Operators.push_back(new CMmlWdgtOperator(this, times));
	m_Operators.push_back(new CMmlWdgtOperator(this, otimes));
	m_Operators.push_back(new CMmlWdgtOperator(this, bullet));
	m_Operators.push_back(new CMmlWdgtOperator(this, bra));
	m_Operators.push_back(new CMmlWdgtOperator(this, ket));
	for (const auto& wdgtOp : m_Operators)
	{
		loOperators->addWidget(wdgtOp);
	}
	loOperators->addStretch(40000);
	addSeparator(loV);
	new CGuiMatrix(loV, this);
	// Menu
	createMenus(signMap);
	connect(signMap, SIGNAL(mapped(int)), this, SLOT(onSignMap(int)));
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
CWndMain::~CWndMain() 
{ 
} 
 
/* METHOD *********************************************************************/ 
/** 
  Clears model data held as data member. 
*******************************************************************************/ 
void CWndMain::clear() 
{ 
	guiMatrix().clear();
	guiMatrix().addEmptyRow();
	m_TxtName->setText("");
	for (auto& op : m_Operators)
	{   // Index 0 is d-dimensional space, exists always.
		op->setCoordIndex(0);
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
void CWndMain::createMenus(QSignalMapper* signMap) 
{ 
	QMenu* menuFile(new QMenu("&File", this));//
	menuBar()->addMenu(menuFile);
	addMenuAction(this, menuFile, "&New...", "Ctrl+N", signMap, idFileNew);
	addMenuAction(this, menuFile, "Select model from &list...", "Ctrl+L", signMap, idFileModelList);
	addMenuAction(this, menuFile, "&Open file...", "Ctrl+O", signMap, idFileOpen);
	addMenuAction(this, menuFile, "&Save", "Ctrl+S", signMap, idFileSave);
	addMenuAction(this, menuFile, "&Save As...", "", signMap, idFileSaveAs);
	m_ActFileClose = addMenuAction(this, menuFile, "&Close...", "", signMap, idFileClose);
	// Recent file list//
	menuFile->addSeparator();
	for (auto& actRecent : m_ActRecentFile)
	{
		actRecent = new QAction(this);
		actRecent->setVisible(false);
		connect(actRecent, SIGNAL(triggered()), this, SLOT(onOpenRecent()));
		menuFile->addAction(actRecent);
	}
	m_ActSeparator = menuFile->addSeparator();
	updateRecentFileActions();
	// FileExit
	QAction* exitAct{new QAction("E&xit", this)};
	exitAct->setShortcut(tr("Ctrl+Q"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
	menuFile->addAction(exitAct);
	QMenu* menuHelp(new QMenu("&Help", this));//
	menuBar()->addMenu(menuHelp);
	addMenuAction(this, menuHelp, "&Help", "", signMap, idHelpHelp);
	addMenuAction(this, menuHelp, "&About", "", signMap, idHelpAbout);
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
void CWndMain::updateRecentFileActions() 
{
	QSettings settings(g_Settings0, g_Settings1);
	QStringList files(settings.value(g_Settings2).toStringList());
	const size_t numRecentFiles{qMin(unsigned(files.size()), unsigned(eNumMaxRecentFile))};
	for (size_t fx{}; fx < numRecentFiles; ++fx)
	{
		const QString text(tr("&%1 %2").arg(fx + 1).arg(files[fx]));
		m_ActRecentFile[fx]->setText(text);
		m_ActRecentFile[fx]->setData(files[fx]);
		m_ActRecentFile[fx]->setVisible(true);
	}
	m_ActSeparator->setVisible(numRecentFiles > 0);
	for (size_t j{numRecentFiles}; j < eNumMaxRecentFile; ++j)
	{
		m_ActRecentFile[j]->setVisible(false);
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
  Updates title and recent file list. 
*******************************************************************************/ 
void CWndMain::updateTitle() 
{ 
	const QString filename(model().pathname().c_str());
	if (filename.isEmpty())
	{
		setWindowTitle(appName());
	}
	else
	{
		setWindowTitle(QString("%1 - %2").arg(filename).arg(appName()));
		QSettings settings(g_Settings0, g_Settings1);
		QStringList files(settings.value(g_Settings2).toStringList());
		files.removeAll(filename);
		files.prepend(filename);
		while (files.size() > eNumMaxRecentFile)
		{
			files.removeLast();
		}
		settings.setValue(g_Settings2, files);
		updateRecentFileActions();
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
  Displays critical dimension or matrix rank. 
@param   valid: 
@param critDim: 
@param    rank: 
*******************************************************************************/ 
void CWndMain::displayCritDim(bool valid, double critDim, int rank) 
{ 
	if (m_TxtCritDim && m_LblCritDim && m_BtnResult)
	{
		const auto order{model().modelOrder()};
		QPalette pal(m_TxtCritDim->palette());
		if (valid)
		{
			m_TxtCritDim->setText(QString::number(critDim));
			pal.setColor(QPalette::Text, Qt::black);
		}
		else
		{
			m_TxtCritDim->setText(QString("%1/%2").arg(rank).arg(order));
			pal.setColor(QPalette::Text, Qt::red);
		}
		m_LblCritDim->setText(valid ? g_StrCritDim : "Rank");
		m_TxtCritDim->setPalette(pal);
		QString ttRank(QString("Displays the rank of the exponent matrix\n"
			"as long as it is smaller than the minimal rank,\n"
			"which is %1 for %2 coordinate(s) and %3 field(s).").arg(order).arg(model().numCoord()).arg(model().numField()));
		m_TxtCritDim->setToolTip(valid ? QString("Displays the\ncritical dimension.") : ttRank);
		m_BtnResult->setEnabled(valid);
		if (m_ActFileClose)
		{
			m_ActFileClose->setEnabled(true);
		}
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
  Updates the Mml edxpressions. 
*******************************************************************************/ 
void CWndMain::updateMml() 
{ 
	for (auto& op : m_Operators)
	{
		op->updateMml();
	}
	guiMatrix().updateMml();
	updateCoordFields();
} 
 
/* METHOD *********************************************************************/ 
/** 
@param coordIndex: 
*******************************************************************************/ 
void CWndMain::removeCoordFromOperator(unsigned coordIndex) 
{ 
	for (auto& op : m_Operators)
	{
		op->deleteCoord(coordIndex);
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
  Creates label, connects to buddy 
@param  text: 
@param buddy: 
@return QLabel* 
*******************************************************************************/ 
QLabel* CWndMain::createLabel(const QString& text, QWidget* buddy) 
{ 
	QLabel* lab{new QLabel(text, this)};
	if (buddy)
	{
		lab->setBuddy(buddy);
	}
	return lab;
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
QSize CWndMain::sizeHint() const 
{ 
	return QSize(minimumWidth(), 620);
} 
 
/* METHOD *********************************************************************/ 
/** 
***********************************************************************/ 
void CWndMain::keyPressEvent(QKeyEvent* ev) 
{ 
	const int key{ev->key()};
	if (key == Qt::Key_F1)
	{
		help().show("Purpose.htm");
	}
	else
	{
		ev->ignore();
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
void CWndMain::updateCoordFields()
{
	for (int cx{}; cx < m_LoCoordField->count(); ++cx)
	{
		if (CMmlWdgtCoordField* coord{dynamic_cast<CMmlWdgtCoordField*>(m_LoCoordField->itemAt(cx)->widget())})
		{
			coord->updateFormula();
			coord->updateVisibility();
		}
	}
	for (int ix{}; ix < m_LoCoordField->columnCount(); ix++)
	{
		m_LoCoordField->setColumnStretch(ix, 1);
	}
}
 
/* METHOD *********************************************************************/
/**
  After drag/drop
*******************************************************************************/
void CWndMain::permuteFields()
{
	for (int cx{}; cx < m_LoCoordField->count(); ++cx)
	{
		if (CMmlWdgtCoordField* field{dynamic_cast<CMmlWdgtCoordField*>(m_LoCoordField->itemAt(cx)->widget())})
		{
			if (field->canPermuteFields())
			{
				field->updateFormula();
				field->updateVisibility();
				model().setDirty();
			}
		}
	}
	update();
}

/* METHOD *********************************************************************/ 
/** 
  Attempts to save the data. 
@return when OK 
*******************************************************************************/ 
bool CWndMain::makeClean()
{
	if (model().dirty())
	{
		string name(trimWhite(model().name()));
		if (name.empty())
		{
			name = "(model without a name)";
		}
		const int ret{yesNoCancel(this,
			"Data modified:\n" + name + "\n\nSave file?",
			"Please confirm"
			)};
		if (ret == QMessageBox::Yes)
		{
			if (model().pathname().empty())
			{
				const QString fileFilters("KXM files (*.kxm)");
				string pathname(QFileDialog::getSaveFileName(this, "Specify the filename",
					s_DefaultDirectory, fileFilters).toStdString());
				if (pathname.empty())
				{
					return false;
				}
				if (!hasExtension(pathname, "*"))
				{
					pathname += ".kxm";
				}
				model().setPathname(pathname);
			}
			return model().saveData(this);
		}
		else if (ret == QMessageBox::Cancel)
		{
			return false;
		}
	}
	return model().makeClean();
} 
 
/* METHOD *********************************************************************/ 
/** 
  Loads file. 
*******************************************************************************/ 
void CWndMain::fileLoad(const string& pathname)
{
	try
	{
		if (pathname.empty())
		{
			return;
		}
		clear();
		updateMml();
		s_DefaultDirectory = extractPath(pathname).c_str();
		model().loadData(pathname);
		updateTitle();
		m_TxtName->setText(model().name().c_str());
		updateMml();
		guiMatrix().setFocusToInteractionRow();
		guiMatrix().determineCriticalDimension();
		m_ActFileClose->setEnabled(true);
	}
	catch (const std::exception& e)
	{
		msgBoxCritical(e.what(), this);
	}
}
 
/* METHOD *********************************************************************/ 
/** 
@param pathNameToUse: Should be model().pathname() or empty. 
*******************************************************************************/ 
void CWndMain::fileSave(const string& pathNameToUse)
{
	if (!pathNameToUse.empty())
	{
		model().saveData(this, pathNameToUse);
		return;
	}
	const QString fileFilters("KXM files (*.kxm)");
	string pathname(QFileDialog::getSaveFileName(this,
		"Specify the filename", s_DefaultDirectory, fileFilters).toStdString());
	if (!pathname.empty())
	{
		if (!hasExtension(pathname, "*"))
		{
			pathname += ".kxm";
		}
		model().saveData(this, pathname);
		updateTitle();
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
*******************************************************************************/ 
void CWndMain::onOpenRecent()
{
	if (QAction* action{qobject_cast<QAction*>(sender())})
	{
		if (makeClean())
		{
			fileLoad(qPrintable(action->data().toString()));
		}
	}
}
 
/* METHOD *********************************************************************/ 
/** 
  Signal handler 
*******************************************************************************/ 
void CWndMain::onNameEdited(const QString& name)
{
	model().setName(name.toStdString());
	model().setDirty();
} 
 
/* METHOD *********************************************************************/ 
/** 
  Signal handler 
*******************************************************************************/ 
void CWndMain::onSignMap(int id)
{
	try
	{
		switch (id)
		{
		case idBtnTag:
			{
				CDlgInput dlg("Kanon: Define tag", this, 620);
				string text(model().category());
				dlg.addTextField("Tag", &text, false)->setToolTip(
					"This text appears in a column of\n"
					"the list of models and allows to\n"
					"sort or filter the models\n"
					"in a way defined by the user.");
				if (QDialog::Accepted == dlg.exec())
				{
					if (dlg.id() == 0)
					{
						dlg.dump();
						model().setTag(text);
					}
				}
			}
			break;
		case idBtnComment:
			{
				CDlgInput dlg("Kanon: Define comment", this, 620);
				string text(model().comment());
				dlg.addTextField("Comment", &text, true);
				if (QDialog::Accepted == dlg.exec())
				{
					if (dlg.id() == 0)
					{
						dlg.dump();
						model().setComment(text);
					}
				}
			}
			break;
		case idBtnCategory:
			{
				const QString strTt("This only is a classification,\n"
					"without any influence on the linear algebra.");
				CDlgInput dlg("Kanon: Define category", this, 620);
				string strDynamics(model().isDynamics() ? "1" : "");
				string strQm(model().isQuantumFieldTheory() ? "1" : "");
				string strReactionDiffusion(model().isReactionDiffusion() ? "1" : "");
				string strStatics(model().isStatics() ? "1" : "");
				dlg.addCheckBox("Statistical mechanics", &strStatics)->setToolTip(strTt);
				dlg.addCheckBox("Critical dynamics with a Hamiltonian", &strDynamics)->setToolTip(strTt);
				dlg.addCheckBox("Reaction diffusion equation", &strReactionDiffusion)->setToolTip(strTt);
				dlg.addCheckBox("Quantum field theory", &strQm)->setToolTip(strTt);
				if (QDialog::Accepted == dlg.exec())
				{
					if (dlg.id() == 0)
					{
						dlg.dump();
						model().setDirty();
						model().setDynamics(!strDynamics.empty());
						model().setQuantumFieldTheory(!strQm.empty());
						model().setReactionDiffusion(!strReactionDiffusion.empty());
						model().setStatics(!strStatics.empty());
					}
				}
			}
			break;
		case idBtnReferences:
			{
				CDlgInput dlg("Kanon: Define references", this, 620);
				string text(model().references());
				dlg.addTextField("References", &text, true);
				if (QDialog::Accepted == dlg.exec())
				{
					if (dlg.id() == 0)
					{
						dlg.dump();
						model().setReferences(text);
					}
				}
			}
			break;
		case idBtnResult:
			{
				string text(htmlModelOutput(guiMatrix().getRxInteraction()));
				//std::ofstream file("x.htm", std::ios::out); file << text;
				CDlgHtml dlg(text.c_str(), this);
				dlg.exec();
			}
			break;
		case idFileModelList:
			{
				CDlgSelectModel dlg(this, model().pathname());
				if (QDialog::Accepted == dlg.exec())
				{
					if (makeClean())
					{
						clear();
						updateMml();
						fileLoad(dlg.pathnameSelected());
					}
				}
			}
			break;
		case idFileNew:
			if (makeClean())
			{	// Add a default field.
				model().setGlyphCoordField(model().numField(), eField, CGlyphCoordField(phi, "Field_0"));
				clear();
				updateMml();
				guiMatrix().determineCriticalDimension();
				m_ActFileClose->setEnabled(true);
			}
			break;
		case idFileOpen:
			if (makeClean())
			{
				clear();
				updateMml();
				const QString fileFilters("KXM files (*.kxm)");
				const QString pathname(QFileDialog::getOpenFileName(this, s_DefaultDirectory, s_DefaultDirectory, fileFilters));
				fileLoad(qPrintable(pathname));
			}
			break;
		case idFileSave:
			fileSave(model().pathname());
			break;
		case idFileSaveAs:
			fileSave("");
			break;
		case idFileClose:
			if (makeClean())
			{
				model().setGlyphCoordField(model().numField(), eField, CGlyphCoordField(phi, "Field_0"));
				clear();
				updateMml();
				guiMatrix().determineCriticalDimension();
				m_ActFileClose->setEnabled(false);
			}
			break;
		case idHelpAbout:
			QMessageBox::about(this, "About ...", ("Version: " + getVersionString()).c_str());
			break;
		case idBtnHelp: [[fallthrough]];
		case idHelpHelp:
			help().show("Purpose.htm");
			break;
		}
	}
	catch (const std::exception& e)
	{
		msgBoxCritical(e.what(), this);
	}
} 
 
/* METHOD *********************************************************************/ 
/** 
  Event handler 
*******************************************************************************/ 
void CWndMain::closeEvent(QCloseEvent* ev)
{
	if (makeClean())
	{
		ev->accept();
	}
	else
	{
		ev->ignore();
	}
}

