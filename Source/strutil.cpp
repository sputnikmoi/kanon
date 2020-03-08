#include <cstdarg>
#include <cstdio>
#include <cstdlib> // wcstombs()
#include <cstring>
#include "strutil.h"

#define isAlNum(ch) isalnum((unsigned char)(ch))
#ifdef _WIN32
# define sPrintF(...) sprintf_s(__VA_ARGS__)
#else
# define sPrintF(...) sprintf(__VA_ARGS__)
#endif

/* FUNCTION *******************************************************************/
/**
  Appends char (blank) until string has given length
@param :
@return new string
*******************************************************************************/
string appendChar(const string& str, size_t len, int ch)
{
	string ret(str);
	for (; ret.size() < len; ret += char(ch)) {}
	return ret;
}

/* FUNCTION *******************************************************************/
/**
  Compares memory ignoring case.
@param :
*
@return 0 when equal
*******************************************************************************/
int cmpmemi(const void* m1, const void* m2, int numByte)
{
#ifndef __GNUC__
	return _memicmp(m1, m2, numByte);
#else
	for (int ix{}; ix < numByte; ix++)
	{
		const int ch1{toupper(((char*)m1)[ix])};
		const int ch2{toupper(((char*)m2)[ix])};
		if (ch1 < ch2)
		{
			return -1;
		}
		if (ch1 > ch2)
		{
			return 1;
		}
	}
	return 0;
#endif
}

/* FUNCTION *******************************************************************/
/**
  Compares strings ignoring case.
@param :
*
@return 0 when equal
*******************************************************************************/
int cmpstri(const string& s1, const string& s2)
{
#ifndef __GNUC__
	return _stricmp(s1.c_str(), s2.c_str());
#else
	return strcasecmp(s1.c_str(), s2.c_str());
#endif
}

/* FUNCTION *******************************************************************/
/**
@return true when str contains text
*******************************************************************************/
bool contains(const string& str, const string& text)
{
	return string::npos != str.find(text);
}

/* FUNCTION *******************************************************************/
/**
@return leading uppercase part of the string (useful for SCPI commands)
*******************************************************************************/
string dropLower(const string& str)
{
	string ret;
	for (const auto ch : str)
	{
		if (islower(ch))
		{
			break;
		}
		ret += ch;
	}
	return ret;
}

/* FUNCTION *******************************************************************/
/**
@return true when str ends with end
*******************************************************************************/
bool endsWith(const string& str, const string& end)
{
	return right(str, end.size()) == end;
}

/* FUNCTION *******************************************************************/
/**
@return filename (with extension)
*******************************************************************************/
string extractFilename(const string& path)
{
	const string::size_type pos{path.find_last_of(":/\\")};
	if (pos == string::npos)
	{
		return path;
	}
	if (pos + 1 >= path.size())
	{
		return "";
	}
	return path.substr(pos + 1);
}

/* FUNCTION *******************************************************************/
/**
  Extracts the path string from a pathname.
@return string (with a '/' or '\\' at the end)
*******************************************************************************/
string extractPath(const string& pathname, bool allowEmpty)
{
	const string::size_type pos{pathname.find_last_of(":/\\")};
	return pos == string::npos ? (allowEmpty ? "" : "./") : pathname.substr(0, pos + 1);
}

/* FUNCTION *******************************************************************/
/**
  Finds token in str starting at start.
@param   str:
@param start:
@param token:
*
@return pos or npos
*******************************************************************************/
string::size_type findToken(const string& str, const size_t& start, const string& token)
{
	if (!token.empty()) for (size_t pos0{start}; pos0 < str.size(); pos0++)
	{
		const string::size_type pos{str.find(token, pos0)};
		if (pos == string::npos)
		{
			break;
		}
		const int ch0{pos > 0 ? str[pos - 1] : -1};
		const int ch1{str[pos + token.size()]};
		if (!(isAlNum(ch0) || ch0 == '_') && !(isAlNum(ch1) || ch1 == '_'))
		{
			return pos;
		}
		pos0 = pos;
	}
	return string::npos;
}

/* FUNCTION *******************************************************************/
/**
  Determines characters in s1 but not in s2 and vice versa.
@param s1:
@param more1: [out]
@param s2:
@param more2: [out]
*
@return true when strings differ.
*******************************************************************************/
bool getDiff(const string& s1, string& more1, const string& s2, string& more2)
{
	more1.erase();
	more2.erase();
	for (const auto ch : s1)
	{
		if (string::npos == s2.find(ch))
		{
			more1 += ch;
		}
	}
	for (const auto ch : s2)
	{
		if (string::npos == s1.find(ch))
		{
			more2 += ch;
		}
	}
	return !more2.empty() || !more1.empty();
}

