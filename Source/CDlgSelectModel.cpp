#ifdef __linux__
# include <unistd.h>
#endif
#include <QKeyEvent>
#include <QSignalMapper>
#include <QtCore/QDir>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHeaderView>
#include "CDlgInput.h"
#include "CDlgSelectModel.h"
#include "CModelData.h"
#include "strutil.h"
#include "Util.h"

namespace
{
	enum { idClose, idCopy, idEdit, idDelete, idFilter, idOk, };
}

/* STATIC INITIALIZATION ******************************************************/
string   CDlgSelectModel::s_FilterDynamics;
string   CDlgSelectModel::s_FilterName;
string   CDlgSelectModel::s_FilterQm;
string   CDlgSelectModel::s_FilterReactionDiffusion;
string   CDlgSelectModel::s_FilterStatics;
string   CDlgSelectModel::s_FilterTag;

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CDlgSelectModel::CDlgSelectModel(QWidget* parent, const string& pathnameToFocus)
	: CDlgSelectBase(parent, "Kanon: Predefined models", CModelData::getTableModel(), 0xF8FFFF)
	, m_UsePathnameToFocus(true)
	, m_PathnameToEdit()
	, m_PathnameToFocus(pathnameToFocus)
{
	CModelData::setTableView(tableView()); // To keep focus during sort
	QSignalMapper* signMap{new QSignalMapper(this)};
	addButton(signMap, "&Edit", idEdit, "Edit selected model.");
	addButton(signMap, "&Delete..", idDelete, "Delete selected model.");
	addButton(signMap, "&Copy..", idCopy, "Copy selected model");
	addButton(signMap, "&Filter..", idFilter, "Restrict list by name\nand attributes.");
	connect(signMap, SIGNAL(mapped(int)), this, SLOT(onSignMap(int)));
	connect(tableView(), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onEdit(const QModelIndex&)));
	readModelFiles();
	resizeColumns();
	applyFilter();
	//	setWindowIcon(QIcon(":/images/Param32.png"));
	m_TableView->setColumnWidth(CModelData::colNumCoord, 85);
	m_TableView->setColumnWidth(CModelData::colNumField, 75);
	m_TableView->setColumnWidth(CModelData::colOrder, 75);
	m_TableView->setColumnWidth(CModelData::colDimension, 95);
	m_TableView->setColumnWidth(CModelData::colNormalVect, 115);
	m_TableView->setColumnWidth(CModelData::colTag, 90);
	m_TableView->setColumnWidth(CModelData::colName, 260);
	m_TableView->setColumnWidth(CModelData::colFlags, 60);
	m_TableView->horizontalHeader()->setStretchLastSection(true);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgSelectModel::readModelFiles()
{
	CModelData::clear();
	QDir dir(pathToData().c_str(), "*.kxm");
	dir.setFilter(QDir::Files);
	dir.setSorting(QDir::Name);
	const QFileInfoList fileInfo(dir.entryInfoList());
	for (int fx{}; fx < fileInfo.size(); ++fx)
	{
		const string filename(fileInfo.at(fx).absoluteFilePath().toStdString());
		CModelData data;
		try
		{
			data.loadData(filename);
		}
		catch (const std::exception& e)
		{
			msgBoxCritical(e.what(), this);
			continue;
		}
		CModelData& mod(CModelData::at(CModelData::size() - 1));
		double critDim;
		if (!mod.determineCritDim(critDim))
		{
			continue;
		}
		for (size_t tx{}; tx < mod.modelOrder(); tx++)
		{	// Attempt terms as interaction until OK.
			try
			{
				if (mod.determineCanonicalDimensions(tx))
				{
					mod.determineNormalVector();
					break;
				}
			}
			catch (...)
			{	// Ignore
			}
		}
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgSelectModel::applyFilter()
{
	CModelData::updateView();
	size_t numVisible{};
	int rowToFocus{-1};
	CModelData::sort(-1);
	for (size_t px{}; px < CModelData::size(); px++)
	{
		const CModelData& mod(CModelData::at(px));
		const bool show{
			stringMatchesFilter(s_FilterName, mod.name(), eCaseInsensitive)
			&& stringMatchesFilter(s_FilterTag, mod.category(), eCaseInsensitive)
			&& (s_FilterDynamics.empty() || mod.isDynamics())
			&& (s_FilterQm.empty() || mod.isQuantumFieldTheory())
			&& (s_FilterReactionDiffusion.empty() || mod.isReactionDiffusion())
			&& (s_FilterStatics.empty() || mod.isStatics())
			};
		CModelData::setRowHidden(px, !show);
		if (show)
		{
			numVisible++;
			if (!m_PathnameToFocus.empty() && m_PathnameToFocus == mod.pathname())
			{	// Select edited model
				m_PathnameToFocus.clear();
				rowToFocus = px;
			}
		}
	}
	if (m_UsePathnameToFocus)
	{
		m_UsePathnameToFocus = false;
		if (rowToFocus < 0)
		{
			rowToFocus = 0;
		}
	}
	CModelData::updateView();
	resizeRows();
	updateTitle(numVisible, s_FilterName.empty() ? QString() : QString(", Filter: ") + s_FilterName.c_str());
	if (rowToFocus >= 0)
	{
		m_TableView->selectRow(rowToFocus);
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
QSize CDlgSelectModel::sizeHint() const
{
	return QSize(900, 620);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgSelectModel::onEdit(const QModelIndex& index)
{
	const int row{index.row()};
	if (row >= 0 && index.isValid())
	{
		m_PathnameToEdit = CModelData::at(row).pathname();
		accept();
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgSelectModel::onSignMap(int id)
{
	const QModelIndex index{currentIndex()};
	switch (id)
	{
	case idCopy:
		if (index.isValid())
		{
			const int row{index.row()};
			const CModelData& modSrc(CModelData::at(row));
			if (modSrc.pathname().empty())
			{
				return;
			}
			string strFilename;
			string strName(modSrc.name() + "_Copy");
			for (;;)
			{	// Allow retry//
				const int idCancel{7};
				CDlgInput dlg("Kanon: Copy a model", this, 620);
				dlg.addButton("Cancel", idCancel)->setToolTip("Abort");
				dlg.addTextField("Name of new model", &strName, false)->setToolTip("Enter a name\nidentifying the model.");
				dlg.addTextField("Filename", &strFilename, false)->setToolTip("Filename without path\n"
					"and extension.");
				if (QDialog::Accepted == dlg.exec() && dlg.id() == 0)
				{
					dlg.dump();
					strFilename = trimWhite(strFilename);
					if (strFilename.empty() || hasExtension(strFilename, "*"))
					{
						msgBox(this, "Invalid filename.\nPlease enter a name "
							"without path and extension.");
					}
					else
					{ // Attempt to save
						CModelData modDst(modSrc);
						modDst.setName(strName);
						strFilename = pathToData() + "/" + strFilename + ".kxm";
						if (QFile(strFilename.c_str()).exists())
						{
							msgBox(this, ("The file already exists:\n" + strFilename).c_str());
						}
						else if (!modDst.saveData(this, strFilename))
						{
							msgBox(this, ("Cannot write to:\n" + strFilename).c_str());
						}
						else
						{	// Success
							const QDir dir(pathToData().c_str());
							strFilename = extractFilename(strFilename);
							m_PathnameToFocus = dir.absoluteFilePath(strFilename.c_str()).toStdString();
							m_UsePathnameToFocus = true;
							readModelFiles();
							applyFilter();
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
			 
		}
		break;
	case idDelete:
		{
			if (index.isValid())
			{
				const int row{index.row()};
				const CModelData& mod{CModelData::at(row)};
				if (mod.pathname().empty())
				{
					return;
				}
				if (yesNo(this, "Delete model '" + mod.name() +
					"'\nThis will delete the file\n\n" + mod.pathname() + "\n"
					, "Delete"))
				{
					if (0 == unlink(mod.pathname().c_str()))
					{
						int rowFocus{row + 1};
						if (rowFocus >= int(CModelData::size()))
						{	// Take predecessor
							rowFocus -= 2;
						}
						if (size_t(rowFocus) < CModelData::size())
						{
							m_UsePathnameToFocus = true;
							m_PathnameToFocus = CModelData::at(rowFocus).pathname();
						}
						readModelFiles();
						applyFilter();
					}
					else
					{
						msgBox(this, ("Cannot delete file\n   " + mod.pathname()).c_str());
					}
				}
			}
		}
		break;
	case idEdit:
		onEdit(currentIndex());
		break;
	case idOk:
		accept();
		break;
	case idFilter:
		onFilter();
		break;
	case idClose:
		reject();
		break;
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgSelectModel::onFilter()
{
	const int idClear{7};
	CDlgInput dlg("Kanon: Define filter", this, 620);
	dlg.addButton("Clear filter", idClear)->setToolTip("Remove all filters.");
	dlg.addTextField("Name filter", &s_FilterName, false)->setToolTip(
		"Only display models with\nthis text in the name.");
	dlg.addTextField("Tag filter", &s_FilterTag, false)->setToolTip
		("Only display models with\nthis text in the tag.");
	dlg.addCheckBox("Statistical Mechanics", &s_FilterStatics)->setToolTip("Select models with this attribute.");
	dlg.addCheckBox("Critical dynamics with a Hamiltonian", &s_FilterDynamics)->setToolTip("Select critical dynamics models.");
	dlg.addCheckBox("Reaction diffusion equations", &s_FilterReactionDiffusion)->setToolTip("Select reaction diffusion equations.");
	dlg.addCheckBox("Quantum field theory", &s_FilterQm)->setToolTip("Select quantum field theories.");
	if (QDialog::Accepted == dlg.exec())
	{
		if (dlg.id() == 0)
		{
			dlg.dump();
		}
		else
		{
			s_FilterDynamics.clear();
			s_FilterName.clear();
			s_FilterQm.clear();
			s_FilterReactionDiffusion.clear();
			s_FilterStatics.clear();
			s_FilterTag.clear();
		}
		applyFilter();
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgSelectModel::keyPressEvent(QKeyEvent* ev)
{
	const bool ctl{bool(ev->modifiers() & Qt::ControlModifier)};
	const int ascii{ev->text().isEmpty() ? 0 : ev->text()[0].toLatin1()};
	if (ctl && ascii == 'F' - '@')
	{
		onFilter();
	}
	else if (ascii == '\e')
	{
		close();
	}
}

