/*
 * CentiTOML
 * <http://github.com/SimLV/centitoml>
 *
 * Copyright (c) 2022 CK Tan, TheSim
 * Based on tomlc99 by CK Tan
 * https://github.com/cktan/tomlc99

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "toml.h"

static void *(*ppmalloc)(size_t) = malloc;

static void (*ppfree)(void *) = free;

void toml_set_memutil(void *(*xxmalloc)(size_t), void (*xxfree)(void *))
{
    if (xxmalloc)
        ppmalloc = xxmalloc;
    if (xxfree)
        ppfree = xxfree;
}

#define MALLOC(a) ppmalloc(a)
#define FREE(a) ppfree(a)
#define true 1
#define false 0

static inline void xfree(const void *x)
{
    if (x)
        FREE((void *) (intptr_t) x);
}

static void toml_free(VALUE *val)
{
    value_fini(val);
}

static char *STRNDUP(const char *s, size_t n)
{
    size_t len = strnlen(s, n);
    char *p = MALLOC(len + 1);
    if (p)
    {
        memcpy(p, s, len);
        p[len] = 0;
    }
    return p;
}

/**
 * Convert a char in utf8 into UCS, and store it in *ret.
 * Return #bytes consumed or -1 on failure.
 */
int toml_utf8_to_ucs(const char *orig, int len, int64_t *ret)
{
    const unsigned char *buf = (const unsigned char *) orig;
    unsigned i = *buf++;
    int64_t v;

    /* 0x00000000 - 0x0000007F:
       0xxxxxxx
    */
    if (0 == (i >> 7))
    {
        if (len < 1)
            return -1;
        v = i;
        return *ret = v, 1;
    }
    /* 0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx
    */
    if (0x6 == (i >> 5))
    {
        if (len < 2)
            return -1;
        v = i & 0x1f;
        for (int j = 0; j < 1; j++)
        {
            i = *buf++;
            if (0x2 != (i >> 6))
                return -1;
            v = (v << 6) | (i & 0x3f);
        }
        return *ret = v, (const char *) buf - orig;
    }

    /* 0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx
    */
    if (0xE == (i >> 4))
    {
        if (len < 3)
            return -1;
        v = i & 0x0F;
        for (int j = 0; j < 2; j++)
        {
            i = *buf++;
            if (0x2 != (i >> 6))
                return -1;
            v = (v << 6) | (i & 0x3f);
        }
        return *ret = v, (const char *) buf - orig;
    }

    /* 0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (0x1E == (i >> 3))
    {
        if (len < 4)
            return -1;
        v = i & 0x07;
        for (int j = 0; j < 3; j++)
        {
            i = *buf++;
            if (0x2 != (i >> 6))
                return -1;
            v = (v << 6) | (i & 0x3f);
        }
        return *ret = v, (const char *) buf - orig;
    }

    /* 0x00200000 - 0x03FFFFFF:
       111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (0x3E == (i >> 2))
    {
        if (len < 5)
            return -1;
        v = i & 0x03;
        for (int j = 0; j < 4; j++)
        {
            i = *buf++;
            if (0x2 != (i >> 6))
                return -1;
            v = (v << 6) | (i & 0x3f);
        }
        return *ret = v, (const char *) buf - orig;
    }

    /* 0x04000000 - 0x7FFFFFFF:
       1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (0x7e == (i >> 1))
    {
        if (len < 6)
            return -1;
        v = i & 0x01;
        for (int j = 0; j < 5; j++)
        {
            i = *buf++;
            if (0x2 != (i >> 6))
                return -1;
            v = (v << 6) | (i & 0x3f);
        }
        return *ret = v, (const char *) buf - orig;
    }
    return -1;
}

/**
 *	Convert a UCS char to utf8 code, and return it in buf.
 *	Return #bytes used in buf to encode the char, or
 *	-1 on error.
 */
int toml_ucs_to_utf8(int64_t code, unsigned char buf[6])
{
    /* http://stackoverflow.com/questions/6240055/manually-converting-unicode-codepoints-into-utf-8-and-utf-16
     */
    /* The UCS code values 0xd800–0xdfff (UTF-16 surrogates) as well
     * as 0xfffe and 0xffff (UCS noncharacters) should not appear in
     * conforming UTF-8 streams.
     */
    if (0xd800 <= code && code <= 0xdfff)
        return -1;
    if (0xfffe <= code && code <= 0xffff)
        return -1;

    /* 0x00000000 - 0x0000007F:
       0xxxxxxx
    */
    if (code < 0)
        return -1;
    if (code <= 0x7F)
    {
        buf[0] = (unsigned char) code;
        return 1;
    }

    /* 0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx
    */
    if (code <= 0x000007FF)
    {
        buf[0] = (unsigned char) (0xc0 | (code >> 6));
        buf[1] = (unsigned char) (0x80 | (code & 0x3f));
        return 2;
    }

    /* 0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x0000FFFF)
    {
        buf[0] = (unsigned char) (0xe0 | (code >> 12));
        buf[1] = (unsigned char) (0x80 | ((code >> 6) & 0x3f));
        buf[2] = (unsigned char) (0x80 | (code & 0x3f));
        return 3;
    }

    /* 0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x001FFFFF)
    {
        buf[0] = (unsigned char) (0xf0 | (code >> 18));
        buf[1] = (unsigned char) (0x80 | ((code >> 12) & 0x3f));
        buf[2] = (unsigned char) (0x80 | ((code >> 6) & 0x3f));
        buf[3] = (unsigned char) (0x80 | (code & 0x3f));
        return 4;
    }

    /* 0x00200000 - 0x03FFFFFF:
       111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x03FFFFFF)
    {
        buf[0] = (unsigned char) (0xf8 | (code >> 24));
        buf[1] = (unsigned char) (0x80 | ((code >> 18) & 0x3f));
        buf[2] = (unsigned char) (0x80 | ((code >> 12) & 0x3f));
        buf[3] = (unsigned char) (0x80 | ((code >> 6) & 0x3f));
        buf[4] = (unsigned char) (0x80 | (code & 0x3f));
        return 5;
    }

    /* 0x04000000 - 0x7FFFFFFF:
       1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x7FFFFFFF)
    {
        buf[0] = (unsigned char) (0xfc | (code >> 30));
        buf[1] = (unsigned char) (0x80 | ((code >> 24) & 0x3f));
        buf[2] = (unsigned char) (0x80 | ((code >> 18) & 0x3f));
        buf[3] = (unsigned char) (0x80 | ((code >> 12) & 0x3f));
        buf[4] = (unsigned char) (0x80 | ((code >> 6) & 0x3f));
        buf[5] = (unsigned char) (0x80 | (code & 0x3f));
        return 6;
    }

    return -1;
}

enum tokentype_t
{
    INVALID,
    DOT,
    COMMA,
    EQUAL,
    LBRACE,
    RBRACE,
    NEWLINE,
    LBRACKET,
    RBRACKET,
    STRING,
};
typedef enum tokentype_t tokentype_t;

typedef struct token_t token_t;
struct token_t
{
    tokentype_t tok;
    int lineno;
    char *ptr; /* points into context->start */
    int len;
    int eof;
};

typedef struct table_t
{
    VALUE *tab;
    VALUE *meta;
} table_t;

typedef struct context_t context_t;
struct context_t
{
    char *start;
    char *stop;
    char *errbuf;
    int errbufsz;

    token_t tok;
    VALUE *root;
    VALUE *meta; // Here we store metainfo

    table_t curtab;

    struct
    {
        int top;
        char *key[10];
        token_t tok[10];
    } tpath;
};

#define IDX_KIND 0
#define IDX_CHILDREN 1
#define IDX_IMPLICIT 2

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FLINE __FILE__ ":" TOSTRING(__LINE__)

static table_t table_dict_get(table_t from, const char *key)
{
    table_t ret = {
            .tab = value_dict_get(from.tab, key),
            .meta = value_dict_get(value_array_get(from.meta, IDX_CHILDREN), key),
    };
    return ret;
}

static int next_token(context_t *ctx, int dotisspecial);

/*
  Error reporting. Call when an error is detected. Always return -1.
*/
static int e_outofmemory(context_t *ctx, const char *fline)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "ERROR: out of memory (%s)", fline);
    return -1;
}

