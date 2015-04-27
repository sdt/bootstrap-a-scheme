#include "reader.h"

#include "debug.h"
#include "exception.h"
#include "symtab.h"
#include "types.h"
#include "valuestack.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* cursor;
    StackIndex tokenIndex;
    int eof;
} Tokeniser;

static void skipComment(Tokeniser* t)
{
    const char* p;
    for (p = 0; *p != 0 && *p != '\n'; p++) {

    }
    // The cursor is now either pointing at the trailing zero, or at the
    // newline (which will get skipped as whitespace).
    t->cursor = p;
}

static void skipSpace(Tokeniser* t)
{
    while (1) {
        char c = *t->cursor;
        if (c == ';') {
            skipComment(t);
        }
        else if (isspace(c)) {
            t->cursor++;
        }
        else {
            return;
        }
    }
}

static int isdelim(char c)
{
    switch (c) {
    case '(': case ')': case '\'': case '"':
        return 1;

    default:
        return 0;
    }
}

static int strEq(const char* s, const char* t)
{
    return strcmp(s, t) == 0;
}

static int tokeniser_eof(Tokeniser* t)
{
    return t->eof;
}

typedef void (CharSink)(char c, void* context);
static const char* stringEscape(const char* s, CharSink* emit, void* context)
{
    for (const char* p = s; ; p++) {
        char c = *p;
        switch (*p) {
        case 0:
        case '"':
            // Return p still pointing to the last char, so the caller can
            // decide what to do.
            return p;

        case '\\':
            p++;
            switch (c = *p) {
                case 'n':   emit('\n', context); break;
                case 't':   emit('\t', context); break;
                case '\\':  emit('\\', context); break;
                default:    emit(c,    context); break;
            }
            break;

        default:
            emit(c, context);
            break;
        }
    }
}

static void countChars(char c, void* context)
{
    int* count = (void*) context;
    count++;
}

static void copyChars(char c, void* context)
{
    char** dest = (char**) context;
    **dest = c;
    (*dest)++;
}

static Pointer makeToken(const char* start, int length)
{
    Pointer ptr = symbol_alloc(length);
    char* dest = (char*) symbol_get(ptr);
    memcpy(dest, start, length);
    dest[length] = 0;
    return ptr;
}

static void tokeniser_advance(Tokeniser* t)
{
    ASSERT(!tokeniser_eof(t), "next() on empty tokeniser");
    SET(t->tokenIndex, nil_make());

    skipSpace(t);
    if (*t->cursor == 0) {
        t->eof = 1;
        return;
    }

    const char* punctChars = "()'";
    for (int i = 0; punctChars[i] != 0; i++) {
        if (*t->cursor == punctChars[i]) {
            t->cursor++;
            SET(t->tokenIndex, makeToken(&punctChars[i], 1));
            return;
        }
    }

    if (*t->cursor == '"') {

        // Find out how long the escaped string is, and check it was terminated
        // properly.
        int length = 0;
        t->cursor++; // advance past the initial "
        const char* end = stringEscape(t->cursor, countChars, &length);
        if (*end != '"') {
            THROW("Expected \", got EOF");
        }

        // Now allocate a string and copy it in.
        Pointer ptr = string_alloc(length);
        char* dest = (char*) string_get(ptr);
        stringEscape(t->cursor, copyChars, &dest);
        dest[length] = 0;

        // Advance the cursor past the string.
        t->cursor = end + 1;

        SET(t->tokenIndex, ptr);
    }
    else {
        // Scan out a token
        const char* start = t->cursor;
        while (1) {
            char c = *t->cursor;
            if ((c == 0) || isspace(c) || isdelim(c)) {
                int length = t->cursor - start;
                SET(t->tokenIndex, makeToken(start, length));
                break;
            }
            // Advance the cursor now, not earlier.
            t->cursor++;
        }
    }
}

static void tokeniser_next(Tokeniser* t)
{
    tokeniser_advance(t);
}

static void tokeniser_init(Tokeniser* t, const char* input)
{
    t->cursor     = input;
    t->eof        = 0;
    t->tokenIndex = RESERVE();

    tokeniser_advance(t);
}

static Pointer tokeniser_token(Tokeniser* t)
{
    return GET(t->tokenIndex);
}

static void tokeniser_deinit(Tokeniser* t)
{
    DROP(1);
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

    Pointer token = tokeniser_token(t);
    if ((token.type == Type_symbol) && strEq(symbol_get(token), ")")) {
        tokeniser_next(t);
        return nil_make();
    }

    StackIndex carIndex = PUSH(readForm(t));
    StackIndex cdrIndex = PUSH(readList(t));

    Pointer ret = pair_make(carIndex, cdrIndex);

    DROP(2);

    return ret;
}

static Pointer readForm(Tokeniser* t)
{
    if (tokeniser_eof(t)) {
        return nil_make();
    }

    Pointer ret = tokeniser_token(t);

    // If ret is a string, it's ready to return
    if (ret.type == Type_symbol) {
        const char* token = symbol_get(ret);

        int intValue;
        if (strEq(token, "(")) {
            tokeniser_next(t);  // consume the '(', read the list and
            return readList(t); // return immediately to avoid hitting the
                                // tokeniser_next() below
        }
        else if (strEq(token, "nil")) {
            ret = nil_make();
        }
        else if (strEq(token, "true")) {
            ret = boolean_make(1);
        }
        else if (strEq(token, "false")) {
            ret = boolean_make(0);
        }
        else if (readInteger(token, &intValue)) {
            ret = integer_make(intValue);
        }
        else {
            // Insert the symbol into the symbol table. We may or may not get
            // the same symbol back.
            ret = symtab_insert(ret);
        }
    }

    PUSH(ret);
    tokeniser_next(t);
    return POP();
}

Pointer readLine(const char* input)
{
    Tokeniser t;
    tokeniser_init(&t, input);
    Pointer ptr = readForm(&t); tokeniser_deinit(&t);
    return ptr;
}
