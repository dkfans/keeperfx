/******************************************************************************/
// PO translation to engine DAT files converter for KeeperFX
/******************************************************************************/
/** @file catalog.hpp
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
#ifndef _CATALOG_HPP_
#define _CATALOG_HPP_

#include <string>
#include <iostream>

#include <vector>
#include <map>

class ProgressInfo;

/// The possible bookmarks for a given item
typedef enum
{
    NO_BOOKMARK = -1,
    BOOKMARK_0,
    BOOKMARK_1,
    BOOKMARK_2,
    BOOKMARK_3,
    BOOKMARK_4,
    BOOKMARK_5,
    BOOKMARK_6,
    BOOKMARK_7,
    BOOKMARK_8,
    BOOKMARK_9,
    BOOKMARK_LAST
} Bookmark;

/** This class holds information about one particular string.
    This includes source string and its occurrences in source code
    (so-called references), translation and translation's status
    (fuzzy, non translated, translated) and optional comment.

    This class is mostly internal, used by Catalog to store data.
 */
class CatalogItem
{
    public:
        /// Ctor. Initializes the object with source string and translation.
        CatalogItem(const std::wstring& str = L"",
                    const std::wstring& str_plural = L"")
                : m_string(str),
                  m_plural(str_plural),
                  m_hasPlural(false),
                  m_hasContext(false),
                  m_references(),
                  m_isFuzzy(false),
                  m_isTranslated(false),
                  m_isModified(false),
                  m_isAutomatic(false),
                  m_validity(Val_Unknown),
                  m_lineNum(0),
                  m_bookmark(NO_BOOKMARK) {}

        CatalogItem(const CatalogItem& dt)
                : m_string(dt.m_string),
                  m_plural(dt.m_plural),
                  m_hasPlural(dt.m_hasPlural),
                  m_hasContext(dt.m_hasContext),
                  m_context(dt.m_context),
                  m_translations(dt.m_translations),
                  m_references(dt.m_references),
                  m_autocomments(dt.m_autocomments),
                  m_oldMsgid(dt.m_oldMsgid),
                  m_isFuzzy(dt.m_isFuzzy),
                  m_isTranslated(dt.m_isTranslated),
                  m_isModified(dt.m_isModified),
                  m_isAutomatic(dt.m_isAutomatic),
                  m_hasBadTokens(dt.m_hasBadTokens),
                  m_moreFlags(dt.m_moreFlags),
                  m_comment(dt.m_comment),
                  m_validity(dt.m_validity),
                  m_lineNum(dt.m_lineNum),
                  m_errorString(dt.m_errorString),
                  m_bookmark(dt.m_bookmark) {}

        /// Returns the source string.
        const std::wstring& GetString() const { return m_string; }

        /// Does this entry have a msgid_plural?
        bool HasPlural() const { return m_hasPlural; }

        /// Returns the plural string.
        const std::wstring& GetPluralString() const { return m_plural; }

        /// Does this entry have a msgctxt?
        bool HasContext() const { return m_hasContext; }

        /// Returns context string (can only be called if HasContext() returns
        /// true and empty string is accepted value).
        const std::wstring& GetContext() const { return m_context; }

        /// How many translations (plural forms) do we have?
        unsigned GetNumberOfTranslations() const
            { return m_translations.size(); }

        /// Returns number of plural forms in this translation; note that this
        /// may be less than what the header says, because some may be
        /// still untranslated
        unsigned GetPluralFormsCount() const;

        /// Returns the nth-translation.
        std::wstring GetTranslation(unsigned n = 0) const;

        /// Returns all translations.
        const std::vector<std::wstring>& GetTranslations() const
            { return m_translations; }

        /// Returns references (#:) lines for the entry
        const std::vector<std::wstring>& GetRawReferences() const { return m_references; }

        /// Returns array of all occurrences of this string in source code,
        /// parsed into individual references
        std::vector<std::wstring> GetReferences() const;

        /// Returns array of all occurrences of this string in source code,
        /// parsed into individual references
        bool HasReference(const std::wstring &ref) const;

        /// Returns comment added by the translator to this entry
        const std::wstring& GetComment() const { return m_comment; }

