/******************************************************************************/
// PO translation to engine DAT files converter for KeeperFX
/******************************************************************************/
/** @file catalog.cpp
 *     PO translation catalog parser.
 * @par Purpose:
 *     Contains code to read UTF .PO translation files.
 * @par Comment:
 *     Based on the implementation from POEdit (http://www.poedit.net).
 * @author   Tomasz Lis <listom@gmail.com>
 * @author   Vaclav Slavik <vslavik@fastmail.fm>
 * @date     5 Aug 2012 - 22 Sep 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <stdio.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <set>
#include <algorithm>
#include "utf8_unchecked.h"

#include "catalog.hpp"

#define poLogError(format,args...) printf(format "\n", ## args)
#define poLogTrace(format,args...) printf(format "\n", ## args)

template<class S>
bool isEmpty(S& a)
{
    return(S::traits_type::eq_int_type( a.rdbuf()->sgetc(), S::traits_type::eof() ));
}

static void StripTextTrail(std::wstring &s)
{
    size_t pos;
    pos = s.find_last_not_of(L" \n\r\t");
    if (pos == std::wstring::npos) { s = L""; return; }
    s.erase(pos+1);
}

static void StripTextStart(std::wstring &s)
{
    size_t pos;
    pos = s.find_first_not_of(L" \n\r\t");
    if (pos == std::wstring::npos) { s = L""; return; }
    s.erase(0,pos);
}

std::vector<std::wstring> SplitTextC(const std::wstring &ss, const std::wstring c = L" ")
{
    std::vector<std::wstring> result;
    const wchar_t *str = ss.c_str();

    while(1)
    {
        const wchar_t *begin = str;

        while((*str) && (c.find(*str) == std::wstring::npos))
                str++;

        result.push_back(std::wstring(begin, str));

        if(0 == *str++)
                break;
    }

    return result;
}

void TextReplaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
        size_t end_pos = start_pos + from.length();
        str.replace(start_pos, end_pos, to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

size_t CurrentTextLine = 0;

// Read one line from file, remove all \r and \n characters, ignore empty lines
static std::wstring ReadTextLine(std::istream* f)
{
    std::wstring s;

    while (s.empty())
    {
        if (f->eof()) return L"";

        // read next line and strip insignificant whitespace from it:
        std::string line;
        std::getline(*f, line);

        std::string::iterator end_it;
        // Convert line to utf-16
        //std::vector<unsigned short> utf16line;
        utf8::utf8to16(line.begin(), line.end(), back_inserter(s));

        StripTextStart(s);
        StripTextTrail(s);
        CurrentTextLine++;
    }

    return s;
}


// If input begins with pattern, fill output with end of input (without
// pattern; strips trailing spaces) and return true.  Return false otherwise
// and don't touch output. Is permissive about whitespace in the input:
// a space (' ') in pattern will match any number of any whitespace characters
// on that position in input.
static bool ReadParam(const std::wstring& input, const std::wstring& pattern, std::wstring& output)
{
    if (input.size() < pattern.size())
        return false;

    unsigned in_pos = 0;
    unsigned pat_pos = 0;
    while (pat_pos < pattern.size() && in_pos < input.size())
    {
        const wchar_t pat = pattern[pat_pos++];

        if (pat == ' ')
        {
            if (!isspace(input[in_pos++]))
                return false;

            while (isspace(input[in_pos]))
            {
                in_pos++;
                if (in_pos == input.size())
                    return false;
            }
        }
        else
        {
            if (input[in_pos++] != pat)
                return false;
        }

    }

    if (pat_pos < pattern.size()) // pattern not fully matched
        return false;

    output = input.substr(in_pos);
    StripTextTrail(output);
    return true;
}


// converts \n into newline character and \\ into \:
static std::wstring UnescapeCEscapes(const std::wstring& str)
{
    std::wstringstream out;
    size_t len = str.size();
    size_t i;

    if ( len == 0 )
        return str;

    for (i = 0; i < len-1; i++)
    {
        if (str[i] == L'\\')
        {
            switch ((wchar_t)str[i+1])
            {
                case 'n':
                    out << L'\n';
                    i++;
                    break;
                case '\\':
                    out << L'\\';
                    i++;
                    break;
                case '"':
                    out << L'"';
                    i++;
                    break;
                default:
                    out << L'\\';
            }
        }
        else
        {
            out << str[i];
        }
    }

    // last character:
    if (i < len)
        out << str[i];

    return out.str();
}


// ----------------------------------------------------------------------
// Catalog::HeaderData
// ----------------------------------------------------------------------

void Catalog::HeaderData::FromString(const std::wstring& str)
{
    std::wstring ss = UnescapeCEscapes(str);
    std::vector<std::wstring> tkn = SplitTextC(ss, L"\n");

    std::wstring ln;

    m_entries.clear();

    for (std::vector<std::wstring>::iterator it = tkn.begin(); it!=tkn.end(); ++it)
    {
        ln = *it;
        size_t pos = ln.find(':');
        if (pos == std::wstring::npos)
        {
            if (ln.length() > 0) { // Allow empty lines
                std::string lln(ln.begin(), ln.end());
                poLogError("Malformed header: '%s'", lln.c_str());
            }
        }
        else
        {
            Entry en;
            en.Key = ln.substr(0, pos);
            StripTextStart(en.Key);
            StripTextTrail(en.Key);
            en.Value = ln.substr(pos + 1);
            StripTextStart(en.Value);
            StripTextTrail(en.Value);

            // correct some common errors:
            if ( en.Key == L"Plural-Forms" )
            {
                if ( !en.Value.empty() && (*en.Value.rbegin() != L';') )
                    en.Value += L";";
            }

            m_entries.push_back(en);
            {
                std::string lKey(en.Key.begin(), en.Key.end());
                std::string lValue(en.Value.begin(), en.Value.end());
                //poLogTrace("poedit.header %s='%s'", lKey.c_str(), lValue.c_str());
            }
        }
    }

    ParseDict();
}

std::wstring Catalog::HeaderData::ToString(const std::wstring& line_delim)
{
    //UpdateDict();

    std::wstringstream hdr;
    for (std::vector<Entry>::const_iterator i = m_entries.begin();
         i != m_entries.end(); i++)
    {
        std::wstring v(i->Value);
        TextReplaceAll(v, L"\\", L"\\\\");
        TextReplaceAll(v, L"\"", L"\\\"");
        hdr << i->Key << ": " << v << L"\\n" << line_delim;
    }
    return hdr.str();
}

void Catalog::HeaderData::ParseDict()
{
    std::wstring dummy;

    Project = GetHeader(L"Project-Id-Version");
    CreationDate = GetHeader(L"POT-Creation-Date");
    RevisionDate = GetHeader(L"PO-Revision-Date");

    dummy = GetHeader(L"Last-Translator");
    if (!dummy.empty())
    {
        std::vector<std::wstring> tkn = SplitTextC(dummy, L"<>");
        if (tkn.size() != 2)
        {
            Translator = dummy;
            TranslatorEmail = L"";
        }
        else
        {
            Translator = tkn[0];
            StripTextTrail(Translator);
            TranslatorEmail = tkn[1];
        }
    }

    dummy = GetHeader(L"Language-Team");
    if (!dummy.empty())
    {
        std::vector<std::wstring> tkn = SplitTextC(dummy, L"<>");
        if (tkn.size() != 2)
        {
            Team = dummy;
            TeamEmail = L"";
        }
        else
        {
            Team = tkn[0];
            StripTextTrail(Team);
            TeamEmail = tkn[1];
        }
    }

    std::wstring ctype = GetHeader(L"Content-Type");
    static const std::wstring charset_eq = L"; charset=";
    int charsetPos = ctype.find(charset_eq);
    if (charsetPos != std::wstring::npos)
    {
        Charset = ctype.substr(charsetPos + charset_eq.length());
        StripTextStart(Charset);
        StripTextTrail(Charset);
    }
    else
    {
        Charset = L"utf-8";
    }

    // Parse language information, with backwards compatibility with X-Poedit-*:
    LanguageCode = GetHeader(L"Language");
    if ( LanguageCode.empty() )
    {
        std::wstring X_Language = GetHeader(L"X-Poedit-Language");

        std::wstring X_Country = GetHeader(L"X-Poedit-Country");

        if ( !X_Language.empty() )
        {
            LanguageCode = X_Language;
            if ( !X_Country.empty() )
                LanguageCode += L"_" + X_Country;
        }
    }
    DeleteHeader(L"X-Poedit-Language");
    DeleteHeader(L"X-Poedit-Country");

    // Parse extended information:
    SourceCodeCharset = GetHeader(L"X-Poedit-SourceCharset");
    BasePath = GetHeader(L"X-Poedit-Basepath");

    Keywords.clear();
    std::wstring kw = GetHeader(L"X-Poedit-KeywordsList");
    if (!kw.empty())
    {
        std::vector<std::wstring> tkn = SplitTextC(kw, L";");
        for (std::vector<std::wstring>::iterator it = tkn.begin(); it!=tkn.end(); ++it)
            Keywords.push_back(*it);
    }
    else
    {
        // try backward-compatibility version X-Poedit-Keywords. The difference is the separator used
        std::wstring kw = GetHeader(L"X-Poedit-Keywords");
        if (!kw.empty())
        {
            std::vector<std::wstring> tkn = SplitTextC(kw, L",");
            for (std::vector<std::wstring>::iterator it = tkn.begin(); it!=tkn.end(); ++it)
                Keywords.push_back(*it);

            // and remove it, it's not for newer versions:
            DeleteHeader(L"X-Poedit-Keywords");
        }
    }

    int i;
    for(i = 0; i < BOOKMARK_LAST; i++)
    {
      Bookmarks[i] = NO_BOOKMARK;
    }
    std::wstring bk = GetHeader(L"X-Poedit-Bookmarks");
    if (!bk.empty())
    {
        std::vector<std::wstring> tkn = SplitTextC(bk, L",");
        i=0;
        long int val;
        for (std::vector<std::wstring>::iterator it = tkn.begin(); it!=tkn.end(); ++it)
        {
            if (i>=BOOKMARK_LAST) break;
            val = _wtol(it->c_str());
            Bookmarks[i] = val;
            i++;
        }
    }

    i = 0;
    SearchPaths.clear();
    while (true)
    {
        std::wstringstream path;
        path << L"X-Poedit-SearchPath-" << i;
        if (!HasHeader(path.str()))
            break;
        SearchPaths.push_back(GetHeader(path.str()));
        i++;
    }
}

std::wstring Catalog::HeaderData::GetHeader(const std::wstring& key) const
{
    const Entry *e = Find(key);
    if (e)
        return e->Value;
    else
        return L"";
}

bool Catalog::HeaderData::HasHeader(const std::wstring& key) const
{
    return Find(key) != NULL;
}

void Catalog::HeaderData::SetHeader(const std::wstring& key, const std::wstring& value)
{
    Entry *e = (Entry*) Find(key);
    if (e)
    {
        e->Value = value;
    }
    else
    {
        Entry en;
        en.Key = key;
        en.Value = value;
        m_entries.push_back(en);
    }
}

void Catalog::HeaderData::SetHeaderNotEmpty(const std::wstring& key,
                                            const std::wstring& value)
{
    if (value.empty())
        DeleteHeader(key);
    else
        SetHeader(key, value);
}

void Catalog::HeaderData::DeleteHeader(const std::wstring& key)
{
    if (HasHeader(key))
    {
        Entries enew;

        for (Entries::const_iterator i = m_entries.begin();
                i != m_entries.end(); i++)
        {
            if (i->Key != key)
                enew.push_back(*i);
        }

        m_entries = enew;
    }
}

const Catalog::HeaderData::Entry *
Catalog::HeaderData::Find(const std::wstring& key) const
{
    size_t size = m_entries.size();
    for (size_t i = 0; i < size; i++)
    {
        if (m_entries[i].Key == key)
            return &(m_entries[i]);
    }
    return NULL;
}


// ----------------------------------------------------------------------
// Parsers
// ----------------------------------------------------------------------

bool CatalogParser::Parse()
{
    std::wstring line, dummy;
    std::wstring mflags, mstr, msgid_plural, mcomment;
    std::vector<std::wstring> mrefs, mautocomments, mtranslations;
    std::vector<std::wstring> msgid_old;
    bool has_plural = false;
    bool has_context = false;
    std::wstring msgctxt;
    unsigned mlinenum = 0;

    CurrentTextLine = 0;
    line = ReadTextLine(m_textFile);

    while (!line.empty())
    {

        // ignore empty special tags (except for automatic comments which we
        // DO want to preserve):
        while (line == L"#," || line == L"#:" || line == L"#|")
            line = ReadTextLine(m_textFile);

        // flags:
        // Can't we have more than one flag, now only the last is kept ...
        if (ReadParam(line, L"#, ", dummy))
        {
            mflags = L"#, " + dummy;
            line = ReadTextLine(m_textFile);
        }

        // auto comments:
        if (ReadParam(line, L"#. ", dummy) || ReadParam(line, L"#.", dummy)) // second one to account for empty auto comments
        {
            mautocomments.push_back(dummy);
            line = ReadTextLine(m_textFile);
        }

        // references:
        else if (ReadParam(line, L"#: ", dummy))
        {
            // Just store the references unmodified, we don't modify this
            // data anywhere.
            mrefs.push_back(dummy);
            line = ReadTextLine(m_textFile);
        }

        // previous msgid value:
        else if (ReadParam(line, L"#| ", dummy))
        {
            msgid_old.push_back(dummy);
            line = ReadTextLine(m_textFile);
        }

        // msgctxt:
        else if (ReadParam(line, L"msgctxt \"", dummy))
        {
            has_context = true;
            msgctxt = dummy.substr(0, dummy.size()-1);
            while (!(line = ReadTextLine(m_textFile)).empty())
            {
                if (line[0u] == L'\t')
                    line.erase(0, 1);
                if (line[0u] == '"' && *line.rbegin() == '"')
                    msgctxt += line.substr(1, line.length() - 2);
                else
                    break;
            }
        }

        // msgid:
        else if (ReadParam(line, L"msgid \"", dummy))
        {
            mstr = dummy.substr(0, dummy.size()-1);
            mlinenum = CurrentTextLine + 1;
            while (!(line = ReadTextLine(m_textFile)).empty())
            {
                if (line[0u] == '\t')
                    line.erase(0, 1);
                if (line[0u] == '"' && *line.rbegin() == '"')
                    mstr += line.substr(1, line.length() - 2);
                else
                    break;
            }
        }

        // msgid_plural:
        else if (ReadParam(line, L"msgid_plural \"", dummy))
        {
            msgid_plural = dummy.substr(0, dummy.size()-1);
            has_plural = true;
            mlinenum = CurrentTextLine + 1;
            while (!(line = ReadTextLine(m_textFile)).empty())
            {
                if (line[0u] == '\t')
                    line.erase(0, 1);
                if (line[0u] == '"' && *line.rbegin() == '"')
                    msgid_plural += line.substr(1, line.length() - 2);
                else
                    break;
            }
        }

        // msgstr:
        else if (ReadParam(line, L"msgstr \"", dummy))
        {
            if (has_plural)
            {
                poLogError("Broken catalog file: singular form msgstr used together with msgid_plural");
                return false;
            }

            std::wstring str = dummy.substr(0, dummy.size()-1);
            while (!(line = ReadTextLine(m_textFile)).empty())
            {
                if (line[0u] == '\t')
                    line.erase(0, 1);
                if (line[0u] == '"' && *line.rbegin() == '"')
                    str += line.substr(1, line.length() - 2);
                else
                    break;
            }
            mtranslations.push_back(str);

            bool shouldIgnore = m_ignoreHeader && mstr.empty();
            if ( !shouldIgnore )
            {
                if (!OnEntry(mstr, L"", false,
                             has_context, msgctxt,
                             mtranslations,
                             mflags, mrefs, mcomment, mautocomments, msgid_old,
                             mlinenum))
                {
                    return false;
                }
            }

            mcomment = mstr = msgid_plural = msgctxt = mflags = L"";
            has_plural = has_context = false;
            mrefs.clear();
            mautocomments.clear();
            mtranslations.clear();
            msgid_old.clear();
        }

        // msgstr[i]:
        else if (ReadParam(line, L"msgstr[", dummy))
        {
            if (!has_plural)
            {
                poLogError("Broken catalog file: plural form msgstr used without msgid_plural");
                return false;
            }

            size_t brackpos = dummy.find(L']');
            std::wstring idx;
            if (brackpos != std::wstring::npos)
                idx = dummy.substr(0,brackpos);
            std::wstring label = L"msgstr[" + idx + L"]";

            while (ReadParam(line, label + L" \"", dummy))
            {
                std::wstring str = dummy.substr(0, dummy.size()-1);

                while (!(line=ReadTextLine(m_textFile)).empty())
                {
                    if (line[0u] == '"' && *line.rbegin() == '"')
                        str += line.substr(1, line.length() - 2);
                    else
                    {
                        if (ReadParam(line, L"msgstr[", dummy))
                        {
                            idx = dummy.substr(brackpos+1);
                            label = L"msgstr[" + idx + L"]";
                        }
                        break;
                    }
                }
                mtranslations.push_back(str);
            }

            if (!OnEntry(mstr, msgid_plural, true,
                         has_context, msgctxt,
                         mtranslations,
                         mflags, mrefs, mcomment, mautocomments, msgid_old,
                         mlinenum))
            {
                return false;
            }

            mcomment = mstr = msgid_plural = msgctxt = mflags = L"";
            has_plural = has_context = false;
            mrefs.clear();
            mautocomments.clear();
            mtranslations.clear();
            msgid_old.clear();
        }

        // deleted lines:
        else if (ReadParam(line, L"#~ ", dummy))
        {
            std::vector<std::wstring> deletedLines;
            deletedLines.push_back(line);
            mlinenum = CurrentTextLine + 1;
            while (!(line = ReadTextLine(m_textFile)).empty())
            {
                // if line does not start with "#~ " anymore, stop reading
                if (!ReadParam(line, L"#~ ", dummy))
                    break;
                // if the line starts with "#~ msgid", we skipped an empty line
                // and it's a new entry, so stop reading too (see bug #329)
                if (ReadParam(line, L"#~ msgid", dummy))
                    break;

                deletedLines.push_back(line);
            }
            if (!OnDeletedEntry(deletedLines,
                                mflags, mrefs, mcomment, mautocomments, mlinenum))
            {
                return false;
            }

            mcomment = mstr = msgid_plural = mflags = L"";
            has_plural = false;
            mrefs.clear();
            mautocomments.clear();
            mtranslations.clear();
            msgid_old.clear();
        }

        // comment:
        else if (line[0u] == '#')
        {
            bool readNewLine = false;

            while (!line.empty() &&
                    line[0u] == '#' &&
                   (line.length() < 2 || (line[1u] != ',' && line[1u] != ':' && line[1u] != '.' )))
            {
                mcomment += line + L"\n";
                readNewLine = true;
                line = ReadTextLine(m_textFile);
            }

            if (!readNewLine)
                line = ReadTextLine(m_textFile);
        }

        else
        {
            line = ReadTextLine(m_textFile);
        }
    }
    poLogTrace("Parsed %ld lines from input file", (long)CurrentTextLine);

    return true;
}



class CharsetInfoFinder : public CatalogParser
{
    public:
        CharsetInfoFinder(std::istream *f)
                : CatalogParser(f), m_charset(L"utf-8") {}
        std::wstring GetCharset() const { return m_charset; }

    protected:
        std::wstring m_charset;

        virtual bool OnEntry(const std::wstring& msgid,
                             const std::wstring& /*msgid_plural*/,
                             bool /*has_plural*/,
                             bool /*has_context*/,
                             const std::wstring& /*context*/,
                             const std::vector<std::wstring>& mtranslations,
                             const std::wstring& /*flags*/,
                             const std::vector<std::wstring>& /*references*/,
                             const std::wstring& /*comment*/,
                             const std::vector<std::wstring>& /*autocomments*/,
                             const std::vector<std::wstring>& /*msgid_old*/,
                             unsigned /*lineNumber*/)
        {
            if (msgid.empty())
            {
                // gettext header:
                Catalog::HeaderData hdr;
                hdr.FromString(mtranslations[0]);
                m_charset = hdr.Charset;
                if (m_charset == L"CHARSET")
                    m_charset = L"utf-8";
                return false; // stop parsing
            }
            return true;
        }
};