static int e_internal(context_t *ctx, const char *fline)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "internal error (%s)", fline);
    return -1;
}

static int e_syntax(context_t *ctx, int lineno, const char *msg)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: %s", lineno, msg);
    return -1;
}

static int e_badkey(context_t *ctx, int lineno)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: bad key", lineno);
    return -1;
}

static int e_keyexists(context_t *ctx, int lineno)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: key exists", lineno);
    return -1;
}

static int e_forbid(context_t *ctx, int lineno, const char *msg)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: %s", lineno, msg);
    return -1;
}

static char ptoml_get_kind(table_t tab)
{
    char kind = value_int32(value_array_get(tab.meta, IDX_KIND));
    return kind;
}

static void ptoml_set_kind(table_t tab, char kind)
{
    value_init_int32(value_array_get(tab.meta, IDX_KIND), kind);
}

static int ptoml_get_readonly(context_t *ctx)
{
    return 0;
}

static void ptoml_set_readonly(context_t *ctx, int readonly)
{
}

static void *expand(void *p, int sz, int newsz)
{
    void *s = MALLOC(newsz);
    if (!s)
        return 0;

    memcpy(s, p, sz);
    FREE(p);
    return s;
}

/*
 * Convert src to raw unescaped utf-8 string.
 * Returns NULL if error with errmsg in errbuf.
 */