        /// Returns array of all auto comments.
        const std::vector<std::wstring>& GetAutoComments() const { return m_autocomments; }

        /// Convenience function: does this entry has a comment?
        bool HasComment() const { return !m_comment.empty(); }

        /// Adds new reference to the entry (used by SourceDigger).
        void AddReference(const std::wstring& ref)
        {
            for (std::vector<std::wstring>::iterator it = m_references.begin(); it < m_references.end(); it++)
            {
                if ((*it).compare(ref) == 0)
                    return;
            }
            m_references.push_back(ref);
        }

        /// Sets the string.
        void SetString(const std::wstring& s)
        {
            m_string = s;
            m_validity = Val_Unknown;
        }

        /// Sets the plural form (if applicable).
        void SetPluralString(const std::wstring& p)
        {
            m_plural = p;
            m_hasPlural = true;
        }

        /// Sets the context (msgctxt0
        void SetContext(const std::wstring& context)
        {
            m_hasContext = true;
            m_context = context;
        }

        /** Sets the translation. Changes "translated" status to true
            if \a t is not empty.
         */
        void SetTranslation(const std::wstring& t, unsigned index = 0);

        /// Sets all translations.
        void SetTranslations(const std::vector<std::wstring>& t);

        /// Sets the comment.
        void SetComment(const std::wstring& c)
        {
            m_comment = c;
        }

        /** Sets gettext flags directly in string format. It may be
            either empty string or "#, fuzzy", "#, c-format",
            "#, fuzzy, c-format" or others (not understood by Poedit).
         */
        void SetFlags(const std::wstring& flags);

        /// Gets gettext flags. \see SetFlags
        std::wstring GetFlags() const;

        /// Sets fuzzy flag.
        void SetFuzzy(bool fuzzy) { m_isFuzzy = fuzzy; }
        /// Gets value of fuzzy flag.
        bool IsFuzzy() const { return m_isFuzzy; }
        /// Sets translated flag.
        void SetTranslated(bool t) { m_isTranslated = t; }
        /// Gets value of translated flag.
        bool IsTranslated() const { return m_isTranslated; }
        /// Sets modified flag.
        void SetModified(bool modified) { m_isModified = modified; }
        /// Gets value of modified flag.
        bool IsModified() const { return m_isModified; }
        /// Sets automatic translation flag.
        void SetAutomatic(bool automatic) { m_isAutomatic = automatic; }
        /// Gets value of automatic translation flag.
        bool IsAutomatic() const { return m_isAutomatic; }
        /// Sets the number of the line this entry occurs on.
        void SetLineNumber(int line) { m_lineNum = line; }
        /// Get line number of this entry.
        int GetLineNumber() const { return m_lineNum; }

        /** Returns true if the gettext flags line contains "foo-format"
            flag when called with "foo" as argument. */
        bool IsInFormat(const std::wstring& format);

        /// Adds new autocomments (#. )
        void AddAutoComments(const std::wstring& com)
        {
            m_autocomments.push_back(com);
        }

        /// Clears autocomments.
        void ClearAutoComments()
        {
            m_autocomments.clear();
        }

        void SetOldMsgid(const std::vector<std::wstring>& data) { m_oldMsgid = data; }
        const std::vector<std::wstring>& GetOldMsgid() const { return m_oldMsgid; }

        // Validity (syntax-checking) status of the entry:
        enum Validity
        {
            Val_Unknown = -1,
            Val_Invalid = 0,
            Val_Valid = 1
        };

        /** Checks if %i etc. are correct in the translation (true if yes).
            Strings that are not c-format are always correct. */
        Validity GetValidity() const { return m_validity; }
        void SetValidity(bool val)
            { m_validity = val ? Val_Valid : Val_Invalid; }

        void SetErrorString(const std::wstring& str) { m_errorString = str; }
        std::wstring GetErrorString() const { return m_errorString; }

        /// Returns the bookmark for the item
        Bookmark GetBookmark() const {return m_bookmark;}
        /// Returns true if the item has a bookmark
        bool HasBookmark() const {return (GetBookmark() != NO_BOOKMARK);}
        /// Sets the bookmark
        void SetBookmark(Bookmark bookmark) {m_bookmark = bookmark;}

