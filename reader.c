#include "reader.h"

#include "debug.h"
#include "exception.h"
#include "types.h"
#include "valuestack.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof((a)[0]))

typedef struct {
    char* cursor;
    char* token;
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

static char* strDup(const char* begin, const char* end)
{
    int length = end - begin;
    char* dup = malloc(length+1);
    memcpy(dup, begin, length);
    dup[length] = 0;
    return dup;
}

static void tokeniser_advance(Tokeniser* t)
{
    ASSERT(!tokeniser_eof(t), "next() on empty tokeniser");

    skipSpace(t);
    if (*t->cursor == 0) {
        if (t->token != NULL) {
            free(t->token);
        }
        t->token = NULL;
        return;
    }

    const char* punctChars = "()'";
    for (int i = 0; punctChars[i] != 0; i++) {
        if (*t->cursor == punctChars[i]) {
            t->cursor++;
            t->token = strDup(punctChars + i, punctChars + i + 1);
            return;
        }
    }

    const char* ret = t->cursor;
    if (*t->cursor == '"') {
        // Scan out a string.
        t->cursor++;
        for (char* wp = ++t->cursor; *t->cursor != 0; t->cursor++) {
            switch (*t->cursor) {
            case '"':
                // Clobber the closing "
                t->token = strDup(ret, wp);
                t->cursor++;
                return;

            case '\\':
                t->cursor++;
                switch (*t->cursor) {
                    case 0:     THROW("Expected \", got EOF");
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
        THROW("Expected \", got EOF");
    }
    else {
        // Scan out a token
        for (t->cursor++; *t->cursor != 0; t->cursor++) {
            if (isspace(*t->cursor)) {
                t->token = strDup(ret, t->cursor++);
                return;
            }
            switch (*t->cursor) {
            case '(': case ')': case '\'': case ' ': case '\t':
                t->token = strDup(ret, t->cursor);
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

    if (*t == 0)
        return 0;

    *value = 0;
    for ( ; *t != 0; t++) {
        if (isdigit(*t)) {
            *value = (*value * 10) + (*t - '0');
        }
        else {
            return 0;
        }
    }
    *value *= sign;
    return 1;
}

static Pointer readForm(Tokeniser* t);
static Pointer readList(Tokeniser* t)
{
    if (tokeniser_eof(t)) {
        THROW("Expected ), got EOF");
    }

    const char* token = tokeniser_peek(t);
    if (strEq(token, ")")) {
        tokeniser_next(t);
        return nil_make();
    }

    StackIndex carIndex = PUSH(readForm(t));
    StackIndex cdrIndex = PUSH(readList(t));

    Pointer ret = pair_make(GET(carIndex), GET(cdrIndex));

    DROP(2);

    return ret;
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
    if (strEq(token, "nil")) {
        return nil_make();
    }
    if (strEq(token, "true")) {
        return boolean_make(1);
    }
    if (strEq(token, "false")) {
        return boolean_make(0);
    }
    int intValue;
    if (readInteger(token, &intValue)) {
        return integer_make(intValue);
    }
    return symbol_make(token);
}

Pointer readLine(char* input)
{
    Tokeniser t;
    tokeniser_init(&t, input);
    return readForm(&t);
}
