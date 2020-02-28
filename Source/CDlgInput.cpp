/******************************************************************************/
/**
@file         CDlgInput.cpp
@copyright    (c)
*
@description  Configurable input dialog
********************************************************************************
*******************************************************************************/
#include <QtCore/QSignalMapper>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include "CDlgInput.h"
#include "strutil.h"
#include "Util.h"

using std::string;

/* METHOD *********************************************************************/
/**
@param          title:
@param         parent:
@param      widthHint:
@param textDefaultBtn: Text of default button at bottom of dialog, or empty.
*******************************************************************************/
CDlgInput::CDlgInput(const QString& title, QWidget* parent, int widthHint,
  const QString& textDefaultBtn)
  : QDialog(parent)
  , m_LayoutBtn()
  , m_LayoutVBox()
  , m_LayoutForm()
  , m_SignMap()
  , m_Widget()
  , m_Data()
  , m_Id()
  , m_Width(widthHint)
{
  setWindowTitle(title);
  QBoxLayout* lv{new QVBoxLayout(this)};
  m_LayoutForm = new QFormLayout;
  lv->addLayout(m_LayoutForm);
  lv->addLayout(m_LayoutVBox = new QVBoxLayout);
  m_LayoutBtn = new QHBoxLayout;
  lv->addLayout(m_LayoutBtn);
  m_SignMap = new QSignalMapper(this);
  if (!textDefaultBtn.isEmpty())
  {
    ::addButton(this, m_LayoutBtn, m_SignMap, textDefaultBtn, 0)->setDefault(true);
    connect(m_SignMap, SIGNAL(mapped(int)), this, SLOT(onButton(int)));
  }
}

/* METHOD *********************************************************************/
/**
  Adds a button to the m_LayoutBtn row at the bottom.
@param idVal: Value 0 is reserved for OK button.
*******************************************************************************/
QWidget* CDlgInput::appendButton(const QString& caption, int idVal)
{
  return ::addButton(this, m_LayoutBtn, m_SignMap, caption, idVal);
}

/* METHOD *********************************************************************/
/**
  Adds a button to the m_LayoutForm.
@param idVal: Value 0 is reserved for OK button.
*******************************************************************************/
QWidget* CDlgInput::addButton(const QString& caption, int idVal)
{
  QPushButton* btn{new QPushButton(caption, this)};
  m_LayoutForm->addRow("", btn);
  m_SignMap->setMapping(btn, idVal);
  QObject::connect(btn, SIGNAL(clicked()), m_SignMap, SLOT(map()));
  return btn;
}

/* METHOD *********************************************************************/
/**
  Adds a line with a checkbox to the dialog.
@param caption:
@param    data: !empy() -> checked
*******************************************************************************/
QWidget* CDlgInput::addCheckBox(const QString& caption, string* data, bool enabled)
{
  QCheckBox* chk{new QCheckBox(this)};
  chk->setChecked(!data->empty());
  chk->setEnabled(enabled);
  m_LayoutForm->addRow(caption, chk);
  m_Widget.push_back(chk);
  m_Data.push_back(SData(data, 0));
  return chk;
}

/* METHOD *********************************************************************/
/**
  Adds a line with a combobox to the dialog.
@param  caption:
@param captions:
@param     data: Selected text
@param    index: [optional] Selected index
*******************************************************************************/
QComboBox* CDlgInput::addCombobox(const QString& caption, const QStringList& captions, string* data, int* index)
{
  QComboBox* cmb{new QComboBox(this)};
  int pos{-1};
  for (int ix{}; ix < captions.size(); ix++)
  {
    cmb->insertItem(ix, captions.at(ix), QVariant(ix));
    if (data->c_str() == captions.at(ix))
    {
      pos = ix;
    }
  }
  if (pos >= 0)
  {
    cmb->setCurrentIndex(pos);
  }
  m_LayoutForm->addRow(caption, cmb);
  m_Widget.push_back(cmb);
  m_Data.push_back(SData(data, index));
  return cmb;
}

/* METHOD *********************************************************************/
/**
  Adds a line with a QTextEdit or a QLineEdit to the dialog.
@param caption:
@param    data: [in/out]
@return Added widget
*******************************************************************************/
QWidget* CDlgInput::addTextField(const QString& caption, string* data, bool multiLine)
{
  if (multiLine)
  {
    QTextEdit* txt{new QTextEdit(this)};
    txt->setText(fromUtf8(*data));
#if 1
    m_LayoutVBox->addWidget(txt);
#else
    // Appears to be impossible to get expanding text with m_LayoutForm
    QSizePolicy policy(txt->sizePolicy());
    policy.setVerticalPolicy(QSizePolicy::Expanding);
    txt->setSizePolicy(policy);
    m_LayoutForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    m_LayoutForm->addRow(caption, txt);
#endif
    m_Widget.push_back(txt);
  }
  else
  {
    QLineEdit* txt{new QLineEdit(this)};
    txt->setText(fromUtf8(*data));
    m_LayoutForm->addRow(caption, txt);
    m_Widget.push_back(txt);
  }
  m_Data.push_back(SData(data, 0));
  if (m_Widget.size() == 1)
  {
    m_Widget[0]->setFocus();
  }
  return m_Widget.at(m_Widget.size() - 1);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgInput::addLabel(const QString& caption, const string& text)
{
  if (!text.empty())
  {
    m_LayoutForm->addRow(caption, new QLabel(text.c_str(), this));
  }
}

/* METHOD *********************************************************************/
/**
  Requests a text string
@param name: Name of requested text
@param name: [in/out] Text
*
@return true when OK pressed
*******************************************************************************/
bool CDlgInput::requestText(const QString& name, string* text)
{
  addTextField(name, text);
  if (Accepted == exec())
  {
    dump();
    return true;
  }
  return false;
}

/* METHOD *********************************************************************/
/**
  Signal handler
@param id:
*******************************************************************************/
void CDlgInput::onButton(int id)
{
  m_Id = id;
  accept();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
QSize CDlgInput::sizeHint() const
{
  return QSize(m_Width, 200);
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CDlgInput::dump() const
{
  for (size_t ix{}; ix < m_Data.size(); ix++)
  {
    if (QLineEdit* line{dynamic_cast<QLineEdit*>(m_Widget[ix])})
    {
      *m_Data[ix].sVal = toUtf8(line->text());
    }
    else if (QTextEdit* tedit{dynamic_cast<QTextEdit*>(m_Widget[ix])})
    {
      *m_Data[ix].sVal = toUtf8(tedit->toPlainText());
    }
    else if (QCheckBox* chk{dynamic_cast<QCheckBox*>(m_Widget[ix])})
    {
      *m_Data[ix].sVal = chk->isChecked() ? "1" : "";
    }
    else if (QComboBox* cmb{dynamic_cast<QComboBox*>(m_Widget[ix])})
    {
      *m_Data[ix].sVal = cmb->currentText().trimmed().toStdString();
      if (m_Data[ix].iVal)
      {
        *m_Data[ix].iVal = cmb->currentIndex();
      }
    }
  }
}
