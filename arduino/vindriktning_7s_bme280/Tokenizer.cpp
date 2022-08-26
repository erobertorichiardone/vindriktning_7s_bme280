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
 
#include "Tokenizer.h"

Tokenizer::Tokenizer()
{
    m_tokens = (char **)malloc(1 * sizeof(char *));
    m_tokens[0] = (char *)malloc(1 * sizeof(char));
}

Tokenizer::~Tokenizer()
{
    for (int i = 0; i < sizeof(m_tokens); ++i) {
        free(m_tokens[i]);
    }
    free(m_tokens);
}

void Tokenizer::pushToken()
{
    if (m_partialToken.length() == 0) {
        return;
    }

    if (sizeof(m_tokens) / sizeof(char*) <= m_tokensSoFar) {
        m_tokens = (char **)realloc(m_tokens, (m_tokensSoFar + 1) * sizeof(char *));
        m_tokens[m_tokensSoFar] = (char *)malloc((m_partialToken.length() + 1) * sizeof(char));
    } else {
        m_tokens[m_tokensSoFar] = (char *)realloc(m_tokens[m_tokensSoFar], (m_partialToken.length() + 1) * sizeof(char));
    }
 
    strcpy(m_tokens[m_tokensSoFar], m_partialToken.c_str());
    m_partialToken = "";
    ++m_tokensSoFar;
}

bool Tokenizer::tokenizeFromSerial()
{
    if (!Serial.available()) {
        return false;
    }

    char ch = Serial.read();
    Serial.print(ch);

    if (m_tokensReady) {
        m_tokensReady = false;
        m_tokensSoFar = 0;
        m_numTokens = 0;
    }

    if (ch == '\r' || ch == '\n') {  // Command recevied and ready.
        pushToken();
        m_numTokens = m_tokensSoFar;
        m_tokensSoFar = 0;
        m_tokensReady = true;
        return true;
    } else if (ch == ' ') {
        pushToken();
    } else {
        m_partialToken += ch;
    }

    return false;
}

bool Tokenizer::tokensReady() const
{
    return m_tokensReady;
}

int Tokenizer::numTokens() const
{
    if (m_tokensReady) {
        return m_numTokens;
    } else {
        return 0;
    }
}

String Tokenizer::operator [] (int i)
{
    if (m_tokensReady && i < m_numTokens) {
        return m_tokens[i];
    } else {
        return "";
    }
}