static char *norm_basic_str(const char *src, int srclen, int multiline,
                            char *errbuf, int errbufsz)
{
    char *dst = 0; /* will write to dst[] and return it */
    int max = 0;   /* max size of dst[] */
    int off = 0;   /* cur offset in dst[] */
    const unsigned char *sp = (unsigned char *) src;
    const unsigned char *sq = sp + srclen;
    int ch;

    /* scan forward on src */
    for (;;)
    {
        if (off >= max - 10)
        { /* have some slack for misc stuff */
            int newmax = max + 50;
            char *x = expand(dst, max, newmax);
            if (!x)
            {
                xfree(dst);
                snprintf(errbuf, errbufsz, "out of memory");
                return 0;
            }
            dst = x;
            max = newmax;
        }

        /* finished? */
        if (sp >= sq)
            break;

        ch = *sp++;
        if (ch != '\\')
        {
            /* these chars must be escaped: U+0000 to U+0008, U+000A to U+001F, U+007F
             */
            if ((0 <= ch && ch <= 0x08) || (0x0a <= ch && ch <= 0x1f) ||
                (ch == 0x7f))
            {
                if (!(multiline && (ch == '\r' || ch == '\n')))
                {
                    xfree(dst);
                    snprintf(errbuf, errbufsz, "invalid char U+%04x", ch);
                    return 0;
                }
            }

            // a plain copy suffice
            dst[off++] = (char) ch;
            continue;
        }

        /* ch was backslash. we expect the escape char. */
        if (sp >= sq)
        {
            snprintf(errbuf, errbufsz, "last backslash is invalid");
            xfree(dst);
            return 0;
        }

        /* for multi-line, we want to kill line-ending-backslash ... */
        if (multiline)
        {

            // if there is only whitespace after the backslash ...
            if (sp[strspn((const char *) sp, " \t\r")] == '\n')
            {
                /* skip all the following whitespaces */
                sp += strspn((const char *) sp, " \t\r\n");
                continue;
            }
        }

        /* get the escaped char */
        ch = *sp++;
        switch (ch)
        {
            case 'u':
            case 'U':
            {
                int64_t ucs = 0;
                int nhex = (ch == 'u' ? 4 : 8);
                for (int i = 0; i < nhex; i++)
                {
                    if (sp >= sq)
                    {
                        snprintf(errbuf, errbufsz, "\\%c expects %d hex chars", ch, nhex);
                        xfree(dst);
                        return 0;
                    }
                    ch = *sp++;
                    int v = ('0' <= ch && ch <= '9')
                            ? ch - '0'
                            : (('A' <= ch && ch <= 'F') ? ch - 'A' + 10 : -1);
                    if (-1 == v)
                    {
                        snprintf(errbuf, errbufsz, "invalid hex chars for \\u or \\U");
                        xfree(dst);
                        return 0;
                    }
                    ucs = ucs * 16 + v;
                }
                int n = toml_ucs_to_utf8(ucs, (unsigned char *) &dst[off]);
                if (-1 == n)
                {
                    snprintf(errbuf, errbufsz, "illegal ucs code in \\u or \\U");
                    xfree(dst);
                    return 0;
                }
                off += n;
            }
                continue;

            case 'b':
                ch = '\b';
                break;
            case 't':
                ch = '\t';
                break;
            case 'n':
                ch = '\n';
                break;
            case 'f':
                ch = '\f';
                break;
            case 'r':
                ch = '\r';
                break;
            case '"':
                ch = '"';
                break;
            case '\\':
                ch = '\\';
                break;
            default:
                snprintf(errbuf, errbufsz, "illegal escape char \\%c", ch);
                xfree(dst);
                return 0;
        }

        dst[off++] = ch;
    }

    // Cap with NUL and return it.
    dst[off++] = 0;
    return dst;
}

/* Normalize a key. Convert all special chars to raw unescaped utf-8 chars. */
static char *normalize_key(context_t *ctx, token_t strtok)
{
    const char *sp = strtok.ptr;
    const char *sq = strtok.ptr + strtok.len;
    int lineno = strtok.lineno;
    char *ret;
    int ch = *sp;
    char ebuf[80];

    /* handle quoted string */
    if (ch == '\'' || ch == '\"')
    {
        /* if ''' or """, take 3 chars off front and back. Else, take 1 char off. */
        int multiline = 0;
        if (sp[1] == ch && sp[2] == ch)
        {
            sp += 3, sq -= 3;
            multiline = 1;
        }
        else
            sp++, sq--;

        if (ch == '\'')
        {
            /* for single quote, take it verbatim. */
            if (!(ret = STRNDUP(sp, sq - sp)))
            {
                e_outofmemory(ctx, FLINE);
                return 0;
            }
        }
        else
        {
            /* for double quote, we need to normalize */
            ret = norm_basic_str(sp, sq - sp, multiline, ebuf, sizeof(ebuf));
            if (!ret)
            {
                e_syntax(ctx, lineno, ebuf);
                return 0;
            }
        }

        /* newlines are not allowed in keys */
        if (strchr(ret, '\n'))
        {
            xfree(ret);
            e_badkey(ctx, lineno);
            return 0;
        }
        return ret;
    }

    /* for bare-key allow only this regex: [A-Za-z0-9_-]+ */
    const char *xp;
    for (xp = sp; xp != sq; xp++)
    {
        int k = *xp;
        if (isalnum(k))
            continue;
        if (k == '_' || k == '-')
            continue;
        e_badkey(ctx, lineno);
        return 0;
    }

    /* dup and return it */
    if (!(ret = STRNDUP(sp, sq - sp)))
    {
        e_outofmemory(ctx, FLINE);
        return 0;
    }
    return ret;
}

static int parse_keyval(context_t *ctx, table_t tab);

static int init_meta(VALUE *meta)
{
    if (value_init_array(meta))
    {
        return -1;
    }
    value_init_int32(value_array_insert(meta, IDX_KIND), 0);
    value_init_null(value_array_insert(meta, IDX_CHILDREN));
    value_init_bool(value_array_insert(meta, IDX_IMPLICIT), 0);
    return 0;
}

/* Create a table in the table.
 */