/* FUNCTION *******************************************************************/
/**
  Removes line from remainder, expands tabs to spaces, returns line.
@return line (with '\n' replaced with space).
@return line
*******************************************************************************/
string getLine(
	string& remainder,     // Input and output
	int tabWidth,          // Optional
	size_t maxLineWidth)   // Optional
{
	string line;
	size_t lineBreakRx{};  // Consumed index
	size_t lineBreakLen{}; // line length
	size_t rx{};
	for (bool done{}; rx < remainder.size(); rx++)
	{
		const int ch{remainder[rx]};
		switch (ch)
		{
		case '\r': break;
		case '\n':
			line += ' ';
			done = true;
			break;
		case ' ':
		case '\t':
			lineBreakLen = line.size();
			lineBreakRx = rx;
			line += ' ';
			if (tabWidth > 0 && ch == '\t')
			{
				for (; (line.size() % tabWidth) != 0; line += ' ') {}
			}
			break;
		case '<':
			// ToDo: Read in tag, ignore for line break
		default: line += char(ch); break;
		}
		if (done)
		{
			break;
		}
		if (line.size() > maxLineWidth)
		{
			if (lineBreakRx)
			{
				rx = lineBreakRx;
				line = line.substr(0, lineBreakLen);
			}
			break;
		}
	}
	remainder = rx + 1  < remainder.size() ? remainder.substr(rx + 1) : "";
	return line;
}

/* FUNCTION *******************************************************************/
/**
  Extracts a token at given index. Sets index to character following the token.
@return empty string when no more tokens.
*******************************************************************************/
string getToken(
	const string& source,
	size_t& index,        // Start index (updated)
	const string& delim,  // Delimiter characters, see wantDelim
	const string& ignore, // Characters to ignore
	bool wantDelim)       // Delimiters also returned
{
	string token;
	for (; index < source.size(); index++)
	{
		const int ch{source[index]};
		const bool isIgnore{0 != memchr(ignore.c_str(), ch, ignore.size())};
		const bool isDelim{0 != memchr(delim.c_str(), ch, delim.size())};
		if (isDelim)
		{	// Delimiter found (namely: source[index])
			if (!token.empty())
			{	// Don't consume delimiter yet
				break;
			}
			if (wantDelim && !isIgnore)
			{	// Consume and return delimiter
				token += source[index++];
				break;
			}
		}
		else if (!isIgnore)
		{
			token += source[index];
		}
	}
	return token;
}

/* FUNCTION *******************************************************************/
/**
@param extension: string like "cpp", "txt". "*" is joker
@return true when file has given extension (case insensitive)
*******************************************************************************/
bool hasExtension(const string& filename, const string& extension)
{
	const string::size_type delim{filename.find_last_of("./\\")};
	if (delim == string::npos || filename[delim] != '.')
	{
		return extension.empty();
	}
	return extension == "*" || toLower(filename.substr(delim + 1)) == toLower(extension);
}

/* FUNCTION *******************************************************************/
/**
@return quoted string
*******************************************************************************/
string quote(const string& str)
{
	return "\"" + str + "\"";
}

/* FUNCTION *******************************************************************/
/**
  Replaces 'old' with 'repText' in 'str'
@return text with replacements
*******************************************************************************/
string replace(const string& str, const string& old,
	const string& repText, int maxNum, bool token)
{
	string ret;
	string::size_type pos0{};
	for (; maxNum > 0; maxNum--)
	{
		const string::size_type pos{str.find(old, pos0)};
		if (pos == string::npos)
		{
			break;
		}
		// Found
		if (token)
		{
			string::size_type pos1{pos + old.size()};
			if ((pos > 0 && (isAlNum(str[pos - 1]) || str[pos - 1] == '_'))
				|| (pos1 < str.size() && (isAlNum(str[pos1]) || str[pos1] == '_')))
			{
				ret += str.substr(pos0, pos - pos0 + 1);
				pos0 = pos + 1;
				continue;
			}
		}
		ret += str.substr(pos0, pos - pos0);
		ret += repText;
		pos0 = pos + old.size();
	}
	return ret + str.substr(pos0);
}

/* FUNCTION *******************************************************************/
/**
  TR1
*
@return string with replacement
*******************************************************************************/
string replace_first(const string& str, const string& old, const string& repText)
{
	return replace(str, old, repText);
}

/* FUNCTION *******************************************************************/
/**
@return numCh rightmost characters
*******************************************************************************/
string right(const string& str, size_t numCh)
{
	return str.substr(str.size() >= string::size_type(numCh) ? str.size() - numCh : 0);
}

/* FUNCTION *******************************************************************/
/**
  Sets the filename extension (append or convert).
Removes extension together with the '.' when extension.empty().
*******************************************************************************/
void setFileExtension(
	string& newName,         // Output. May be same string as filename
	const string& filename,  // Input
	const string& extension) // ("cpp", "h", ... or empty)
{
	const string::size_type delim{filename.find_last_of("./\\")};
	if (delim == string::npos || filename[delim] == '/' || filename[delim] == '\\')
	{
		if (extension.empty())
		{
			newName = filename;
		}
		else newName = filename + "." + extension;
	}
	else
	{
		if (extension.empty())
		{
			newName = filename.substr(0, delim);
		}
		else newName = filename.substr(0, delim + 1) + extension;
	}
}

