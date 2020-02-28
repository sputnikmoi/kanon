#include <cstdio>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include "CDlgCoordFieldSymbol.h"
#include "CGlyph.h"
#include "CModelData.h"
#include "Util.h"

using std::string;

/* METHOD *********************************************************************/
/**
  Ctor
@param       parent:
@param         type: Coordinate or field?
@param indexInModel: Index in CModelData
*******************************************************************************/
CDlgCoordFieldSymbol::CDlgCoordFieldSymbol(QWidget* parent, ECoordField type, size_t indexInModel)
	: QDialog(parent)
	, m_IsCoordinate(type == eCoord)
	, m_StrType(type == eCoord ? "coordinate" : "field")
	, m_IndexInModel(indexInModel)
	, m_Glyph()
	, m_TxtComment(new QTextEdit(this))
	, m_CmbSymbol(new QComboBox(this))
	, m_CmbSuffix()
	, m_ChkBold()
	, m_ChkPrimed()
	, m_ChkTilde()
	, m_BtnOk()
	, m_BtnCancel()
{
	const bool exists{(m_IsCoordinate && indexInModel < model().numCoord())
		|| (!m_IsCoordinate && indexInModel < model().numField())};
	const QString prefix(exists ? "Define " : "Add a ");
	const QString postFix(exists ? "" : " to the model");
	setWindowTitle(prefix + m_StrType + postFix);
	 
	QBoxLayout* loV{new QVBoxLayout(this)};
	loV->setMargin(5);
	 
	QGridLayout* loGrid{new QGridLayout};
	loV->addLayout(loGrid);
	 
	loGrid->addWidget(createLabel("&Symbol", m_CmbSymbol), 0, colSymbol);
	if (!m_IsCoordinate)
	{
		addToCmb(chi);
		addToCmb(lambda);
		addToCmb(phi);
		addToCmb(Phi);
		addToCmb(pi);
		addToCmb(psi);
		addToCmb(Psi);
		addToCmb(rho);
		addToCmb(sigma);
		addToCmb(a_);
		addToCmb(A_);
		addToCmb(b_);
		addToCmb(B_);
		addToCmb(c_);
		addToCmb(f_);
		addToCmb(F_);
		addToCmb(h_);
		addToCmb(j_);
		addToCmb(m_);
		addToCmb(M_);
		addToCmb(n_);
		addToCmb(N_);
		addToCmb(p_);
		addToCmb(q_);
		addToCmb(r_);
	}
	else
	{
		addToCmb(x_);
		addToCmb(t_);
		addToCmb(y_);
		addToCmb(z_);
		addToCmb(h_);
		addToCmb(l_);
		addToCmb(s_);
		addToCmb(u_);
		addToCmb(v_);
		addToCmb(w_);
	}
	loGrid->addWidget(m_CmbSymbol, 1, colSymbol);
	m_CmbSymbol->setToolTip("The mathematical symbol\nrepresenting the " + m_StrType + ".\n\n"
		"Internally " + m_StrType + "s are identified by an index.\n"
		"If you select a symbol here then the symbol\n"
		"consistently also changes in the Langrangian."
		);
	if (m_IsCoordinate)
	{
		m_ChkPrimed = new QCheckBox("", this);
		loGrid->addWidget(createLabel("&Primed", m_ChkPrimed), 0, colSuffix);
		loGrid->addWidget(m_ChkPrimed, 1, colSuffix);
	}
	else
	{
		m_CmbSuffix = new QComboBox(this);
		loGrid->addWidget(createLabel("Su&ffix", m_CmbSuffix), 0, colSuffix);
		loGrid->addWidget(m_CmbSuffix, 1, colSuffix);
		m_CmbSuffix->addItem("(none)", QVariant(-1));
		for (int ix{}; ix < 10; ix++)
		{
			m_CmbSuffix->addItem(QString::number(ix), QVariant(ix));
		}
		m_CmbSuffix->setToolTip("The " + m_StrType + " may\nhave a numerical suffix.");
		loGrid->addWidget(createLabel("&Tilde", m_ChkTilde = new QCheckBox("", this)), 0, colTilde);
		loGrid->addWidget(m_ChkTilde, 1, colTilde);
		m_ChkTilde->setToolTip("The field may have a tilde.");
		loGrid->addWidget(createLabel("&Bold", m_ChkBold = new QCheckBox("", this)), 0, colBold);
		loGrid->addWidget(m_ChkBold, 1, colBold);
		m_ChkBold->setToolTip("The field may be displayed in bold.");
		loGrid->setColumnStretch(colBold + 1, 4);
	}
	loV->addSpacing(10);
	loV->addWidget(createLabel("&Comment", m_TxtComment));
	loV->addWidget(m_TxtComment);
	m_TxtComment->setToolTip("Describes the " + m_StrType + ".");
	// Set data//
	if (m_IsCoordinate)
	{
		if (m_IndexInModel< model().numCoord())
		{
			m_Glyph = model().glyphCoord(m_IndexInModel);
		}
		else
		{
			m_Glyph.setSymbol(y_);
		}
	}
	else
	{
		if (m_IndexInModel < model().numField())
		{
			m_Glyph = model().glyphField(m_IndexInModel);
		}
		else
		{
			m_Glyph.setSymbol(phi);
		}
	}
	m_TxtComment->setText(m_Glyph.comment().c_str());
	const SCoordFieldAttributes attrs(m_Glyph.attrs());
	if (m_ChkBold)
	{
		m_ChkBold->setChecked(attrs.m_Bold);
	}
	if (m_ChkPrimed)
	{
		m_ChkPrimed->setChecked(attrs.m_Primed);
	}
	if (m_ChkTilde)
	{
		m_ChkTilde->setChecked(attrs.m_Tilde);
	}
	for (int ix{}; ix < m_CmbSymbol->count(); ix++)
	{
		if (attrs.m_Symb == m_CmbSymbol->itemData(ix, Qt::UserRole).toInt())
		{
			m_CmbSymbol->setCurrentIndex(ix);
			break;
		}
	}
	if (m_CmbSuffix)
	{
		m_CmbSuffix->setCurrentIndex(m_CmbSuffix->findData(attrs.m_Suffix));
	}
	loV->addSpacing(20);
	QBoxLayout* loH1{new QHBoxLayout};
	loV->addLayout(loH1);
	loH1->addWidget(m_BtnOk = new QPushButton("&OK", this));
	loH1->addWidget(m_BtnCancel = new QPushButton("&Cancel", this));
	loV->addStretch(10000);
	 
	connect(m_BtnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(m_BtnOk, SIGNAL(clicked()), this, SLOT(onOk()));
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
QLabel* CDlgCoordFieldSymbol::createLabel(const QString& text, QWidget* buddy)
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
void CDlgCoordFieldSymbol::addToCmb(int ch)
{
	m_CmbSymbol->addItem(QChar(ch), QVariant(ch));
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgCoordFieldSymbol::onCancel()
{
	done(Rejected);
}

/* METHOD *********************************************************************/
/**
  Accepts data.
*******************************************************************************/
void CDlgCoordFieldSymbol::onOk()
{
	m_Glyph.setComment(toUtf8(m_TxtComment->toPlainText()));
	m_Glyph.setBold(m_ChkBold ? m_ChkBold->isChecked() : false);
	m_Glyph.setTilde(m_ChkTilde ? m_ChkTilde->isChecked() : false);
	if (m_ChkPrimed)
	{
		m_Glyph.setPrimed(m_ChkPrimed->isChecked());
	}
	if (m_CmbSuffix)
	{
		const int suffix{m_CmbSuffix->itemData(m_CmbSuffix->currentIndex()).toInt()};
		m_Glyph.setSuffix(suffix);
	}
	const int symb{m_CmbSymbol->itemData(m_CmbSymbol->currentIndex()).toInt()};
	m_Glyph.setSymbol(static_cast<ESymbol>(symb));
	model().setGlyphCoordField(m_IndexInModel, m_IsCoordinate ? eCoord : eField, m_Glyph);
	done(Accepted);
}