class LoadParser : public CatalogParser
{
    public:
        LoadParser(Catalog *c, std::istream *f)
              : CatalogParser(f), m_catalog(c) {}

    protected:
        Catalog *m_catalog;

        virtual bool OnEntry(const std::wstring& msgid,
                             const std::wstring& msgid_plural,
                             bool has_plural,
                             bool has_context,
                             const std::wstring& context,
                             const std::vector<std::wstring>& mtranslations,
                             const std::wstring& flags,
                             const std::vector<std::wstring>& references,
                             const std::wstring& comment,
                             const std::vector<std::wstring>& autocomments,
                             const std::vector<std::wstring>& msgid_old,
                             unsigned lineNumber);

        virtual bool OnDeletedEntry(const std::vector<std::wstring>& deletedLines,
                                    const std::wstring& flags,
                                    const std::vector<std::wstring>& references,
                                    const std::wstring& comment,
                                    const std::vector<std::wstring>& autocomments,
                                    unsigned lineNumber);
};


bool LoadParser::OnEntry(const std::wstring& msgid,
                         const std::wstring& msgid_plural,
                         bool has_plural,
                         bool has_context,
                         const std::wstring& context,
                         const std::vector<std::wstring>& mtranslations,
                         const std::wstring& flags,
                         const std::vector<std::wstring>& references,
                         const std::wstring& comment,
                         const std::vector<std::wstring>& autocomments,
                         const std::vector<std::wstring>& msgid_old,
                         unsigned lineNumber)
{
    if (msgid.empty())
    {
        // gettext header:
        m_catalog->m_header.FromString(mtranslations[0]);
        m_catalog->m_header.Comment = comment;
        //poLogTrace("Entry - header");
    }
    else
    {
        CatalogItem d;
        if (!flags.empty()) d.SetFlags(flags);
        d.SetString(msgid);
        if (has_plural)
            d.SetPluralString(msgid_plural);
        if (has_context)
            d.SetContext(context);
        d.SetTranslations(mtranslations);
        d.SetComment(comment);
        d.SetLineNumber(lineNumber);
        for (size_t i = 0; i < references.size(); i++)
            d.AddReference(references[i]);
        for (size_t i = 0; i < autocomments.size(); i++)
            d.AddAutoComments(autocomments[i]);
        d.SetOldMsgid(msgid_old);
        m_catalog->AddItem(d);
        /*{
            std::string lmsgid(msgid.begin(), msgid.end());
            poLogTrace("Entry - msgid \"%s\"",lmsgid.c_str());
        }*/
    }
    return true;
}