static table_t create_table_in_table(context_t *ctx, table_t tab, token_t keytok)
{
    /* first, normalize the key to be used for lookup.
     * remember to free it if we error out.
     */
    table_t ret = {0};
    char *newkey = normalize_key(ctx, keytok);
    if (!newkey)
    {
        return ret;
    }

    /* if key exists: error out */
    struct VALUE *dest = value_dict_get(tab.tab, newkey);
    struct VALUE *meta;
    if (dest)
    {
        meta = value_dict_get(value_array_get(tab.meta, IDX_CHILDREN), newkey);
        /* special case: if table exists, but was created implicitly ... */
        if (value_bool(value_array_get(meta, IDX_IMPLICIT)))
        {
            ret.meta = meta;
            ret.tab = dest;
            /* we make it explicit now, and simply return it. */
            value_init_bool(value_array_get(meta, IDX_IMPLICIT), 0);
            return ret;
        }
        xfree(newkey); /* don't need this anymore */
        e_keyexists(ctx, keytok.lineno);
        return ret;
    }

    /* create a new table entry */
    dest = value_dict_add(tab.tab, newkey);
    meta = value_dict_add(value_array_get(tab.meta, IDX_CHILDREN), newkey);
    if (!dest || !meta || init_meta(meta))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_dict(dest))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_dict(value_array_get(meta, IDX_CHILDREN)))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }

    ret.tab = dest;
    ret.meta = meta;
    return ret;
}

/* Create an array in the table.
 */
static table_t create_keyarray_in_table(context_t *ctx, table_t tab,
                                        token_t keytok, char kind)
{
    /* first, normalize the key to be used for lookup.
     * remember to free it if we error out.
     */
    table_t ret = {0};
    char *newkey = normalize_key(ctx, keytok);
    if (!newkey)
        return ret;

    /* if key exists: error out */
    if (value_dict_get(tab.tab, newkey))
    {
        xfree(newkey); /* don't need this anymore */
        e_keyexists(ctx, keytok.lineno);
        return ret;
    }

    /* make a new array entry */
    VALUE *dest = value_dict_add(tab.tab, newkey);
    VALUE *meta = value_dict_add(value_array_get(tab.meta, IDX_CHILDREN), newkey);
    if (!dest || !meta || init_meta(meta))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_array(dest))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_array(value_array_get(meta, IDX_CHILDREN)))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    value_init_int32(value_array_get(meta, IDX_KIND), kind);

    ret.tab = dest;
    ret.meta = meta;

    return ret;
}

static table_t create_value_in_table(context_t *ctx, table_t parent, token_t key)
{
    table_t ret = {0};
    VALUE *dest = value_dict_add_(parent.tab, key.ptr, key.len);
    VALUE *meta = value_dict_add_(value_array_get(parent.meta, IDX_CHILDREN), key.ptr, key.len);
    if (!dest || !meta || init_meta(meta))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    ret.tab = dest;
    ret.meta = meta;
    return ret;
}

static table_t create_value_in_array(context_t *ctx,
                                     table_t parent)
{
    table_t ret = {0};
    VALUE *dest = value_array_append(parent.tab);
    VALUE *meta = value_array_append(value_array_get(parent.meta, IDX_CHILDREN));
    if (!dest || !meta || init_meta(meta))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    ret.tab = dest;
    ret.meta = meta;
    return ret;
}

/* Create an array in an array
 */
static table_t create_array_in_array(context_t *ctx,
                                     table_t parent)
{
    table_t ret = {0};
    VALUE *dest = value_array_append(parent.tab);
    VALUE *meta = value_array_append(value_array_get(parent.meta, IDX_CHILDREN));
    if (!dest || !meta || init_meta(meta))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_array(dest))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_array(value_array_get(meta, IDX_CHILDREN)))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    ret.tab = dest;
    ret.meta = meta;
    return ret;
}

/* Create a table in an array
 */
static table_t create_table_in_array(context_t *ctx,
                                     table_t parent)
{
    table_t ret = {0};
    VALUE *dest = value_array_append(parent.tab);
    VALUE *meta = value_array_append(value_array_get(parent.meta, IDX_CHILDREN));
    if (!dest || !meta || init_meta(meta))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_dict(dest))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    if (value_init_dict(value_array_get(meta, IDX_CHILDREN)))
    {
        e_outofmemory(ctx, FLINE);
        return ret;
    }
    ret.tab = dest;
    ret.meta = meta;
    return ret;
}

static int skip_newlines(context_t *ctx, int isdotspecial)
{
    while (ctx->tok.tok == NEWLINE)
    {
        if (next_token(ctx, isdotspecial))
            return -1;
        if (ctx->tok.eof)
            break;
    }
    return 0;
}

static inline int eat_token(context_t *ctx, tokentype_t typ, int isdotspecial,
                            const char *fline)
{
    if (ctx->tok.tok != typ)
        return e_internal(ctx, fline);

    if (next_token(ctx, isdotspecial))
        return -1;

    return 0;
}

#include "toml_conv.c"

static int parse_simple_token(context_t *ctx, table_t dst, token_t token)
{
    char valtype = ptoml_valtype(token, dst.tab);

    value_init_int32(value_array_get(dst.meta, IDX_KIND), valtype);

    switch (valtype)
    {
        case 's':
            if (value_init_string_(dst.tab, token.ptr + 1, token.len - 2))
                return e_outofmemory(ctx, FLINE);
            return valtype;
        case 'b':
            return valtype;
        case 'i':
        case 'd':
            if (init_number(dst.tab, token.ptr, token.len))
                return e_syntax(ctx, ctx->tok.lineno, "Unexpected value");
            return valtype;
        case 'u':
            return e_syntax(ctx, ctx->tok.lineno, "Unexpected value");
        default:
            return e_internal(ctx, FLINE);
    }
}

/* We are at '{ ... }'.
 * Parse the table.
 */
