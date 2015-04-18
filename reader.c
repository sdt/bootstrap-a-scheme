#include "reader.h"

#include "debug.h"
#include "exception.h"
#include "types.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof((a)[0]))

typedef struct {
    char*       cursor;
    const char* token;
} Tokeniser;

static void skipSpace(Tokeniser* t)
{
    while (isspace(*t->cursor)) {
        t->cursor++;
    }
}

static int strEq(const char* s, const char* t)
{
    return strcmp(s, t) == 0;
}

static int tokeniser_eof(Tokeniser* t)
{
    return t->token == NULL;
}

static void tokeniser_advance(Tokeniser* t)
{
    ASSERT(!tokeniser_eof(t), "next() on empty tokeniser");

    skipSpace(t);
    if (*t->cursor == 0) {
        t->token = NULL;
        return;
    }

    // These must be single chars, but we specify them as strings to we can
    // return them directly.
    const char* punctChars[] = {
        "(", ")", "'"
    };
    for (int i = 0; i < ARRAY_SIZE(punctChars); i++) {
        if (*t->cursor == punctChars[i][0]) {
            t->cursor++;
            t->token = punctChars[i];
            return;
        }
    }

    t->token = t->cursor;
    if (*t->cursor == '"') {
        // Scan out a string.
        *t->cursor = ':'; //XXX: TEMPORARY
        for (char* wp = ++t->cursor; *t->cursor != 0; t->cursor++) {
            switch (*t->cursor) {
            case '"':
                // Clobber the closing "
                *wp = 0;
                t->cursor++;
                return;

            case '\\':
                t->cursor++;
                switch (*t->cursor) {
                    case 0:     throw("Expected \", got EOF");
                    case 'n':   *wp++ = '\n'; break;
                    case 't':   *wp++ = '\t'; break;
                    case '\\':  *wp++ = '\\'; break;
                    default:    *wp++ = *t->cursor; break;
                }
                break;

            default:
                *wp++ = *t->cursor;
                break;
            }
        }
        throw("Expected \", got EOF");
    }
    else {
        // Scan out a token
        for (t->cursor++; *t->cursor != 0; t->cursor++) {
            if (isspace(*t->cursor)) {
                *t->cursor++ = 0;
                return;
            }
            switch (*t->cursor) {
            case '(': case ')': case '\'': case ' ': case '\t':
                *t->cursor++ = 0;
                return;
            }
        }
    }
    return;
}

static const char* tokeniser_peek(Tokeniser* t)
{
    ASSERT(!tokeniser_eof(t), "peek() on empty tokeniser");
    return t->token;
}

static const char* tokeniser_next(Tokeniser* t)
{
    ASSERT(!tokeniser_eof(t), "next() on empty tokeniser");
    const char* token = t->token;
    tokeniser_advance(t);
    return token;
}

static void tokeniser_init(Tokeniser* t, char* input)
{
    t->cursor = input;
    t->token  = input;
    tokeniser_advance(t);
}
static int readInteger(const char* t, int* value)
{
    int sign = 1;
    switch (*t) {
        case '+': t++; break;
        case '-': sign = -1; t++; break;
        default: break;
    }

    *value = 0;
    for ( ; *t != 0; t++) {
        if (isdigit(*t)) {
            *value = (*value * 10) + (*t - '0');
        }
        else {
            return 0;
        }
    }
    return *value * sign;
}

static Pointer readForm(Tokeniser* t);
static Pointer readList(Tokeniser* t)
{
    if (tokeniser_eof(t)) {
        throw("Expected ), got EOF");
    }

    const char* token = tokeniser_peek(t);
    if (strEq(token, ")")) {
        tokeniser_next(t);
        return nil_make();
    }

    Pointer car = readForm(t);
    return pair_make(car, readList(t));
}

static Pointer readForm(Tokeniser* t)
{
    if (tokeniser_eof(t)) {
        return nil_make();
    }

    const char* token = tokeniser_next(t);

    if (strEq(token, "(")) {
        return readList(t);
    }
    if (token[0] == '"') {
        return string_make(token + 1); // strip the leading quote
    }
    int intValue;
    if (readInteger(token, &intValue)) {
        return integer_make(intValue);
    }
    //TODO: this should be a symbol
    return string_make(token);
}

Pointer readLine(char* input)
{
    Tokeniser t;
    tokeniser_init(&t, input);
    return readForm(&t);
}
