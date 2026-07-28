/*
 *  IXWebSocketHttpHeaders.h
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2018 Machine Zone, Inc. All rights reserved.
 */

#include "IXWebSocketHttpHeaders.h"

#include "IXSocket.h"
#include <algorithm>

namespace ix
{
    std::pair<bool, WebSocketHttpHeaders> parseHttpHeaders(
        std::unique_ptr<Socket>& socket, const CancellationRequest& isCancellationRequested)
    {
        WebSocketHttpHeaders headers;

        char line[1024];
        int i;

        while (true)
        {
            int colon = 0;

            for (i = 0; i < 2 || (i < 1023 && line[i - 2] != '\r' && line[i - 1] != '\n'); ++i)
            {
                if (!socket->readByte(line + i, isCancellationRequested))
                {
                    return std::make_pair(false, headers);
                }

                if (line[i] == ':' && colon == 0)
                {
                    colon = i;
                }
            }
            if (line[0] == '\r' && line[1] == '\n')
            {
                break;
            }

            // line is a single header entry. split by ':', and add it to our
            // header map. ignore lines with no colon.
            if (colon > 0)
            {
                line[i] = '\0';
                std::string lineStr(line);

                int start = colon + 1;
                while (start < (int) lineStr.size() && lineStr[start] == ' ')
                {
                    start++;
                }

                std::string name(lineStr.substr(0, colon));
                std::string value;
                if (start < (int) lineStr.size())
                {
                    value = lineStr.substr(start);
                    // trim trailing whitespace (\r, \n, spaces)
                    value.erase(std::find_if(value.rbegin(),
                                             value.rend(),
                                             [](unsigned char c) { return !std::isspace(c); })
                                    .base(),
                                value.end());
                }

                headers[name] = value;
            }
        }

        return std::make_pair(true, headers);
    }
} // namespace ix