bool LoadParser::OnDeletedEntry(const std::vector<std::wstring>& deletedLines,
                                const std::wstring& flags,
                                const std::vector<std::wstring>& /*references*/,
                                const std::wstring& comment,
                                const std::vector<std::wstring>& autocomments,
                                unsigned lineNumber)
{
    CatalogDeletedData d;
    if (!flags.empty()) d.SetFlags(flags);
    d.SetDeletedLines(deletedLines);
    d.SetComment(comment);
    d.SetLineNumber(lineNumber);
    for (size_t i = 0; i < autocomments.size(); i++)
      d.AddAutoComments(autocomments[i]);
    m_catalog->AddDeletedItem(d);

    return true;
}



// ----------------------------------------------------------------------
// Catalog class
// ----------------------------------------------------------------------

Catalog::Catalog()
{
    m_isOk = true;
    m_header.BasePath = L"";
    for(int i = BOOKMARK_0; i < BOOKMARK_LAST; i++)
    {
        m_header.Bookmarks[i] = -1;
    }
}


Catalog::~Catalog()
{
    Clear();
}


Catalog::Catalog(const std::string& po_file, int flags)
{
    m_isOk = Load(po_file, flags);
}


bool Catalog::Load(const std::string& po_file, int flags)
{
    std::fstream f;

    Clear();
    m_isOk = false;
    m_fileName = po_file;
    m_header.BasePath = L"";

    /* Load the .po file: */
    f.open(po_file.c_str(), std::fstream::in);

    LoadParser parser(this, &f);
    parser.IgnoreHeader(flags & CreationFlag_IgnoreHeader);
    if (!parser.Parse())
    {
        poLogError("Couldn't load file %s, it is probably corrupted.", po_file.c_str());
        return false;
    }

    // now that the catalog is loaded, update its items with the bookmarks
    for (unsigned i = BOOKMARK_0; i < BOOKMARK_LAST; i++)
    {
        if (m_header.Bookmarks[i] != -1 &&
            m_header.Bookmarks[i] < (int)m_items.size())
        {
            m_items[m_header.Bookmarks[i]].SetBookmark(
                    static_cast<Bookmark>(i));
        }
    }


    m_isOk = true;

    f.close();

    return true;
}

