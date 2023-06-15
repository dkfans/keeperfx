/*
 * CentiTOML
 * <http://github.com/SimLV/centitoml>
 *
 * Copyright (c) 2022 CK Tan, TheSim

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

#ifndef CENTITOML_TOML_H
#define CENTITOML_TOML_H

#include "value.h"

void toml_set_memutil(void *(*xxmalloc)(size_t), void (*xxfree)(void *));

/* Parse a string
 * Return a table on success, or 0 otherwise.
 * Caller must value_fini(root) after use.
 */
int toml_parse(char *conf, char *errbuf, size_t errbufsz, VALUE *root);

/* Parse a string with len
 * Return a table on success, or 0 otherwise.
 * Caller must value_fini(root) after use.
 */
int toml_parse_len(char *conf, size_t len, char *errbuf, size_t errbufsz, VALUE *root);

#endif //CENTITOML_TOML_H