    private:
        std::wstring m_string, m_plural;
        bool m_hasPlural;

        bool m_hasContext;
        std::wstring m_context;

        std::vector<std::wstring> m_translations;

        std::vector<std::wstring> m_references, m_autocomments;
        std::vector<std::wstring> m_oldMsgid;
        bool m_isFuzzy, m_isTranslated, m_isModified, m_isAutomatic;
        bool m_hasBadTokens;
        std::wstring m_moreFlags;
        std::wstring m_comment;
        Validity m_validity;
        int m_lineNum;
        std::wstring m_errorString;
        Bookmark m_bookmark;
};

/** This class holds information about one particular deleted item.
    This includes deleted lines, references, translation's status
    (fuzzy, non translated, translated) and optional comment(s).

    This class is mostly internal, used by Catalog to store data.
 */
// FIXME: derive this from CatalogItem (or CatalogItemBase)
class CatalogDeletedData
{
    public:
        /// Ctor.
        CatalogDeletedData()
                : m_lineNum(0) {}
        CatalogDeletedData(const std::vector<std::wstring>& deletedLines)
                : m_deletedLines(deletedLines),
                  m_lineNum(0) {}

        CatalogDeletedData(const CatalogDeletedData& dt)
                : m_deletedLines(dt.m_deletedLines),
                  m_references(dt.m_references),
                  m_autocomments(dt.m_autocomments),
                  m_flags(dt.m_flags),
                  m_comment(dt.m_comment),
                  m_lineNum(dt.m_lineNum) {}

        /// Returns the deleted lines.
        const std::vector<std::wstring>& GetDeletedLines() const { return m_deletedLines; }

        /// Returns references (#:) lines for the entry
        const std::vector<std::wstring>& GetRawReferences() const { return m_references; }

        /// Returns comment added by the translator to this entry
        const std::wstring& GetComment() const { return m_comment; }

        /// Returns array of all auto comments.
        const std::vector<std::wstring>& GetAutoComments() const { return m_autocomments; }

        /// Convenience function: does this entry has a comment?
        bool HasComment() const { return !m_comment.empty(); }

        /// Adds new reference to the entry (used by SourceDigger).
        void AddReference(const std::wstring& ref)
        {
            for (std::vector<std::wstring>::iterator it = m_references.begin(); it < m_references.end(); it++)
            {
                if ((*it).compare(ref) == 0)
                    return;
            }
            m_references.push_back(ref);
        }

        /// Sets the string.
        void SetDeletedLines(const std::vector<std::wstring>& a)
        {
            m_deletedLines = a;
        }

        /// Sets the comment.
        void SetComment(const std::wstring& c)
        {
            m_comment = c;
        }

        /** Sets gettext flags directly in string format. It may be
            either empty string or "#, fuzzy", "#, c-format",
            "#, fuzzy, c-format" or others (not understood by Poedit).
         */
        void SetFlags(const std::wstring& flags) {m_flags = flags;};

        /// Gets gettext flags. \see SetFlags
        std::wstring GetFlags() const {return m_flags;};

        /// Sets the number of the line this entry occurs on.
        void SetLineNumber(int line) { m_lineNum = line; }
        /// Get line number of this entry.
        int GetLineNumber() const { return m_lineNum; }

        /// Adds new autocomments (#. )
        void AddAutoComments(const std::wstring& com)
        {
            m_autocomments.push_back(com);
        }

        /// Clears autocomments.
        void ClearAutoComments()
        {
            m_autocomments.clear();
        }

    private:
        std::vector<std::wstring> m_deletedLines;

        std::vector<std::wstring> m_references, m_autocomments;
        std::wstring m_flags;
        std::wstring m_comment;
        int m_lineNum;
};


typedef std::vector<CatalogItem> CatalogItemArray;
typedef std::vector<CatalogDeletedData> CatalogDeletedDataArray;
typedef std::map<std::wstring, unsigned> CatalogItemIndex;

/** This class stores all translations, together with filelists, references
    and other additional information. It can read .po files and save both
    .mo and .po files. Furthermore, it provides facilities for updating the
    catalog from source files.
 */
