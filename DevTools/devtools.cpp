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

#include "devtools.h"
#include <iostream>
#include <cstdarg>

const thread::id DevTools::mainThreadId = this_thread::get_id();

string DevTools::getHex(unsigned char value) {
    char result[3]; // 3rd char is \0
    snprintf(result, 3, "%X", value);
    if (result[1] == 0) {
        result[1] = result[0];
        result[0] = '0';
    }
    return string(result);
}

string DevTools::getHex(unsigned char* value, int count) {
    string result = "";
    for (int idx = 0; idx < count; idx++) {
        string item = DevTools::getHex(value[idx]);
        result.append(item);
        if (idx != count - 1) {
            result.append(" ");
        }
    }
    return result;
}

string DevTools::getBin(unsigned char value) {
    char result[8];
    unsigned int pattern = 0x01 << 7;
    for (int idx = 0; idx < 8; idx++) {
        unsigned char mask = pattern >> idx;
        result[idx] = (value & mask) > 0 ? '1' : '0';
    };
    return string(result, 8);
}

string DevTools::getBin(unsigned char* value, int count) {
    string result = "BIN      │ 0X │ Dec │ Id\n─────────┼────┼─────┼─────\n";
    char hex[3]; // 3rd char is \0
    for (int idx = 0; idx < count; idx++) {
        string item = DevTools::getBin(value[idx]);
        result.append(item);
        result.append(" │ ");
        snprintf(hex, 3, "%02X", value[idx]);
        result.append(hex);
        result.append(" │ ");
        if (value[idx] < 100) {
            result.append(" ");
        }
        if (value[idx] < 10) {
            result.append(" ");
        }
        result.append(to_string(value[idx]));
        result.append(" │ ");
        result.append(to_string((idx + 1) * 8));
        if (idx != count - 1) {
            result.append("\n");
        }
    }
    return result;
}

void DevTools::printLog(const char* fileName, const char* funcName, int lineNumber, const char *text...) {
    thread::id curThread = this_thread::get_id();
    bool isMain = curThread == mainThreadId;
    va_list args;
    va_start(args, text);
    cout << "📌 ";
    if (isMain) {
        cout << "MAIN_THREAD";
    } else {
        cout << "THREAD: " << curThread;
    }
    cout << " [" << fileName << " " << lineNumber << "] " << funcName << endl;
    bool isKey = true;
    while (text != NULL) {
        cout << text;
        if (!isKey) {
            cout << endl;
        }
        text = va_arg(args, const char*);
        isKey = !isKey;
    }
    cout << "🏁" << endl;
    va_end(args);
}
