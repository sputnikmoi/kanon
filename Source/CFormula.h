#ifndef CFORMULA_H
#define CFORMULA_H

#include <string>
#include <vector>

class CGlyphBase;
class CModelData;
class CXmlCreator;
class QDomElement;
class QPainter;

/* CLASS DECLARATION **********************************************************/
/**
  Represents a "formula". Used for a model term or the complete Lagrangian.
  Is independent of Qt.
*******************************************************************************/
class CFormula
{
  bool m_CanHaveCursor;
  bool m_CsrBlinkState;
  bool m_HasFocus;
  size_t m_CsrPos; // An index to the right of current glyph
  std::vector<CGlyphBase*> m_Formula;
  std::string m_Comment;
public:
  CFormula();
  CFormula(const CFormula&);
  CFormula& operator=(const CFormula&);
  ~CFormula();
  std::string validate(const CModelData&) const;
  void add(CGlyphBase*);
  void clear();
  int  paint(QPainter&);
  void setFocus(bool state) { m_HasFocus = state; }
  void allowCursor(bool state = true) { m_CanHaveCursor = state; } // To allow edit
  void setCsrToEnd();
  bool withCursor() const { return m_CanHaveCursor; }
  void setComment(const std::string& text) { m_Comment = text; }
  std::string comment() const { return m_Comment; }
  int getExp(size_t ixColumn, const CModelData& mod) const;
  int getExpD() const;

  void fromXml(QDomElement&);
  void toXml(CXmlCreator&) const;
  bool consumeKey(bool& dirty, int key, bool ctl);
  bool containsCoord(size_t ix) const;
  bool containsField(size_t ix) const;
  void removeCoord(size_t ix);
  void replaceCoord(size_t ixOld, size_t ixNew);
  void removeField(size_t ix);
  void permuteFields(const std::vector<size_t>& permutation);
  void toggleCsrState(bool);
};

#endif