class Catalog
{
    public:
        /// PO file header information.
        class HeaderData
        {
        public:
            HeaderData() {}

            /** Initializes the headers from string that is in msgid "" format
                (i.e. list of key:value\n entries). */
            void FromString(const std::wstring& str);

            /** Converts the header into string representation that can be
                directly written to .po file as msgid "". */
            std::wstring ToString(const std::wstring& line_delim = L"");

            /// Updates headers list from parsed values entries below
            void UpdateDict();
            /// Reverse operation to UpdateDict
            void ParseDict();

            /// Returns value of header or empty string if missing.
            std::wstring GetHeader(const std::wstring& key) const;

            /// Returns true if given key is present in the header.
            bool HasHeader(const std::wstring& key) const;

            /** Sets header to given value. Overwrites old value if present,
                appends to the end of header values otherwise. */
            void SetHeader(const std::wstring& key, const std::wstring& value);

            /// Like SetHeader, but deletes the header if value is empty
            void SetHeaderNotEmpty(const std::wstring& key, const std::wstring& value);

            /// Removes given header entry
            void DeleteHeader(const std::wstring& key);

            struct Entry
            {
                std::wstring Key, Value;
            };
            typedef std::vector<Entry> Entries;

            const Entries& GetAllHeaders() const { return m_entries; }

            // Parsed values:

            std::wstring LanguageCode, Project, CreationDate,
                     RevisionDate, Translator, TranslatorEmail,
                     Team, TeamEmail, Charset, SourceCodeCharset;

            std::vector<std::wstring> SearchPaths, Keywords;
            int Bookmarks[BOOKMARK_LAST];
            std::wstring BasePath;

            std::wstring Comment;

        protected:
            Entries m_entries;

            const Entry *Find(const std::wstring& key) const;
        };

        enum CreationFlags
        {
            CreationFlag_IgnoreHeader = 1
        };

        /// Default ctor. Creates empty catalog, you have to call Load.
        Catalog();

        /// Ctor that loads the catalog from \a po_file with Load.
        /// \a flags is CreationFlags combination.
        Catalog(const std::string& po_file, int flags = 0);

        ~Catalog();

        /** Creates new, empty header. Sets Charset to something meaningful
            ("UTF-8", currently).
         */
        void CreateNewHeader();

        /** Creates new header initialized based on a POT file's header.
         */
        void CreateNewHeader(const HeaderData& pot_header);

        /// Clears the catalog, removes all entries from it.
        void Clear();

        /** Loads catalog from .po file.
            If file named po_file ".poedit" (e.g. "cs.po.poedit") exists,
            this function loads additional information from it. .po.poedit
            file contains parts of catalog header data that are not part
            of standard .po format, namely SearchPaths, Keywords, BasePath
            and Language.

            @param flags  CreationFlags combination.
         */
        bool Load(const std::string& po_file, int flags = 0);

        /** Saves catalog to file. Creates both .po (text) and .mo (binary)
            version of the catalog (unless the latter was disabled in
            preferences). Calls external xmsgfmt program to generate the .mo
            file.

            Note that \a po_file refers to .po file, .mo file will have same
            name & location as .po file except for different extension.
         */
        bool Save(const std::string& po_file, bool save_mo, int& validation_errors);

        /// Exports the catalog to HTML format
        bool ExportToHTML(const std::wstring& filename);

        /// Converts the translations to vector
        std::vector<std::wstring> * ToTranslationsVectorByRef(const std::wstring& refname, size_t max_size);

        /** Updates the catalog from sources.
            \see SourceDigger, Parser, UpdateFromPOT.
         */
        bool Update(ProgressInfo *progress, bool summary = true);

        /** Updates the catalog from POT file.
            \see Update
         */
        bool UpdateFromPOT(const std::wstring& pot_file,
                           bool summary = true,
                           bool replace_header = false);

        /// Returns the number of strings/translations in the catalog.
        size_t GetCount() const { return m_items.size(); }

        /** Returns number of all, fuzzy, badtokens and untranslated items.
            Any argument may be NULL if the caller is not interested in
            given statistic value.

            @note "untranslated" are entries without translation; "unfinished"
                  are entries with any problems
         */
        void GetStatistics(int *all, int *fuzzy, int *badtokens,
                           int *untranslated, int *unfinished);

