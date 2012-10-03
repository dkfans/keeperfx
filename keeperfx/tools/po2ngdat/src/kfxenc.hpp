/*
 * kfxenc.hpp
 *
 *  Created on: 03-10-2012
 *      Author: tomasz
 */

#ifndef KFXENC_HPP_
#define KFXENC_HPP_

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