static int parse_inline_table(context_t *ctx, table_t tab)
{
    if (eat_token(ctx, LBRACE, 1, FLINE))
        return -1;

    for (;;)
    {
        if (ctx->tok.tok == NEWLINE)
            return e_syntax(ctx, ctx->tok.lineno,
                            "newline not allowed in inline table");

        /* until } */
        if (ctx->tok.tok == RBRACE)
            break;

        if (ctx->tok.tok != STRING)
            return e_syntax(ctx, ctx->tok.lineno, "expect a string");

        if (parse_keyval(ctx, tab))
            return -1;

        if (ctx->tok.tok == NEWLINE)
            return e_syntax(ctx, ctx->tok.lineno,
                            "newline not allowed in inline table");

        /* on comma, continue to scan for next keyval */
        if (ctx->tok.tok == COMMA)
        {
            if (eat_token(ctx, COMMA, 1, FLINE))
                return -1;
            continue;
        }
        break;
    }

    if (eat_token(ctx, RBRACE, 1, FLINE))
        return -1;

    ptoml_set_readonly(ctx, true);

    return 0;
}

/* We are at '[...]' */
static int parse_array(context_t *ctx, table_t arr)
{
    if (eat_token(ctx, LBRACKET, 0, FLINE))
        return -1;

    for (;;)
    {
        if (skip_newlines(ctx, 0))
            return -1;

        /* until ] */
        if (ctx->tok.tok == RBRACKET)
            break;

        switch (ctx->tok.tok)
        {
            case STRING:
            {
                /* set array kind if this will be the first entry */
                char kind = ptoml_get_kind(arr);

                /* make a new value in array */
                table_t newval = create_value_in_array(ctx, arr);
                if (!newval.tab)
                    return e_outofmemory(ctx, FLINE);

                int valtype = parse_simple_token(ctx, newval, ctx->tok);
                if (valtype <= 0)
                    return valtype;

                /* set array type if this is the first entry */
                if (value_array_size(arr.tab) == 1)
                    ptoml_set_kind(arr, (char) valtype);
                else if (kind != valtype)
                    ptoml_set_kind(arr, 'm'); /* mixed */

                if (eat_token(ctx, STRING, 0, FLINE))
                    return -1;
                break;
            }

            case LBRACKET:
            { /* [ [array], [array] ... ] */
                /* set the array kind if this will be the first entry */
                char kind = ptoml_get_kind(arr);
                if (kind == 0)
                    ptoml_set_kind(arr, 'a');
                else if (kind != 'a')
                    ptoml_set_kind(arr, 'm');

                table_t subarr = create_array_in_array(ctx, arr);
                if (!subarr.tab)
                    return -1;
                if (parse_array(ctx, subarr))
                    return -1;
                break;
            }

            case LBRACE:
            { /* [ {table}, {table} ... ] */
                /* set the array kind if this will be the first entry */
                char kind = ptoml_get_kind(arr);
                if (kind == 0)
                    ptoml_set_kind(arr, '{');
                else if (kind != '{')
                    ptoml_set_kind(arr, 'm');


                table_t subtab = create_table_in_array(ctx, arr);
                if (!subtab.tab)
                    return -1;
                if (parse_inline_table(ctx, subtab))
                    return -1;
                break;
            }

            default:
                return e_syntax(ctx, ctx->tok.lineno, "syntax error");
        }

        if (skip_newlines(ctx, 0))
            return -1;

        /* on comma, continue to scan for next element */
        if (ctx->tok.tok == COMMA)
        {
            if (eat_token(ctx, COMMA, 0, FLINE))
                return -1;
            continue;
        }
        break;
    }

    if (eat_token(ctx, RBRACKET, 1, FLINE))
        return -1;
    return 0;
}

/* handle lines like these:
   key = "value"
   key = [ array ]
   key = { table }
*/
static int parse_keyval(context_t *ctx, table_t tab)
{
    if (ptoml_get_readonly(ctx))
    {
        return e_forbid(ctx, ctx->tok.lineno,
                        "cannot insert new entry into existing table");
    }

    token_t key = ctx->tok;
    if (eat_token(ctx, STRING, 1, FLINE))
        return -1;

    if (ctx->tok.tok == DOT)
    {
        /* handle inline dotted key.
           e.g.
           physical.color = "orange"
           physical.shape = "round"
        */
        struct table_t subtab;
        {
            char *subtabstr = normalize_key(ctx, key);
            if (!subtabstr)
                return -1;

            subtab = table_dict_get(tab, subtabstr);
            xfree(subtabstr);
        }
        if (!subtab.tab)
        {
            subtab = create_table_in_table(ctx, tab, key);
            if (!subtab.tab)
                return -1;
        }
        if (next_token(ctx, 1))
            return -1;
        if (parse_keyval(ctx, subtab))
            return -1;
        return 0;
    }

    if (ctx->tok.tok != EQUAL)
    {
        return e_syntax(ctx, ctx->tok.lineno, "missing =");
    }

    if (next_token(ctx, 0))
        return -1;

    switch (ctx->tok.tok)
    {
        case STRING:
        { /* key = "value" */

            table_t keyval = create_value_in_table(ctx, tab, key);
            if (!keyval.tab)
                return -1;
            token_t val = ctx->tok;

            if (parse_simple_token(ctx, keyval, val) < 0)
                return -1;

            if (next_token(ctx, 1))
                return -1;

            return 0;
        }

        case LBRACKET:
        { /* key = [ array ] */
            table_t arr = create_keyarray_in_table(ctx, tab, key, 0);
            if (parse_array(ctx, arr))
                return -1;
            return 0;
        }

        case LBRACE:
        { /* key = { table } */
            table_t nxttab = create_table_in_table(ctx, tab, key);
            if (!nxttab.tab)
                return -1;
            if (parse_inline_table(ctx, nxttab))
                return -1;
            return 0;
        }

        default:
            return e_syntax(ctx, ctx->tok.lineno, "syntax error");
    }
    return 0;
}