        /// Gets n-th item in the catalog (read-write access).
        CatalogItem& operator[](unsigned n) { return m_items[n]; }

        /// Gets n-th item in the catalog (read-only access).
        const CatalogItem& operator[](unsigned n) const { return m_items[n]; }

        /// Gets catalog header (read-write access).
        HeaderData& Header() { return m_header; }

        /// Returns plural forms count: taken from Plural-Forms header if
        /// present, 0 otherwise (unless there are existing plural forms
        /// translations in the file)
        unsigned GetPluralFormsCount() const;

        /// Returns true if Plural-Forms header doesn't match plural forms
        /// usage in catalog items
        bool HasWrongPluralFormsCount() const;

        /// Does this catalog have any items with plural forms?
        bool HasPluralItems() const;

        /** Returns status of catalog object: true if ok, false if damaged
            (i.e. constructor or Load failed).
         */
        bool IsOk() const { return m_isOk; }

        /** Returns xx_YY ISO code of catalog's language if either the Poedit
            extensions headers are present or if filename is known and is in
            the xx[_YY] form, otherwise returns empty string. */
        std::wstring GetLocaleCode() const;

        /// Adds entry to the catalog (the catalog will take ownership of
        /// the object).
        void AddItem(const CatalogItem& data);

        /// Adds entry to the catalog (the catalog will take ownership of
        /// the object).
        void AddDeletedItem(const CatalogDeletedData& data);

        /// Returns true if the catalog contains obsolete entries (~.*)
        bool HasDeletedItems();

        /// Removes all obsolete translations from the catalog
        void RemoveDeletedItems();

        /// Finds item by line number
        CatalogItem *FindItemByLine(int lineno);

        /// Finds item by line number
        CatalogItem *FindItemByReference(const std::wstring& ref);

        /// Sets the given item to have the given bookmark and returns the index
        /// of the item that previously had this bookmark (or -1)
        int SetBookmark(int id, Bookmark bookmark);

        /// Returns the index of the item that has the given bookmark or -1
        int GetBookmarkIndex(Bookmark bookmark) const
        {
            return m_header.Bookmarks[bookmark];
        }


        /// Validates correctness of the translation by running msgfmt
        /// Returns number of errors (i.e. 0 if no errors).
        int Validate();

    protected:
        int DoValidate(const std::string& po_file);
        bool DoSaveOnly(const std::string& po_file);


    private:
        CatalogItemArray m_items;
        CatalogDeletedDataArray m_deletedItems;

        bool m_isOk;
        std::string m_fileName;
        HeaderData m_header;

        friend class LoadParser;
};


/// Internal class - used for parsing of po files.
class CatalogParser
{
    public:
        CatalogParser(std::istream *f)
            : m_textFile(f),
              m_ignoreHeader(false)
        {}

        virtual ~CatalogParser() {}

        /// Tell the parser to ignore header entries when processing
        void IgnoreHeader(bool ignore) { m_ignoreHeader = ignore; }

        /** Parses the entire file, calls OnEntry each time
            new msgid/msgstr pair is found.

            @return false if parsing failed, true otherwise
         */
        bool Parse();

    protected:
        /** Called when new entry was parsed. Parsing continues
            if returned value is true and is cancelled if it
            is false.
         */
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
                             unsigned lineNumber) = 0;

        /** Called when new deleted entry was parsed. Parsing continues
            if returned value is true and is cancelled if it
            is false. Defaults to an empty implementation.
         */
        virtual bool OnDeletedEntry(const std::vector<std::wstring>& /*deletedLines*/,
                                    const std::wstring& /*flags*/,
                                    const std::vector<std::wstring>& /*references*/,
                                    const std::wstring& /*comment*/,
                                    const std::vector<std::wstring>& /*autocomments*/,
                                    unsigned /*lineNumber*/)
        {
            return true;
        }

        /// Textfile being parsed.
        std::istream *m_textFile;

        /// Whether the header should be parsed or not
        bool m_ignoreHeader;
};

#endif // _CATALOG_HPP_
