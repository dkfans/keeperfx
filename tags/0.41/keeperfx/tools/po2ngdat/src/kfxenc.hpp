/******************************************************************************/
// PO translation to engine DAT files converter for KeeperFX
/******************************************************************************/
/** @file kfxenc.hpp
 *     Header file for kfxenc.cpp.
 * @par Purpose:
 *     Contains code to read code page tables and use them to encode
 *     UTF strings into different code pages.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis <listom@gmail.com>
 * @date     5 Aug 2012 - 22 Sep 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFXENC_HPP_
#define KFXENC_HPP_

#include <stdint.h> // <cstdint> seem to require c++11
#include <map>

class UnicodeConvert
{
    typedef std::map<uint32_t,uint16_t> ConvertCodes;

    public:
        /// Default ctor. Creates empty converter, you have to call Load.
        UnicodeConvert();

        ~UnicodeConvert();

        /** Loads encoding from file.
         *  @param flags  Flags combination.
         */
        bool Load(const std::string& enc_file, int flags = 0);

        /** Adds entry to the encoding array.
         * @param uni
         * @param simp
         */
        void AddItem(uint32_t uni,uint16_t simp);

        /** Convert a single unicode char to its code.
         * @param uni
         * @return
         */
        uint16_t EncodeChar(uint32_t uni);

        /** Convert a single unicode string to local string.
         * @param uni
         * @return
         */
        std::string EncodeU32String(const std::vector<uint32_t> &unistr);

        /** Convert a single unicode string to local string.
         * @param uni
         * @return
         */
        std::string EncodeU16String(const std::wstring &unistr);

    private:
        ConvertCodes m_items;

        bool m_isOk;
        std::string m_fileName;
};

#endif /* KFXENC_HPP_ */