void Catalog::AddItem(const CatalogItem& data)
{
    m_items.push_back(data);
}

void Catalog::AddDeletedItem(const CatalogDeletedData& data)
{
    m_deletedItems.push_back(data);
}

bool Catalog::HasDeletedItems()
{
    return !m_deletedItems.empty();
}

void Catalog::RemoveDeletedItems()
{
    m_deletedItems.clear();
}

CatalogItem *Catalog::FindItemByLine(int lineno)
{
    CatalogItem *last = NULL;

    for ( CatalogItemArray::iterator i = m_items.begin();
          i != m_items.end(); ++i )
    {
        if ( i->GetLineNumber() > lineno )
            return last;
        last = &(*i);
    }

    return last;
}

CatalogItem *Catalog::FindItemByReference(const std::wstring& ref)
{
    for ( CatalogItemArray::iterator i = m_items.begin();
          i != m_items.end(); ++i )
    {
        if ( i->HasReference(ref) )
            return &(*i);
    }
    return NULL;
}

void Catalog::Clear()
{
    m_items.clear();
    m_deletedItems.clear();
    m_isOk = true;
    for(int i = BOOKMARK_0; i < BOOKMARK_LAST; i++)
    {
        m_header.Bookmarks[i] = -1;
    }
}

int Catalog::SetBookmark(int id, Bookmark bookmark)
{
    int result = (bookmark==NO_BOOKMARK)?-1:m_header.Bookmarks[bookmark];

    // unset previous bookmarks, if any
    Bookmark bk = m_items[id].GetBookmark();
    if (bk != NO_BOOKMARK)
        m_header.Bookmarks[bk] = -1;
    if (result > -1)
        m_items[result].SetBookmark(NO_BOOKMARK);

    // set new bookmark
    m_items[id].SetBookmark(bookmark);
    if (bookmark != NO_BOOKMARK)
        m_header.Bookmarks[bookmark] = id;

    // return id of previous item for that bookmark
    return result;
}