/* FUNCTION *******************************************************************/
/**
  Sets the filename extension (append or convert).
Removes extension together with the '.' when extension.empty().
*******************************************************************************/
string setFileExtension(const string& filename, const string& extension)
{
	string newName;
	setFileExtension(newName, filename, extension);
	return newName;
}

/* FUNCTION *******************************************************************/
/**
  Terminates string with given character
*******************************************************************************/
string setLastChar(const string& str, int ch)
{
	string ret(str);
	const string::size_type len{str.size()};
	if (len == 0 || str[len - 1] != ch)
	{
		ret += char(ch);
	}
	return ret;
}

/* FUNCTION *******************************************************************/
/**
  Decomposes source into tokens.
@param result:
@param    sep:
@param  checkEscape: \<sep> isn't a separator when true.
@return vector of tokens (without separator)
*******************************************************************************/
void split(std::vector<string>& result, const string& source, int sep, bool checkEscape)
{
	result.clear();
	string part;
	for (size_t ix{}; ix < source.size(); ix++)
	{
		const auto ch{source[ix]};
		if (checkEscape && ch == '\\' && ix + 1 < source.size() && source[ix + 1] == sep)
		{
			part += char(sep);
			ix++;
		}
		else if (ch == sep)
		{
			if (!part.empty())
			{
				result.push_back(part);
				part.erase();
			}
		}
		else
		{
			part += ch;
		}
	}
	if (!part.empty())
	{
		result.push_back(part);
	}
}

/* FUNCTION *******************************************************************/
/**
@return true if str starts with start
*******************************************************************************/
bool startsWith(const string& str, const string& start)
{
	return str.substr(0, start.size()) == start;
}

/* FUNCTION *******************************************************************/
/**
@return Unicode converted to multibyte ascii string.
*******************************************************************************/
string toAscii(const std::wstring& wstr)
{
	string ret;
	const size_t numByte{wcstombs(nullptr, wstr.c_str(), 0) + 1};
	if (numByte > 0)
	{
		char* buf{new char[numByte]};
		wcstombs(buf, wstr.c_str(), numByte); // "Not thread safe"?
		ret = buf;
		delete [] buf;
	}
	return ret;
}

/* FUNCTION *******************************************************************/
/**
@return Lowercase string
*******************************************************************************/
string toLower(const string& str)
{
	string ret;
	for (auto ch : str)
	{
		if ('A' <= ch && ch <= 'Z')
		{
			ch += 'a'-'A';
		}
		ret += ch;
	}
	return ret;
}

/* FUNCTION *******************************************************************/
/**
  Converts number to string
*******************************************************************************/
string toString(int val, const char* format)
{
	char buf[128];
	sPrintF(buf, format, val);
	return string(buf);
}
string toString(unsigned val, const char* format)
{
	char buf[128];
	sPrintF(buf, format, val);
	return string(buf);
}

#ifndef __GNUC__
string toString(_int64 val, const char* format)
{
	char buf[128];
	sprintf_s(buf, format, val);
	return string(buf);
}
#endif

string toString(double val, const char* format)
{
	char buf[128];
	sPrintF(buf, format, val);
	return string(buf);
}

/* FUNCTION *******************************************************************/
/**
  Conversion to string a la sprintf.
*******************************************************************************/
string toString(const char* format, ...)
{
	char buf[2048];
	va_list marker;
	va_start(marker, format);
#ifdef __GNUC__
	vsnprintf(buf, sizeof(buf), format, marker);
#else
	vsprintf_s(buf, format, marker);
#endif
	va_end(marker);
	return string(buf);
}

/* FUNCTION *******************************************************************/
/**
@return str converted to upper case
*******************************************************************************/
string toUpper(const string& str)
{
	string ret;
	for (auto ch : str)
	{
		if ('a' <= ch && ch <= 'z')
		{
			ch += 'A'-'a';
		}
		ret += ch;
	}
	return ret;
}

/* FUNCTION *******************************************************************/
/**
@return String without leading and trailing white space
*******************************************************************************/
string trimWhite(const string& str)
{
	if (!str.empty())
	{
		unsigned start{};
		for (; memchr(" \t\n\r", str[start], 4); start++) {}
		string::size_type end{str.size() - 1};
		for (; end > 0 && memchr(" \t\n\r", str[end], 4); end--) {}
		return end >= start ? str.substr(start, 1 + end - start) : "";
	}
	return str;
}

/* FUNCTION *******************************************************************/
/**
  Removes quotes around string.
@param   str:
@param chQuote: Default is '"'
@return String without quotes
*******************************************************************************/
std::string unQuote(const string& str, int chQuote)
{
	string ret(str);
	if (ret.size() >= 2 && ret[0] == chQuote && ret[ret.size() - 1] == chQuote)
	{
		ret = ret.substr(1, ret.size() - 2);
	}
	return ret;
}
