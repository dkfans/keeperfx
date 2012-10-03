
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "utf8_unchecked.h"

#include "kfxenc.hpp"

#define encLogError(format,args...) printf(format "\n", ## args)

UnicodeConvert::UnicodeConvert()
{
    m_isOk = false;
}

UnicodeConvert::~UnicodeConvert()
{
    m_isOk = false;
}

bool UnicodeConvert::Load(const std::string& enc_file, int flags)
{
    std::fstream f;
    size_t currentLine;
    size_t errors;

    m_isOk = false;
    m_fileName = enc_file;

    /* Load the .po file: */
    f.open(enc_file.c_str(), std::fstream::in);

    currentLine = 0;
    errors = 0;
    while (!f.eof())
    {
        // read next line and strip insignificant whitespace from it:
        std::string line;
        std::getline(f, line);
        currentLine++;

        size_t pos;
        pos = line.find_first_not_of(" \n\r\t");
        // If empty line or comment
        if ((pos == std::string::npos) || (line[pos] == '#'))
            continue;

        uint16_t code = strtol(line.substr(pos).c_str(),NULL,16);
        if (code == 0)
        {
            errors++;
            encLogError("Invalid hex number in encoding entry at line %ld.", (long)currentLine);
            continue;
        }
        pos = line.find("\t",pos);
        // If invalid line
        if (pos == std::string::npos)
        {
            errors++;
            encLogError("Encoding entry at line %ld has no tab separated char.", (long)currentLine);
            continue;
        }
        // Convert target char to utf-32
        std::vector<uint32_t> unistr;
        utf8::utf8to32(line.begin()+pos+1, line.end(), back_inserter(unistr));
        if (unistr.size() < 1)
        {
            errors++;
            encLogError("Couldn't read utf8 char after tab at line %ld.", (long)currentLine);
            continue;
        }
        AddItem(unistr[0],code);
    }

    if (errors > 0)
    {
        encLogError("Couldn't load file %s, it is probably corrupted.", enc_file.c_str());
        return false;
    }

    m_isOk = true;

    f.close();

    return true;
}

void UnicodeConvert::AddItem(uint32_t uni,uint16_t simp)
{
    m_items.insert ( std::pair<uint32_t,uint16_t>(uni,simp) );
}

uint16_t UnicodeConvert::EncodeChar(uint32_t uni)
{
    ConvertCodes::iterator it = m_items.find(uni);
    if (it != m_items.end()) {
        return it->second;
    }
    if (uni < 0xff)
        return uni;
    return '_';
}

std::string UnicodeConvert::EncodeU32String(const std::vector<uint32_t> &unistr)
{
    std::stringstream ss;
    for (std::vector<uint32_t>::const_iterator it = unistr.begin(); it!=unistr.end(); ++it)
    {
        uint16_t c = EncodeChar(*it);
        ss << (char)c;
        if (c > 0xff)
            ss << (char)(c>>8);
    }
    return ss.str();
}

std::string UnicodeConvert::EncodeU16String(const std::wstring &unistr)
{
    std::stringstream ss;
    for (std::wstring::const_iterator it = unistr.begin(); it!=unistr.end(); )
    {
        uint32_t cp = utf8::internal::mask16(*it++);
        // Take care of surrogate pairs first
        if (utf8::internal::is_lead_surrogate(cp)) {
            uint32_t trail_surrogate = utf8::internal::mask16(*it++);
            cp = (cp << 10) + trail_surrogate + utf8::internal::SURROGATE_OFFSET;
        }
        uint16_t c = EncodeChar(cp);
        ss << (char)(c);
        if (c > 0xff)
            ss << (char)(c>>8);
    }
    return ss.str();
}