std::vector<std::wstring> * Catalog::ToTranslationsVectorByRef(const std::wstring& refname, size_t max_size)
{
    std::vector<std::wstring> * translations = new std::vector<std::wstring>();
    size_t i;
    for (i=0; i < max_size; i++)
    {
        std::wstringstream ref;
        ref << refname << ":" << i;
        CatalogItem * ci = FindItemByReference(ref.str());
        if (ci == NULL) {
            translations->push_back(L"");
            continue;
        }
        std::wstring trans = ci->GetTranslation();
        if (trans.length() < 1) {
            trans = ci->GetString();
        }
        translations->push_back(trans);
    }
    // Remove empty strings at end
    for (std::vector<std::wstring>::reverse_iterator it = translations->rbegin(); it != translations->rend(); ++it)
    {
        if ((*it).length() > 0)
            break;
        i--;
    }
    translations->erase(translations->begin()+i,translations->end());
    return translations;
}

// misc file-saving helpers
namespace
{

/*bool CanEncodeStringToCharset(const std::wstring& s, wxMBConv& conv)
{
    if (s.empty())
        return true;
    if (!s.mb_str(conv))
        return false;
    return true;
}

bool CanEncodeToCharset(Catalog& catalog, const std::wstring& charset)
{
    if (charset.Lower() == L"utf-8" || charset.Lower() == L"utf8")
        return true;

    wxCSConv conv(charset);

    catalog.Header().UpdateDict();
    const Catalog::HeaderData::Entries& hdr(catalog.Header().GetAllHeaders());

    for (Catalog::HeaderData::Entries::const_iterator i = hdr.begin();
            i != hdr.end(); i++)
    {
        if (!CanEncodeStringToCharset(i->Value, conv))
            return false;
    }

    size_t cnt = catalog.size();
    for (size_t i = 0; i < cnt; i++)
    {
        if (!CanEncodeStringToCharset(catalog[i].GetTranslation(), conv) ||
            !CanEncodeStringToCharset(catalog[i].GetString(), conv))
        {
            return false;
        }
    }
    return true;
}


void GetCRLFBehaviour(std::istreamType& type, bool& preserve)
{
    std::wstring format = wxConfigBase::Get()->Read("crlf_format", "unix");

    if (format == "win") type = std::istreamType_Dos;
    else  type = std::istreamType_Unix;

    preserve = (bool)(wxConfigBase::Get()->Read("keep_crlf", true));
}


std::istreamType GetDesiredCRLFFormat(const std::wstring& po_file)
{
    std::istreamType crlfDefault, crlf;
    bool crlfPreserve;
    GetCRLFBehaviour(crlfDefault, crlfPreserve);

    std::istream f;
    if ( crlfPreserve && wxFileExists(po_file) &&
         f.Open(po_file, wxConvISO8859_1) )
    {
        poLogNull null;
        crlf = f.GuessType();

        // Discard any unsupported setting. In particular, we ignore "Mac"
        // line endings, because the ancient OS 9 systems aren't used anymore,
        // OSX uses Unix ending *and* "Mac" endings break gettext tools. So if
        // we encounter a catalog with "Mac" line endings, we silently convert
        // it into Unix endings (i.e. the modern Mac).
        if (crlf == std::istreamType_Mac)
            crlf = std::istreamType_Unix;
        if (crlf != std::istreamType_Dos && crlf != std::istreamType_Unix)
            crlf = crlfDefault;

        f.Close();
    }
    else
    {
        crlf = crlfDefault;
    }

    return crlf;
}*/


/** Adds \n characters as necessary for good-looking output
 */
std::wstring FormatStringForFile(const std::wstring& text)
{
    std::wstringstream s;
    unsigned n_cnt = 0;
    int len = text.length();

    // Scan the string up to len-2 because we don't want to account for the
    // very last \n on the line:
    //       "some\n string \n"
    //                      ^
    //                      |
    //                      \--- = len-2
    int i;
    for (i = 0; i < len-2; i++)
    {
        if (text[i] == '\\' && text[i+1] == 'n')
        {
            n_cnt++;
            s << L"\\n\"\n\"";
            i++;
        }
        else
            s << text[i];
    }
    // ...and add not yet processed characters to the string...
    for (; i < len; i++)
        s << text[i];

    if (n_cnt >= 1)
        return L"\"\n\"" + s.str();
    else
        return s.str();
}

} // anonymous namespace