typedef struct tabpath_t tabpath_t;
struct tabpath_t
{
    int cnt;
    token_t key[10];
};

/* at [x.y.z] or [[x.y.z]]
 * Scan forward and fill tabpath until it enters ] or ]]
 * There will be at least one entry on return.
 */
static int fill_tabpath(context_t *ctx)
{
    int lineno = ctx->tok.lineno;
    int i;

    /* clear tpath */
    for (i = 0; i < ctx->tpath.top; i++)
    {
        char **p = &ctx->tpath.key[i];
        xfree(*p);
        *p = 0;
    }
    ctx->tpath.top = 0;

    for (;;)
    {
        if (ctx->tpath.top >= 10)
            return e_syntax(ctx, lineno,
                            "table path is too deep; max allowed is 10.");

        if (ctx->tok.tok != STRING)
            return e_syntax(ctx, lineno, "invalid or missing key");

        char *key = normalize_key(ctx, ctx->tok);
        if (!key)
            return -1;
        ctx->tpath.tok[ctx->tpath.top] = ctx->tok;
        ctx->tpath.key[ctx->tpath.top] = key;
        ctx->tpath.top++;

        if (next_token(ctx, 1))
            return -1;

        if (ctx->tok.tok == RBRACKET)
            break;

        if (ctx->tok.tok != DOT)
            return e_syntax(ctx, lineno, "invalid key");

        if (next_token(ctx, 1))
            return -1;
    }

    if (ctx->tpath.top <= 0)
        return e_syntax(ctx, lineno, "empty table selector");

    return 0;
}

/* Walk tabpath from the root, and create new tables on the way.
 * Sets ctx->curtab to the final table.
 */
static int walk_tabpath(context_t *ctx)
{
    /* start from root */
    table_t cur = {ctx->root, ctx->meta};

    VALUE *meta_item = ctx->meta;

    for (int i = 0; i < ctx->tpath.top; i++)
    {
        const char *key = ctx->tpath.key[i];

        VALUE *nexttab = 0;

        char kind = ptoml_get_kind(cur);

        switch (kind)
        {
            case 'r': // root table
            case '{':
                /* found a table. nexttab is where we will go next. */
                nexttab = value_dict_get(cur.tab, key);
                meta_item = value_dict_get(value_array_get(cur.meta, IDX_CHILDREN), key);
                break;

            case 'a':
                /* found an array. nexttab is the last table in the array. */
                if (kind != '{')
                {
                    return e_internal(ctx, FLINE);
                }

                size_t nitem = value_array_size(cur.tab);
                if (nitem == 0)
                {
                    return e_internal(ctx, FLINE);
                }

                nexttab = value_array_get(cur.tab, nitem - 1);
                meta_item = value_array_get(value_array_get(cur.meta, IDX_CHILDREN), nitem - 1);
                break;
            case 'v':
                return e_keyexists(ctx, ctx->tpath.tok[i].lineno);

            default:
            { /* Not found. Let's create an implicit table. */
                table_t next = create_table_in_table(ctx, cur, ctx->tpath.tok[i]);
                if (!next.tab)
                {
                    return e_outofmemory(ctx, FLINE);
                }

                /* tabs created by walk_tabpath are considered implicit */
                value_init_bool(value_array_get(next.meta, IDX_IMPLICIT), 1);
                nexttab = next.tab;
                meta_item = next.meta;
            }
                break;
        }

        /* switch to next tab */
        cur.tab = nexttab;
        cur.meta = meta_item;
    }
    ctx->curtab = cur;

    return 0;
}

