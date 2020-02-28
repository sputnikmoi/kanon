/******************************************************************************/
/**
@file         CDlgInput.h
@copyright    (c)
*
@description  Configurable input dialog
********************************************************************************
*******************************************************************************/
#ifndef CDLGINPUT_H
#define CDLGINPUT_H
#include <QtWidgets/QDialog>

/* FORWARD DECLARATIONS *******************************************************/
class QBoxLayout;
class QComboBox;
class QFormLayout;
class QLineEdit;
class QSignalMapper;

/* CLASS DECLARATIONS *********************************************************/
/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CDlgInput : public QDialog
{
  Q_OBJECT

  struct SData
  {
    int* iVal;
    std::string* sVal;
    SData(std::string* sVal, int* iVal) : iVal(iVal), sVal(sVal) {}
  };
  QBoxLayout* m_LayoutBtn;
  QBoxLayout* m_LayoutVBox; // Before m_LayoutBtn, allows to add expanding text
  QFormLayout* m_LayoutForm;
  QSignalMapper* m_SignMap;
  std::vector<QWidget*> m_Widget;
  std::vector<SData> m_Data;
  int m_Id;
  int m_Width;
public:
  CDlgInput(const QString& title, QWidget* parent,
    int widthHint = 800, const QString& textDefaultBtn = "&OK");
  QWidget* addButton(const QString& caption, int id);
  QWidget* addCheckBox(const QString& caption, std::string* data, bool enabled = true);
  QComboBox* addCombobox(const QString& caption, const QStringList& captions, std::string* data, int* index = 0);
  QWidget* addTextField(const QString& caption, std::string* data, bool multiLine = false);
  void addLabel(const QString& caption, const std::string& text);
  QWidget* appendButton(const QString& caption, int id);
  void dump() const;
  int id() const { return m_Id; }
  bool requestText(const QString& name, std::string* text);
private:
  QSize sizeHint() const;
private slots:
  void onButton(int);
};

#endif