static unsigned GetCountFromPluralFormsHeader(const Catalog::HeaderData& header)
{
    if ( header.HasHeader(L"Plural-Forms") )
    {
        // e.g. "Plural-Forms: nplurals=3; plural=(n%10==1 && n%100!=11 ?
        //       0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);\n"

        std::wstring form = header.GetHeader(L"Plural-Forms");
        size_t pos = form.find(L';');
        if (pos != std::wstring::npos)
            form = form.erase(pos);
        pos = form.find(L'=');
        if (pos != std::wstring::npos)
        {
            if (form.substr(0,pos).compare(L"nplurals") == 0)
            {
                std::wstring sval = form.substr(pos+1);
                return _wtol(sval.c_str());
            }
        }
    }

    return 0;
}

unsigned Catalog::GetPluralFormsCount() const
{
    unsigned count = GetCountFromPluralFormsHeader(m_header);

    for ( CatalogItemArray::const_iterator i = m_items.begin();
          i != m_items.end(); ++i )
    {
        count = std::max(count, i->GetPluralFormsCount());
    }

    return count;
}

bool Catalog::HasWrongPluralFormsCount() const
{
    unsigned count = 0;

    for ( CatalogItemArray::const_iterator i = m_items.begin();
          i != m_items.end(); ++i )
    {
        count = std::max(count, i->GetPluralFormsCount());
    }

    if ( count == 0 )
        return false; // nothing translated, so we can't tell

    // if 'count' is less than the count from header, it may simply mean there
    // are untranslated strings
    if ( count > GetCountFromPluralFormsHeader(m_header) )
        return true;

    return false;
}

