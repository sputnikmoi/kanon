#ifndef STRUTIL_H
#define STRUTIL_H
#include <string>
#include <vector>

using std::string;

string appendChar(const string&, size_t len, int ch = ' ');
int    cmpmemi(const void*, const void*, int numByte);
int    cmpstri(const string&, const string&);
bool   contains(const string& str, const string& text);
string dropLower(const string& str);
bool   endsWith(const string&, const string& end);
string extractFilename(const string&);
string extractPath(const string& pathname, bool allowEmpty = false);
string::size_type findToken(const string& str, const size_t& start, const string& token);
bool   getDiff(const string& s1, string& more1, const string& s2, string& more2);
string getLine(string& remainder, int tabWidth = -1, size_t maxLineWidth = 0xFFFFFFFF);
string getToken(const string& source, size_t& index,
  const string& delim = " \t\r\n,;", const string& ignore = "", bool wantDelim = false);
bool   hasExtension(const string& name, const string& extension);
string quote(const string& str);
string replace(const string& str, const string& old, const string& repText, int maxNum = 1, bool token = false);
string replace_first(const string& str, const string& old, const string& repText);
string right(const string& str, size_t numCh);
void   setFileExtension(string& newName, const string& name, const string& extension);
string setFileExtension(const string& filename, const string& extension);
string setLastChar(const string&, int ch = '/');
void   split(std::vector<string>& result, const string& source, int sep, bool checkEscape = false);
bool   startsWith(const string&, const string& start);
string toAscii(const std::wstring& wstr);
string toLower(const string&);
string toString(const char* format, ...);
string toString(double, const char* format="%g");
string toString(int, const char* format="%d");
string toString(unsigned, const char* format="%u");
#ifndef __GNUC__
string toString(_int64, const char* format="%I64d");
#endif
string toUpper(const string&);
string trimWhite(const string&);
string unQuote(const string&, int chQuote='"');

#endif
