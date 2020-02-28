/******************************************************************************/
/**
@file         CXmlCreator.cpp
@copyright
*
@description  This class generates an XML file.
********************************************************************************
*******************************************************************************/
#include <cassert>
#include <cstdio>
#include "CXmlCreator.h"
#include "strutil.h"

/* METHOD *********************************************************************/
/**
  Opens file for write. Check success with isOpen().
@param filename:
*******************************************************************************/
CXmlCreator::CXmlCreator(const string& filename)
  : m_Filename(filename)
  , m_Level()
{
  m_File = fopen(m_Filename.c_str(), "w");
}
CXmlCreator::~CXmlCreator()
{
  close();
}

void CXmlCreator::close()
{
  if (m_File)
  {
    fclose(m_File);
    m_File = nullptr;
  }
  m_Attr.clear();
}

void CXmlCreator::write(const string& text)
{
  fprintf(m_File, "%s", text.c_str());
}

void CXmlCreator::indent(int delta)
{
  m_Level += delta;
  for(int ix{}; ix < m_Level; ix++)
  {
    fprintf(m_File,"  ");
  }
}

/* METHOD *********************************************************************/
/**
  Writes attributes stored in m_Attr.
*******************************************************************************/
void CXmlCreator::insertAttributes()
{
  for (size_t ix{}; ix + 1 < m_Attr.size(); ix += 2)
  {
    const string s1(m_Attr[ix]);
    fprintf(m_File," %s=", s1.c_str());
    const string s2(m_Attr[ix + 1]);
    fprintf(m_File,"\"%s\"", s2.c_str());
  }
  m_Attr.clear();
}

/* METHOD *********************************************************************/
/**
@return HTML text
*******************************************************************************/
string CXmlCreator::encode(const string& text)
{
  string ret;
  for (size_t ix{}; ix < text.size(); ix++)
  {
    const int ch{text[ix]};
    switch (ch)
    {
    case '<': ret += "&lt;"; break;
    case '>': ret += "&gt;"; break;
    case '&': ret += "&amp;"; break;
    case '\'': ret += "&apos;"; break;
    case '"': ret += "&quot;"; break;
    case '\r': break;
    case '\n':
      if (true)
      {
        ret += "&#10;";
      }
      else
      {
        ret += ch;
      }
      break;
    default:
      ret += ch;
      break;
    }
  }
  return ret;
}
string CXmlCreator::encodePcData(const string& text)
{
  string ret;
  for (size_t ix{}; ix < text.size(); ix++)
  {
    const int ch{text[ix]};
    if (ch != '\r')
    {
      ret += ch;
    }
  }
  return ret;
}

string CXmlCreator::bool2string(bool val)
{
  return val ? "true" : "false";
}

/* METHOD *********************************************************************/
/**
  Opens tag with optional attributes (see addAttrib()),
@param      sTag: Tag name
@param autoClose: Close the tag within the angular brackets?
*******************************************************************************/
void CXmlCreator::createTag(const string& sTag, bool autoClose)
{
  fprintf(m_File,"\n");
  indent();
  fprintf(m_File,"<%s", sTag.c_str());
  insertAttributes();
  if (autoClose)
  {
    fprintf(m_File," />");
  }
  else
  {
    fprintf(m_File,">");
    m_TagStack.push(sTag);
    m_Level++;
  }
}

/* METHOD *********************************************************************/
/**
  Cosumes the topmost m_TagStack entry.
@param sOptionalTag: Optional consistency check.
*******************************************************************************/
void CXmlCreator::closeTag(const string& sOptionalTag)
{
  fprintf(m_File,"\n");
  indent(-1);
  assert(!m_TagStack.empty());
  const string tag(m_TagStack.top());
  if (!sOptionalTag.empty())
  {
    assert(tag == sOptionalTag);
  }
  fprintf(m_File,"</%s>", tag.c_str());
  m_TagStack.pop();
}

/* METHOD *********************************************************************/
/**
  Creates <tagName/>
@param sTag: Tag name
*******************************************************************************/
void CXmlCreator::createClose(const string& sTag)
{
  createTag(sTag, true);
}

void CXmlCreator::closeTags()
{
  while(m_TagStack.size() != 0)
  {
    fprintf(m_File,"\n");
    indent(-1);
    fprintf(m_File,"</%s>", m_TagStack.top().c_str());
    m_TagStack.pop();
  }
  close();
}

/* METHOD *********************************************************************/
/**
  Opens AND closes a tag with optional attributes (see addAttrib()) and given
  data "sValue".
@param   sTag: Tag name
@param sValue: Value or empty string
*******************************************************************************/
void CXmlCreator::createChild(const string& sTag, const string& sValue)
{
  fprintf(m_File,"\n");
  indent();
  fprintf(m_File,"<%s", sTag.c_str());
  insertAttributes();
  fprintf(m_File,">%s</%s>", encode(sValue).c_str(), sTag.c_str());
}

/* METHOD *********************************************************************/
/**
  Opens AND closes a tag with optional attributes (see addAttrib()) and given
  data "sValue".
@param   sTag: Tag name
@param sValue: Value or empty string
*******************************************************************************/
void CXmlCreator::createChildPcData(const string& sTag, const string& sValue)
{
  if (!sValue.empty())
  {
    fprintf(m_File,"\n");
    indent();
    fprintf(m_File,"<%s", sTag.c_str());
    insertAttributes();
    fprintf(m_File,"><![CDATA[%s]]></%s>", encodePcData(sValue).c_str(), sTag.c_str());
  }
}

/* METHOD *********************************************************************/
/**
  Stores attributes for next tag.
@param   sKey:
@param sValue:
*******************************************************************************/
void CXmlCreator::addAttrib(const string& sKey, const string& sVal)
{
  m_Attr.push_back(sKey);
  m_Attr.push_back(encode(sVal));
}

/* METHOD *********************************************************************/
/**
  Stores attributes for next tag.
@param   sKey:
@param sValue:
*******************************************************************************/
void CXmlCreator::addAttribSkipEmpty(const string& sKey, const string& sVal)
{
  if (!sVal.empty())
  {
    m_Attr.push_back(sKey);
    m_Attr.push_back(encode(sVal));
  }
}

/* METHOD *********************************************************************/
/**
  Stores an attribute for next tag.
@param   sKey:
@param sValue:
*******************************************************************************/
void CXmlCreator::addAttribKey(const string& sKey, unsigned val, bool skipNull)
{
  if (val != 0 || !skipNull)
  {
    string text(toString(int(val), "%0x"));
    addAttrib(sKey, text);
  }
}

void CXmlCreator::addComment(const string& sComment)
{
  fprintf(m_File, "\n");
  indent();
  fprintf(m_File, "<!--%s-->",sComment.c_str());
}