bool Catalog::HasPluralItems() const
{
    for ( CatalogItemArray::const_iterator i = m_items.begin();
          i != m_items.end(); ++i )
    {
        if ( i->HasPlural() )
            return true;
    }

    return false;
}


void Catalog::GetStatistics(int *all, int *fuzzy, int *badtokens,
                            int *untranslated, int *unfinished)
{
    if (all) *all = 0;
    if (fuzzy) *fuzzy = 0;
    if (badtokens) *badtokens = 0;
    if (untranslated) *untranslated = 0;
    if (unfinished) *unfinished = 0;

    for (size_t i = 0; i < GetCount(); i++)
    {
        bool ok = true;

        if (all)
            (*all)++;

        if ((*this)[i].IsFuzzy())
        {
            (*fuzzy)++;
            ok = false;
        }
        if ((*this)[i].GetValidity() == CatalogItem::Val_Invalid)
        {
            (*badtokens)++;
            ok = false;
        }
        if (!(*this)[i].IsTranslated())
        {
            (*untranslated)++;
            ok = false;
        }

        if ( !ok && unfinished )
            (*unfinished)++;
    }
}


void CatalogItem::SetFlags(const std::wstring& flags)
{
    m_isFuzzy = false;
    m_moreFlags = L"";

    if (flags.empty()) return;
    std::vector<std::wstring> tkn = SplitTextC(flags.substr(1), L" ,");
    for (std::vector<std::wstring>::iterator it = tkn.begin(); it!=tkn.end(); ++it)
    {
        std::wstring s;
        s = *it;
        if (s.compare(L"fuzzy") == 0) m_isFuzzy = true;
        else m_moreFlags = L", " + s;
    }
}