/* handle lines like [x.y.z] or [[x.y.z]] */
static int parse_select(context_t *ctx)
{
    assert(ctx->tok.tok == LBRACKET);

    /* true if [[ */
    int llb = (ctx->tok.ptr + 1 < ctx->stop && ctx->tok.ptr[1] == '[');
    /* need to detect '[[' on our own because next_token() will skip whitespace,
       and '[ [' would be taken as '[[', which is wrong. */

    /* eat [ or [[ */
    if (eat_token(ctx, LBRACKET, 1, FLINE))
        return -1;
    if (llb)
    {
        assert(ctx->tok.tok == LBRACKET);
        if (eat_token(ctx, LBRACKET, 1, FLINE))
            return -1;
    }

    if (fill_tabpath(ctx))
        return -1;

    /* For [x.y.z] or [[x.y.z]], remove z from tpath.
     */
    token_t z = ctx->tpath.tok[ctx->tpath.top - 1];
    xfree(ctx->tpath.key[ctx->tpath.top - 1]);
    ctx->tpath.top--;

    /* set up ctx->curtab to parent */
    if (walk_tabpath(ctx))
    {
        return -1;
    }

    if (!llb)
    {
        /* [x.y.z] -> create z = {} in x.y */
        table_t curtab = create_table_in_table(ctx, ctx->curtab, z);
        if (!curtab.tab)
            return -1;
        ctx->curtab = curtab;
    }
    else
    {
        /* [[x.y.z]] -> create z = [] in x.y */
        table_t arr;
        {
            char *zstr = normalize_key(ctx, z);
            if (!zstr)
                return -1;
            arr = table_dict_get(ctx->curtab, zstr);
            xfree(zstr);
        }
        if (!arr.tab)
        {
            arr = create_keyarray_in_table(ctx, ctx->curtab, z, '{');
            if (!arr.tab)
                return -1;
        }
        if (ptoml_get_kind(arr) != '{')
            return e_syntax(ctx, z.lineno, "array mismatch");

        /* add to z[] */
        table_t dest;
        {
            table_t t = create_table_in_array(ctx, arr);
            if (!t.tab)
                return -1;

            // Sim
            //if (0 == (t->key = STRDUP("__anon__")))
            //    return e_outofmemory(ctx, FLINE);

            dest = t;
        }

        ctx->curtab = dest;
    }

    if (ctx->tok.tok != RBRACKET)
    {
        return e_syntax(ctx, ctx->tok.lineno, "expects ]");
    }
    if (llb)
    {
        if (!(ctx->tok.ptr + 1 < ctx->stop && ctx->tok.ptr[1] == ']'))
        {
            return e_syntax(ctx, ctx->tok.lineno, "expects ]]");
        }
        if (eat_token(ctx, RBRACKET, 1, FLINE))
            return -1;
    }

    if (eat_token(ctx, RBRACKET, 1, FLINE))
        return -1;

    if (ctx->tok.tok != NEWLINE)
        return e_syntax(ctx, ctx->tok.lineno, "extra chars after ] or ]]");

    return 0;
}

int toml_parse(char *conf, char *errbuf, size_t errbufsz, VALUE *root)
{
    return toml_parse_len(conf, strlen(conf), errbuf, errbufsz, root);
}

int toml_parse_len(char *conf, size_t len, char *errbuf, size_t errbufsz, VALUE *root)
{
    context_t ctx;
    VALUE meta;

    // clear errbuf
    if (errbufsz <= 0)
        errbufsz = 0;
    if (errbufsz > 0)
        errbuf[0] = 0;

    // init context
    memset(&ctx, 0, sizeof(ctx));
    ctx.start = conf;
    ctx.stop = ctx.start + len;
    ctx.errbuf = errbuf;
    ctx.errbufsz = errbufsz;

    // start with an artificial newline of length 0
    ctx.tok.tok = NEWLINE;
    ctx.tok.lineno = 1;
    ctx.tok.ptr = conf;
    ctx.tok.len = 0;
    ctx.root = root;
    ctx.meta = &meta;

    // create metadata dict
    value_init_array(&meta);
    value_init_int32(value_array_insert(&meta, IDX_KIND), 'r');
    value_init_dict(value_array_insert(&meta, IDX_CHILDREN));
    value_init_bool(value_array_insert(&meta, IDX_IMPLICIT), 0);

    // make a root table
    value_init_dict(root);

    // set root as default table
    {
        table_t cur = {ctx.root, ctx.meta};
        ctx.curtab = cur;
    }

    /* Scan forward until EOF */
    for (token_t tok = ctx.tok; !tok.eof; tok = ctx.tok)
    {
        switch (tok.tok)
        {

            case NEWLINE:
                if (next_token(&ctx, 1))
                    goto fail;
                break;

            case STRING:
                if (parse_keyval(&ctx, ctx.curtab))
                    goto fail;

                if (ctx.tok.tok != NEWLINE)
                {
                    e_syntax(&ctx, ctx.tok.lineno, "extra chars after value");
                    goto fail;
                }

                if (eat_token(&ctx, NEWLINE, 1, FLINE))
                    goto fail;
                break;

            case LBRACKET: /* [ x.y.z ] or [[ x.y.z ]] */
                if (parse_select(&ctx))
                    goto fail;
                break;

            default:
                e_syntax(&ctx, tok.lineno, "syntax error");
                goto fail;
        }
    }

    /* success */
    for (int i = 0; i < ctx.tpath.top; i++)
        xfree(ctx.tpath.key[i]);
    toml_free(ctx.meta);
    return 0;

    fail:
    // Something bad has happened. Free resources and return error.
    for (int i = 0; i < ctx.tpath.top; i++)
        xfree(ctx.tpath.key[i]);
    toml_free(ctx.root);
    toml_free(ctx.meta);
    return 1;
}

static void set_token(context_t *ctx, tokentype_t tok, int lineno, char *ptr,
                      int len)
{
    token_t t;
    t.tok = tok;
    t.lineno = lineno;
    t.ptr = ptr;
    t.len = len;
    t.eof = 0;
    ctx->tok = t;
}

static void set_eof(context_t *ctx, int lineno)
{
    set_token(ctx, NEWLINE, lineno, ctx->stop, 0);
    ctx->tok.eof = 1;
}

