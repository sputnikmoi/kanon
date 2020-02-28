/******************************************************************************/
/**
@file         CXmlCreator.h
@copyright
*
@description  This class generates an XML file.
********************************************************************************
*******************************************************************************/
#ifndef CXMLCREATOR_H
#define CXMLCREATOR_H
#include <string>
#include <vector>
#include <stack>
using namespace std;

/* CLASS DECLARATION **********************************************************/
/**
*******************************************************************************/
class CXmlCreator
{
public:
  CXmlCreator(const string& filename);
  virtual ~CXmlCreator();
  void close();
  bool isOpen() const { return m_File != nullptr; }
  string filename() const { return m_Filename; }
  static string encode(const string& text);
  static string encodePcData(const string& text);
  static string bool2string(bool val);
  void createChild(const string& sTag, const string& sValue);
  void createChildPcData(const string& sTag, const string& sValue);
  void createTag(const string& sTag, bool autoClose = false);
  void closeTag(const string& sOptionalTag = "");
  void createClose(const string& sTag);
  void closeTags();
  void addAttrib(const string& sAttrName, const string& sAttrvalue);
  void addAttribSkipEmpty(const string& sKey, const string& sVal);
  void addAttribKey(const string& sKey, unsigned val, bool skipNull = true);
  void addComment(const string& sComment);
  void write(const string& text);
private:
  string m_Filename;
  vector<string> m_Attr;
  FILE* m_File;
  int m_Level;
  stack<string> m_TagStack;
  void indent(int delta = 0);
  void insertAttributes();
};

#endif