std::wstring CatalogItem::GetFlags() const
{
    std::wstring f;
    if (m_isFuzzy) f = L", fuzzy";
    f += m_moreFlags;
    if (!f.empty())
        return L"#" + f;
    else
        return L"";
}

bool CatalogItem::IsInFormat(const std::wstring& format)
{
    std::wstringstream lookingFor;
    lookingFor << format << L"-format";
    std::vector<std::wstring> tkn = SplitTextC(m_moreFlags, L" ,");
    for (std::vector<std::wstring>::iterator it = tkn.begin(); it!=tkn.end(); ++it)
    {
        if (*it == lookingFor.str())
            return true;
    }
    return false;
}

std::wstring CatalogItem::GetTranslation(unsigned idx) const
{
    if (idx >= GetNumberOfTranslations())
        return L"";
    else
        return m_translations[idx];
}

void CatalogItem::SetTranslation(const std::wstring &t, unsigned idx)
{
    while (idx >= m_translations.size())
        m_translations.push_back(L"");
    m_translations[idx] = t;

    m_validity = Val_Unknown;

    m_isTranslated = true;
    for (size_t i = 0; i < m_translations.size(); i++)
    {
        if (m_translations[i].empty())
        {
            m_isTranslated = false;
            break;
        }
    }
}

void CatalogItem::SetTranslations(const std::vector<std::wstring> &t)
{
    m_translations = t;

    m_validity = Val_Unknown;

    m_isTranslated = true;
    for (size_t i = 0; i < m_translations.size(); i++)
    {
        if (m_translations[i].empty())
        {
            m_isTranslated = false;
            break;
        }
    }
}

unsigned CatalogItem::GetPluralFormsCount() const
{
    unsigned trans = GetNumberOfTranslations();
    if ( !HasPlural() || !trans )
        return 0;

    return trans - 1;
}

std::vector<std::wstring> CatalogItem::GetReferences() const
{
    // A line may contain several references, separated by white-space.
    // Each reference is in the form "path_name:line_number"
    // (path_name may contain spaces)

    std::vector<std::wstring> refs;

    for ( std::vector<std::wstring>::const_iterator i = m_references.begin(); i != m_references.end(); ++i )
    {
        std::wstring line = *i;

        StripTextStart(line);
        StripTextTrail(line);
        while (!line.empty())
        {
            size_t i = 0;
            while (i < line.length() && line[i] != ':') { i++; }
            while (i < line.length() && !isspace(line[i])) { i++; }

            refs.push_back(line.substr(0,i));
            line = line.substr(i);
            StripTextStart(line);
            StripTextTrail(line);
        }
    }

    return refs;
}

bool CatalogItem::HasReference(const std::wstring &ref) const
{
    for ( std::vector<std::wstring>::const_iterator i = m_references.begin(); i != m_references.end(); ++i )
    {
        std::wstring line = *i;
        StripTextStart(line);
        StripTextTrail(line);
        while (!line.empty())
        {
            size_t i = 0;
            while (i < line.length() && line[i] != ':') { i++; }
            while (i < line.length() && !isspace(line[i])) { i++; }

            if (ref.compare(line.substr(0,i)) == 0)
                return true;
            line = line.substr(i);
            StripTextStart(line);
            StripTextTrail(line);
        }
    }
    return false;
}
