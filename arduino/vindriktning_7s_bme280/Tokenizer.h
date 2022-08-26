/*
 *   Copyright 2022 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#include <Arduino.h>

class Tokenizer {
public:
    Tokenizer();
    ~Tokenizer();

    bool tokenizeFromSerial();
    bool tokensReady() const;
    int numTokens() const;
    String operator [] (int);

private:
    void pushToken();
    String m_partialToken="";  // Initialised to nothing.
    char **m_tokens;
    int m_tokensSoFar = 0;
    int m_numTokens = 0;
    bool m_tokensReady = false;
};