static int scan_string(context_t *ctx, char *p, int lineno, int dotisspecial)
{
    char *orig = p;
    if (0 == strncmp(p, "'''", 3))
    {
        char *q = p + 3;

        while (1)
        {
            q = strstr(q, "'''");
            if (0 == q)
            {
                return e_syntax(ctx, lineno, "unterminated triple-s-quote");
            }
            while (q[3] == '\'')
                q++;
            break;
        }

        set_token(ctx, STRING, lineno, orig, q + 3 - orig);
        return 0;
    }

    if (0 == strncmp(p, "\"\"\"", 3))
    {
        char *q = p + 3;

        while (1)
        {
            q = strstr(q, "\"\"\"");
            if (0 == q)
            {
                return e_syntax(ctx, lineno, "unterminated triple-d-quote");
            }
            if (q[-1] == '\\')
            {
                q++;
                continue;
            }
            while (q[3] == '\"')
                q++;
            break;
        }

        // the string is [p+3, q-1]

        int hexreq = 0; /* #hex required */
        int escape = 0;
        for (p += 3; p < q; p++)
        {
            if (escape)
            {
                escape = 0;
                if (strchr("btnfr\"\\", *p))
                    continue;
                if (*p == 'u')
                {
                    hexreq = 4;
                    continue;
                }
                if (*p == 'U')
                {
                    hexreq = 8;
                    continue;
                }
                if (p[strspn(p, " \t\r")] == '\n')
                    continue; /* allow for line ending backslash */
                return e_syntax(ctx, lineno, "bad escape char");
            }
            if (hexreq)
            {
                hexreq--;
                if (strchr("0123456789ABCDEF", *p))
                    continue;
                return e_syntax(ctx, lineno, "expect hex char");
            }
            if (*p == '\\')
            {
                escape = 1;
                continue;
            }
        }
        if (escape)
            return e_syntax(ctx, lineno, "expect an escape char");
        if (hexreq)
            return e_syntax(ctx, lineno, "expected more hex char");

        set_token(ctx, STRING, lineno, orig, q + 3 - orig);
        return 0;
    }

    if ('\'' == *p)
    {
        for (p++; *p && *p != '\n' && *p != '\''; p++);
        if (*p != '\'')
        {
            return e_syntax(ctx, lineno, "unterminated s-quote");
        }

        set_token(ctx, STRING, lineno, orig, p + 1 - orig);
        return 0;
    }

    if ('\"' == *p)
    {
        int hexreq = 0; /* #hex required */
        int escape = 0;
        for (p++; *p; p++)
        {
            if (escape)
            {
                escape = 0;
                if (strchr("btnfr\"\\", *p))
                    continue;
                if (*p == 'u')
                {
                    hexreq = 4;
                    continue;
                }
                if (*p == 'U')
                {
                    hexreq = 8;
                    continue;
                }
                return e_syntax(ctx, lineno, "bad escape char");
            }
            if (hexreq)
            {
                hexreq--;
                if (strchr("0123456789ABCDEF", *p))
                    continue;
                return e_syntax(ctx, lineno, "expect hex char");
            }
            if (*p == '\\')
            {
                escape = 1;
                continue;
            }
            if (*p == '\'')
            {
                if (p[1] == '\'' && p[2] == '\'')
                {
                    return e_syntax(ctx, lineno, "triple-s-quote inside string lit");
                }
                continue;
            }
            if (*p == '\n')
                break;
            if (*p == '"')
                break;
        }
        if (*p != '"')
        {
            return e_syntax(ctx, lineno, "unterminated quote");
        }

        set_token(ctx, STRING, lineno, orig, p + 1 - orig);
        return 0;
    }

    /* check for timestamp without quotes */
    if (0 == scan_date(p, 0, 0, 0) || 0 == scan_time(p, 0, 0, 0))
    {
        // forward thru the timestamp
        p += strspn(p, "0123456789.:+-Tt Zz");
        // squeeze out any spaces at end of string
        for (; p[-1] == ' '; p--);
        // tokenize
        set_token(ctx, STRING, lineno, orig, p - orig);
        return 0;
    }

    /* literals */
    for (; *p && *p != '\n'; p++)
    {
        int ch = *p;
        if (ch == '.' && dotisspecial)
            break;
        if ('A' <= ch && ch <= 'Z')
            continue;
        if ('a' <= ch && ch <= 'z')
            continue;
        if (strchr("0123456789+-_.", ch))
            continue;
        break;
    }

    set_token(ctx, STRING, lineno, orig, p - orig);
    return 0;
}

static int next_token(context_t *ctx, int dotisspecial)
{
    int lineno = ctx->tok.lineno;
    char *p = ctx->tok.ptr;
    int i;

    /* eat this tok */
    for (i = 0; i < ctx->tok.len; i++)
    {
        if (*p++ == '\n')
            lineno++;
    }

    /* make next tok */
    while (p < ctx->stop)
    {
        /* skip comment. stop just before the \n. */
        if (*p == '#')
        {
            for (p++; p < ctx->stop && *p != '\n'; p++);
            continue;
        }

        if (dotisspecial && *p == '.')
        {
            set_token(ctx, DOT, lineno, p, 1);
            return 0;
        }

        switch (*p)
        {
            case ',':
                set_token(ctx, COMMA, lineno, p, 1);
                return 0;
            case '=':
                set_token(ctx, EQUAL, lineno, p, 1);
                return 0;
            case '{':
                set_token(ctx, LBRACE, lineno, p, 1);
                return 0;
            case '}':
                set_token(ctx, RBRACE, lineno, p, 1);
                return 0;
            case '[':
                set_token(ctx, LBRACKET, lineno, p, 1);
                return 0;
            case ']':
                set_token(ctx, RBRACKET, lineno, p, 1);
                return 0;
            case '\n':
                set_token(ctx, NEWLINE, lineno, p, 1);
                return 0;
            case '\r':
            case ' ':
            case '\t':
                /* ignore white spaces */
                p++;
                continue;
        }

        return scan_string(ctx, p, lineno, dotisspecial);
    }

    set_eof(ctx, lineno);
    return 0;
}
