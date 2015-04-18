#include "reader.h"

#include "exception.h"

#include <ctype.h>
#include <stdlib.h>

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof((a)[0]))

typedef struct {
    char* input;
    char* cursor;
} Tokeniser;

static void skip_ws(Tokeniser* t)
{
    while (isspace(*t->cursor)) {
        t->cursor++;
    }
}

static int tokeniser_eof(Tokeniser* t)
{
    skip_ws(t);
    return *t->cursor == 0;
}

static const char* tokeniser_next(Tokeniser* t)
{
    if (tokeniser_eof(t)) {
        return NULL;
    }

    // These must be single chars, but we specify them as strings to we can
    // return them directly.
    const char* punctChars[] = {
        "(", ")", "'"
    };
    for (int i = 0; i < ARRAY_SIZE(punctChars); i++) {
        if (*t->cursor == punctChars[i][0]) {
            t->cursor++;
            return punctChars[i];
        }
    }

    const char* ret = t->cursor;
    if (*t->cursor == '"') {
        // Scan out a string.
        *t->cursor = ':'; //XXX: TEMPORARY
        for (char* wp = ++t->cursor; *t->cursor != 0; t->cursor++) {
            switch (*t->cursor) {
            case '"':
                // Clobber the closing "
                *wp = 0;
                t->cursor++;
                return ret;

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
                return ret;
            }
            switch (*t->cursor) {
            case '(': case ')': case '\'': case ' ': case '\t':
                *t->cursor++ = 0;
                return ret;
            }
        }
    }
    return ret;
}

static Pointer readForm(Tokeniser* t)
{
    if (tokeniser_eof(t)) {
        return nil_make();
    }

    Pointer string = string_make(tokeniser_next(t));
    return pair_make(string, readForm(t));
}

Pointer readLine(char* input)
{
    Tokeniser t = { input, input };
    return readForm(&t);
}
