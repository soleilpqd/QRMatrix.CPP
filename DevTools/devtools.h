/*
    QRMatrix - QR pixels presentation.
    Copyright © 2023 duongpq/soleilpqd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DEVTOOLS_H
#define DEVTOOLS_H

#include <string>
#include <stdio.h>
#include <thread>

using namespace std;

#define LOG(...) DevTools::printLog(__FILE_NAME__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__, NULL)

class DevTools {
public:
    static const thread::id mainThreadId;
    /// Use LOG macro instead
    static void printLog(const char* fileName, const char* funcName, int lineNumber, const char *text...);
    static string getHex(unsigned char value);
    static string getHex(unsigned char* value, int count);
    static string getBin(unsigned char value);
    static string getBin(unsigned char* value, int count);
};

#endif // DEVTOOLS_H
