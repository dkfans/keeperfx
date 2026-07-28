/*
 * Lightweight URL & URI parser (RFC 1738, RFC 3986)
 * https://github.com/corporateshark/LUrlParser
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2015 Sergey Kosarevsky (sk@linderdaum.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *  IXUrlParser.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone, Inc. All rights reserved.
 */

#include "IXUrlParser.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace
{
    enum LUrlParserError
    {
        LUrlParserError_Ok = 0,
        LUrlParserError_Uninitialized = 1,
        LUrlParserError_NoUrlCharacter = 2,
        LUrlParserError_InvalidSchemeName = 3,
        LUrlParserError_NoDoubleSlash = 4,
        LUrlParserError_NoSlash = 5,
        LUrlParserError_MissingClosingBracketForIPv6 = 6,
        LUrlParserError_InvalidCharacterAfterIPv6Host = 7,
        LUrlParserError_MissingHost = 8,
        LUrlParserError_InvalidPort = 9,
        LUrlParserError_UnmatchedClosingBracketInHost = 10,
        LUrlParserError_InvalidOpeningBracketInHost = 11,
        LUrlParserError_InvalidPortSeparator = 12,
        LUrlParserError_MissingPortAfterColon = 13,
        LUrlParserError_EmptyIPv6Host = 14,
        LUrlParserError_UnbracketedIPv6Host = 15,
        LUrlParserError_MultipleAtSignsInAuthority = 16,
    };

    class clParseURL
    {
    public:
        LUrlParserError m_ErrorCode;
        std::string m_Scheme;
        std::string m_Host;
        std::string m_Port;
        std::string m_Path;
        std::string m_Query;
        std::string m_Fragment;
        std::string m_UserName;
        std::string m_Password;

        clParseURL()
            : m_ErrorCode(LUrlParserError_Uninitialized)
        {
        }

        /// return 'true' if the parsing was successful
        bool IsValid() const
        {
            return m_ErrorCode == LUrlParserError_Ok;
        }

        /// helper to convert the port number to int, return 'true' if the port is valid (within the
        /// 0..65535 range)
        bool GetPort(int* OutPort) const;

        /// parse the URL
        static clParseURL ParseURL(const std::string& URL);

    private:
        explicit clParseURL(LUrlParserError ErrorCode)
            : m_ErrorCode(ErrorCode)
        {
        }
    };

    static bool IsSchemeValid(const std::string& SchemeName)
    {
        for (auto c : SchemeName)
        {
            if (!isalpha(c) && c != '+' && c != '-' && c != '.') return false;
        }

        return true;
    }

    bool clParseURL::GetPort(int* OutPort) const
    {
        if (!IsValid())
        {
            return false;
        }

        int Port = atoi(m_Port.c_str());

        if (Port <= 0 || Port > 65535)
        {
            return false;
        }

        if (OutPort)
        {
            *OutPort = Port;
        }

        return true;
    }

    // Practical URI parser for ws/wss/http/https URLs.
    //
    // Standards references:
    // - RFC 3986: generic URI syntax used for scheme, authority, path, query, fragment.
    // - RFC 3986 Section 3.2.2: IP-literal host syntax (bracketed IPv6 in authority).
    // - RFC 6455 Section 3: ws/wss URI schemes (default ports handled in getProtocolPort).
    //
    // Note: this is not a full RFC validator; it is a fast parser with targeted syntax checks.
    clParseURL clParseURL::ParseURL(const std::string& URL)
    {
        clParseURL Result;
        const std::size_t npos = std::string::npos;
        std::size_t current = 0;

        // Step 1: read scheme (<scheme>:) and normalize to lowercase.
        std::size_t schemeEnd = URL.find(':', current);
        if (schemeEnd == npos)
        {
            return clParseURL(LUrlParserError_NoUrlCharacter);
        }

        Result.m_Scheme = URL.substr(current, schemeEnd - current);
        if (!IsSchemeValid(Result.m_Scheme))
        {
            return clParseURL(LUrlParserError_InvalidSchemeName);
        }

        std::transform(Result.m_Scheme.begin(),
                       Result.m_Scheme.end(),
                       Result.m_Scheme.begin(),
                       ::tolower);

        current = schemeEnd + 1;

        // Step 2: require authority prefix "//".
        if (current + 1 >= URL.size() || URL[current] != '/' || URL[current + 1] != '/')
        {
            return clParseURL(LUrlParserError_NoDoubleSlash);
        }
        current += 2;

        // Step 3: locate the authority boundary (<authority> ends at '/' or '?').
        std::size_t authorityEnd = URL.find_first_of("/?", current);
        if (authorityEnd == npos)
        {
            authorityEnd = URL.size();
        }

        // Step 4: parse optional user info (<user>[:<password>]@).
        bool hasUserInfo = false;
        std::size_t atPos = URL.find('@', current);
        if (atPos != npos && atPos < authorityEnd)
        {
            hasUserInfo = true;
        }

        if (hasUserInfo)
        {
            // User info can contain ':' but must contain exactly one '@' separator
            // within the authority segment.
            if (URL.find('@', atPos + 1) != npos && URL.find('@', atPos + 1) < authorityEnd)
            {
                return clParseURL(LUrlParserError_MultipleAtSignsInAuthority);
            }

            std::size_t colonPos = URL.find(':', current);
            if (colonPos != npos && colonPos < atPos)
            {
                Result.m_UserName = URL.substr(current, colonPos - current);
                Result.m_Password = URL.substr(colonPos + 1, atPos - colonPos - 1);
            }
            else
            {
                Result.m_UserName = URL.substr(current, atPos - current);
            }

            current = atPos + 1;
        }

        // Step 5: parse host and optional port from authority tail.
        authorityEnd = URL.find_first_of("/?", current);
        if (authorityEnd == npos)
        {
            authorityEnd = URL.size();
        }

        std::string authority = URL.substr(current, authorityEnd - current);
        std::size_t openingBracketPos = authority.find('[');
        std::size_t closingBracketPos = authority.find(']');

        // Malformed authority checks (RFC 3986):
        // - IP-literals must be enclosed in '[' and ']' (Section 3.2.2).
        // - A closing bracket without an opening bracket is invalid.
        // - An opening bracket is only valid at the beginning of host when parsing [IPv6].
        // - Unbracketed multi-colon authorities are either invalid separators or IPv6 literals
        //   missing required brackets.
        if (closingBracketPos != npos && openingBracketPos == npos)
        {
            return clParseURL(LUrlParserError_UnmatchedClosingBracketInHost);
        }

        if (openingBracketPos != npos && openingBracketPos != 0)
        {
            return clParseURL(LUrlParserError_InvalidOpeningBracketInHost);
        }

        if (openingBracketPos == npos)
        {
            const auto colonCount = std::count(authority.begin(), authority.end(), ':');
            if (colonCount > 1)
            {
                bool looksLikeUnbracketedIPv6 =
                    authority.find('.') == npos &&
                    authority.find_first_not_of("0123456789abcdefABCDEF:") == npos;

                if (looksLikeUnbracketedIPv6)
                {
                    return clParseURL(LUrlParserError_UnbracketedIPv6Host);
                }

                return clParseURL(LUrlParserError_InvalidPortSeparator);
            }
        }

        if (current < authorityEnd && URL[current] == '[')
        {
            // Step 5a: bracketed IPv6 host: [<ipv6>][:<port>]
            std::size_t closeBracket = URL.find(']', current + 1);
            if (closeBracket == npos || closeBracket >= authorityEnd)
            {
                // Bracketed IPv6 literal started but did not terminate before authority end.
                return clParseURL(LUrlParserError_MissingClosingBracketForIPv6);
            }

            Result.m_Host = URL.substr(current + 1, closeBracket - current - 1);
            if (Result.m_Host.empty())
            {
                return clParseURL(LUrlParserError_EmptyIPv6Host);
            }

            std::size_t afterBracket = closeBracket + 1;
            if (afterBracket < authorityEnd)
            {
                if (URL[afterBracket] != ':')
                {
                    // After ] only ':' is allowed before authority terminates.
                    return clParseURL(LUrlParserError_InvalidCharacterAfterIPv6Host);
                }
                Result.m_Port = URL.substr(afterBracket + 1, authorityEnd - afterBracket - 1);
                if (Result.m_Port.empty())
                {
                    // ':' present, but no port digits followed.
                    return clParseURL(LUrlParserError_MissingPortAfterColon);
                }
            }
        }
        else
        {
            // Step 5b: regular host: <host>[:<port>]
            std::size_t colonPos = URL.find(':', current);
            if (colonPos != npos && colonPos < authorityEnd)
            {
                Result.m_Host = URL.substr(current, colonPos - current);
                Result.m_Port = URL.substr(colonPos + 1, authorityEnd - colonPos - 1);
                if (Result.m_Port.empty())
                {
                    // ':' present, but no port digits followed.
                    return clParseURL(LUrlParserError_MissingPortAfterColon);
                }
            }
            else
            {
                Result.m_Host = URL.substr(current, authorityEnd - current);
            }
        }

        // Step 5c: validate host/port extracted from authority.
        if (Result.m_Host.empty())
        {
            // Host is mandatory in authority form: //host[:port]
            return clParseURL(LUrlParserError_MissingHost);
        }

        if (Result.m_Port.find_first_not_of("0123456789") != std::string::npos)
        {
            // Port must be decimal digits only (conversion/range checked later by GetPort).
            return clParseURL(LUrlParserError_InvalidPort);
        }

        current = authorityEnd;

        // Step 6: if authority reaches end of string, URL parsing is complete.
        if (current >= URL.size())
        {
            Result.m_ErrorCode = LUrlParserError_Ok;
            return Result;
        }

        // Step 7: parse path start (either '/' or query-only '?').
        if (URL[current] != '/' && URL[current] != '?')
        {
            return clParseURL(LUrlParserError_NoSlash);
        }

        if (URL[current] == '/')
        {
            ++current;
        }

        // Step 8: parse path up to '?' or '#'.
        std::size_t pathEnd = URL.find_first_of("?#", current);
        if (pathEnd == npos)
        {
            Result.m_Path = URL.substr(current);
            Result.m_ErrorCode = LUrlParserError_Ok;
            return Result;
        }

        Result.m_Path = URL.substr(current, pathEnd - current);
        current = pathEnd;

        // Step 9: parse optional query after '?', up to '#'.
        if (current < URL.size() && URL[current] == '?')
        {
            ++current;

            std::size_t queryEnd = URL.find('#', current);
            if (queryEnd == npos)
            {
                Result.m_Query = URL.substr(current);
                Result.m_ErrorCode = LUrlParserError_Ok;
                return Result;
            }

            Result.m_Query = URL.substr(current, queryEnd - current);
            current = queryEnd;
        }

        // Step 10: parse optional fragment after '#', to end of URL.
        if (current < URL.size() && URL[current] == '#')
        {
            ++current;
            Result.m_Fragment = URL.substr(current);
        }

        Result.m_ErrorCode = LUrlParserError_Ok;
        return Result;
    }

    int getProtocolPort(const std::string& protocol)
    {
        if (protocol == "ws" || protocol == "http")
        {
            return 80;
        }
        else if (protocol == "wss" || protocol == "https")
        {
            return 443;
        }
        return -1;
    }
} // namespace

namespace ix
{
    bool UrlParser::parse(const std::string& url,
                          std::string& protocol,
                          std::string& host,
                          std::string& path,
                          std::string& query,
                          int& port)
    {
        bool isProtocolDefaultPort;
        return parse(url, protocol, host, path, query, port, isProtocolDefaultPort);
    }

    bool UrlParser::parse(const std::string& url,
                              std::string& protocol,
                              std::string& host,
                              std::string& path,
                              std::string& query,
                              int& port,
                              bool& isProtocolDefaultPort)
    {
        clParseURL res = clParseURL::ParseURL(url);

        if (!res.IsValid())
        {
            return false;
        }

        protocol = res.m_Scheme;
        host = res.m_Host;
        path = res.m_Path;
        query = res.m_Query;

        const auto protocolPort = getProtocolPort(protocol);
        if (!res.GetPort(&port))
        {
            port = protocolPort;
        }
        isProtocolDefaultPort = port == protocolPort;

        if (path.empty())
        {
            path = "/";
        }
        else if (path[0] != '/')
        {
            path = '/' + path;
        }

        if (!query.empty())
        {
            path += "?";
            path += query;
        }

        return true;
    }

} // namespace ix
