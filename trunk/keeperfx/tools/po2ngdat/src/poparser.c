/*
 * poparser.c
 *
 *  Created on: 30-09-2012
 *      Author: tomasz
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#include "poparser.h"

FILE * open_catalog_file (const char *input_name)
{
  static const char *extension[] = { "", ".po", ".pot", };
  char *file_name;
  FILE *ret_val;
  size_t k;

  for (k = 0; k < sizeof(extension)/sizeof(extension[0]); ++k)
  {
      file_name = (char *) malloc (strlen(input_name) + strlen(extension[k]) + 1);
      strcpy(file_name, input_name); strcat(file_name, extension[k]);

      ret_val = fopen (file_name, "r");
      if (ret_val != NULL || errno != ENOENT)
        {
          free (file_name);
          return ret_val;
        }

      free (file_name);
  }
  /* File does not exist.  */
  errno = ENOENT;
  return NULL;
}

pofile_catalog_reader_ty * pofile_catalog_reader_alloc(void)
{
    pofile_catalog_reader_ty *pop;

    pop = (pofile_catalog_reader_ty *) malloc(sizeof(pofile_catalog_reader_ty));

    size_t i;

    pop->domain = MESSAGE_DOMAIN_DEFAULT;
    pop->comment_previd = NULL;
    pop->comment_del = NULL;
    pop->nmsg.comment_dot = NULL;
    pop->nmsg.comment = NULL;
    pop->nmsg.filepos_count = 0;
    pop->nmsg.filepos = NULL;
    for (i = 0; i < NFORMATS; i++)
      pop->nmsg.is_format[i] = undecided;
    pop->nmsg.is_fuzzy = false;
    pop->nmsg.range.min = -1;
    pop->nmsg.range.max = -1;
    pop->nmsg.do_wrap = undecided;

    return pop;
}

void pofile_catalog_reader_free (pofile_catalog_reader_ty *pop)
{
  free (pop);
}

/* Add the accumulated comments to the message.  */
static void
pofile_copy_comment_state (pofile_catalog_reader_ty *this, message_ty *mp)
{
  size_t j, i;

    {
      if (this->nmsg.comment != NULL)
        for (j = 0; j < this->nmsg.comment->nitems; ++j)
          message_comment_append (mp, this->nmsg.comment->item[j]);
      if (this->nmsg.comment_dot != NULL)
        for (j = 0; j < this->nmsg.comment_dot->nitems; ++j)
          message_comment_dot_append (mp, this->nmsg.comment_dot->item[j]);
    }
    {
      for (j = 0; j < this->nmsg.filepos_count; ++j)
        {
          lex_pos_ty *pp;

          pp = &this->nmsg.filepos[j];
          message_comment_filepos (mp, pp->file_name, pp->line_number);
        }
    }
  mp->is_fuzzy = this->nmsg.is_fuzzy;
  for (i = 0; i < NFORMATS; i++)
    mp->is_format[i] = this->nmsg.is_format[i];
  mp->range = this->nmsg.range;
  mp->do_wrap = this->nmsg.do_wrap;
}

static void
pofile_reset_comment_state (pofile_catalog_reader_ty *this)
{
  size_t j, i;

    {
      if (this->nmsg.comment != NULL)
        {
          string_list_free (this->nmsg.comment);
          this->nmsg.comment = NULL;
        }
      if (this->nmsg.comment_dot != NULL)
        {
          string_list_free (this->nmsg.comment_dot);
          this->nmsg.comment_dot = NULL;
        }
      if (this->comment_previd != NULL)
        {
          string_list_free (this->comment_previd);
          this->comment_previd = NULL;
        }
      if (this->comment_del != NULL)
        {
          string_list_free (this->comment_del);
          this->comment_del = NULL;
        }
    }
    {
      for (j = 0; j < this->nmsg.filepos_count; ++j)
        free (this->nmsg.filepos[j].file_name);
      if (this->nmsg.filepos != NULL)
        free (this->nmsg.filepos);
      this->nmsg.filepos_count = 0;
      this->nmsg.filepos = NULL;
    }
  this->nmsg.is_fuzzy = false;
  for (i = 0; i < NFORMATS; i++)
    this->nmsg.is_format[i] = undecided;
  this->nmsg.range.min = -1;
  this->nmsg.range.max = -1;
  this->nmsg.do_wrap = undecided;

  this->nmsg.msgctxt = NULL;
  this->nmsg.msgid = NULL;
  this->nmsg.msgid_plural = NULL;
  this->nmsg.prev_msgctxt = NULL;
  this->nmsg.prev_msgid = NULL;
  this->nmsg.prev_msgid_plural = NULL;
  this->nmsg.obsolete = 0;
}

void
pofile_add_message (pofile_catalog_reader_ty *this,
                     char *msgctxt,
                     char *msgid,
                     const lex_pos_ty *msgid_pos,
                     char *msgid_plural,
                     char *msgstr, size_t msgstr_len,
                     const lex_pos_ty *msgstr_pos,
                     char *prev_msgctxt,
                     char *prev_msgid,
                     char *prev_msgid_plural,
                     bool force_fuzzy, bool obsolete)
{
  message_ty *mp;

  if (this->mdlp != NULL)
    /* Select the appropriate sublist of this->mdlp.  */
    this->mlp = msgdomain_list_sublist (this->mdlp, this->domain, true);

  /* See if this message ID has been seen before.  */
  mp = message_list_search (this->mlp, msgctxt, msgid);

  if (mp)
    {
      if (!(msgstr_len == mp->msgstr_len
            && memcmp (msgstr, mp->msgstr, msgstr_len) == 0))
        {
          /* We give a fatal error about this, regardless whether the
             translations are equal or different.*/
          printf("Duplicate message definition at line %d of file '%s'",msgid_pos->line_number,msgid_pos->file_name);
        }
      /* We don't need the just constructed entries' parameter string
         (allocated in po-gram-gen.y).  */
      free (msgid);
      if (msgid_plural != NULL)
        free (msgid_plural);
      free (msgstr);
      if (msgctxt != NULL)
        free (msgctxt);
      if (prev_msgctxt != NULL)
        free (prev_msgctxt);
      if (prev_msgid != NULL)
        free (prev_msgid);
      if (prev_msgid_plural != NULL)
        free (prev_msgid_plural);

      /* Add the accumulated comments to the message.  */
      pofile_copy_comment_state (this, mp);
    }
  else
    {
      /* Construct message to add to the list.
         Obsolete message go into the list at least for duplicate checking.
         It's the caller's responsibility to ignore obsolete messages when
         appropriate.  */
      mp = message_alloc (msgctxt, msgid, msgid_plural, msgstr, msgstr_len,
                          msgstr_pos);
      mp->prev_msgctxt = prev_msgctxt;
      mp->prev_msgid = prev_msgid;
      mp->prev_msgid_plural = prev_msgid_plural;
      mp->obsolete = obsolete;
      pofile_copy_comment_state (this, mp);
      if (force_fuzzy)
        mp->is_fuzzy = true;

      message_list_append (this->mlp, mp);
    }
}

/* Process ['msgctxt'/]'msgid'/'msgstr' pair from .po file.  */
void
pofile_directive_message (pofile_catalog_reader_ty *this, const lex_pos_ty * curfpos)
{
  pofile_add_message (this, this->nmsg.msgctxt, this->nmsg.msgid, curfpos, this->nmsg.msgid_plural,
      this->nmsg.msgstr, this->nmsg.msgstr_len, curfpos,
      this->nmsg.prev_msgctxt, this->nmsg.prev_msgid, this->nmsg.prev_msgid_plural,
      this->nmsg.is_fuzzy, this->nmsg.obsolete);

  /* Prepare for next message.  */
  pofile_reset_comment_state (this);
}

/* Parse a special comment and put the result in *fuzzyp, formatp, *rangep,
   *wrapp.  */
void
po_parse_comment_special (const char *s,
                          bool *fuzzyp, enum is_format formatp[NFORMATS],
                          struct argument_range *rangep, enum is_wrap *wrapp)
{
  size_t i;

  *fuzzyp = false;
  for (i = 0; i < NFORMATS; i++)
    formatp[i] = undecided;
  rangep->min = -1;
  rangep->max = -1;
  *wrapp = undecided;

  while (*s != '\0')
    {
      const char *t;

      /* Skip whitespace.  */
      while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) != NULL)
        s++;

      /* Collect a token.  */
      t = s;
      while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) == NULL)
        s++;
      if (s != t)
        {
          size_t len = s - t;

          /* Accept fuzzy flag.  */
          if (len == 5 && memcmp (t, "fuzzy", 5) == 0)
            {
              *fuzzyp = true;
              continue;
            }

          /* Accept format description.  */
          if (len >= 7 && memcmp (t + len - 7, "-format", 7) == 0)
            {
              const char *p;
              size_t n;
              enum is_format value;

              p = t;
              n = len - 7;

              if (n >= 3 && memcmp (p, "no-", 3) == 0)
                {
                  p += 3;
                  n -= 3;
                  value = no;
                }
              else if (n >= 9 && memcmp (p, "possible-", 9) == 0)
                {
                  p += 9;
                  n -= 9;
                  value = possible;
                }
              else if (n >= 11 && memcmp (p, "impossible-", 11) == 0)
                {
                  p += 11;
                  n -= 11;
                  value = impossible;
                }
              else
                value = yes;

              for (i = 0; i < NFORMATS; i++)
                if (strlen (format_language[i]) == n
                    && memcmp (format_language[i], p, n) == 0)
                  {
                    formatp[i] = value;
                    break;
                  }
              if (i < NFORMATS)
                continue;
            }

          /* Accept range description "range: <min>..<max>".  */
          if (len == 6 && memcmp (t, "range:", 6) == 0)
            {
              /* Skip whitespace.  */
              while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) != NULL)
                s++;

              /* Collect a token.  */
              t = s;
              while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) == NULL)
                s++;
              /* Parse it.  */
              if (*t >= '0' && *t <= '9')
                {
                  unsigned int min = 0;

                  for (; *t >= '0' && *t <= '9'; t++)
                    {
                      if (min <= INT_MAX / 10)
                        {
                          min = 10 * min + (*t - '0');
                          if (min > INT_MAX)
                            min = INT_MAX;
                        }
                      else
                        /* Avoid integer overflow.  */
                        min = INT_MAX;
                    }
                  if (*t++ == '.')
                    if (*t++ == '.')
                      if (*t >= '0' && *t <= '9')
                        {
                          unsigned int max = 0;
                          for (; *t >= '0' && *t <= '9'; t++)
                            {
                              if (max <= INT_MAX / 10)
                                {
                                  max = 10 * max + (*t - '0');
                                  if (max > INT_MAX)
                                    max = INT_MAX;
                                }
                              else
                                /* Avoid integer overflow.  */
                                max = INT_MAX;
                            }
                          if (min <= max)
                            {
                              rangep->min = min;
                              rangep->max = max;
                              continue;
                            }
                        }
                }
            }

          /* Accept wrap description.  */
          if (len == 4 && memcmp (t, "wrap", 4) == 0)
            {
              *wrapp = yes;
              continue;
            }
          if (len == 7 && memcmp (t, "no-wrap", 7) == 0)
            {
              *wrapp = no;
              continue;
            }

          /* Unknown special comment marker.  It may have been generated
             from a future xgettext version.  Ignore it.  */
        }
    }
}

void
pofile_flags (message_ty *this, const char *s)
{
    po_parse_comment_special (s, &this->is_fuzzy, this->is_format, &this->range,
                              &this->do_wrap);
}

void
pofile_comment_dot (message_ty *this, const char *s)
{
    if (this->comment_dot == NULL)
      this->comment_dot = string_list_alloc ();
    string_list_append (this->comment_dot, s);
}

void
pofile_comment_filepos (message_ty *this,
                         const char *name, size_t line)
{
    size_t nbytes;
    lex_pos_ty *pp;

    nbytes = (this->filepos_count + 1) * sizeof (this->filepos[0]);
    this->filepos = realloc(this->filepos, nbytes);
    pp = &this->filepos[this->filepos_count++];
    pp->file_name = strdup(name);
    pp->line_number = line;
}

void
pofile_comment_previd (pofile_catalog_reader_ty *this, const char *s)
{
    if (this->comment_previd == NULL)
      this->comment_previd = string_list_alloc ();
    string_list_append (this->comment_previd, s);
}

void
pofile_comment_del (pofile_catalog_reader_ty *this, const char *s)
{
    if (this->comment_del == NULL)
      this->comment_del = string_list_alloc ();
    string_list_append (this->comment_del, s);
}

void
pofile_comment_ord (message_ty *this, const char *s)
{
    if (this->comment == NULL)
      this->comment = string_list_alloc ();
    string_list_append (this->comment, s);
}

/* Process 'domain' directive from .po file.  */
void
pofile_directive_domain (pofile_catalog_reader_ty *this, const char *name)
{
    this->domain = strdup(name);
    this->mlp = message_list_alloc(true);
    this->kw = COMMENT;

  /* If there are accumulated comments, throw them away, they are
     probably part of the file header, or about the domain directive,
     and will be unrelated to the next message.  */
    pofile_reset_comment_state (this);
}



short pofile_catalog_reader_parse_line(pofile_catalog_reader_ty *pop, const char *line, const lex_pos_ty * curfpos)
{
    const char * lptr;
    char * eptr;
    char valbuf[MAX_MESSAGE_SIZE+1];
    bool has_string;

    has_string = false;
    printf("Parsing line %d of file '%s'.\n",curfpos->line_number,curfpos->file_name);
    lptr = line;

    while (isspace(*lptr)) lptr++;

    if ((pop->kw == MSGSTR) && (strncmp(lptr,"\"",1) != 0))
    {
        pofile_directive_message(pop,curfpos);
        pop->kw = COMMENT;
    }

    if (strncmp(lptr,"#,",2) == 0)
    { // flags
        lptr += 2;
        while (isspace(*lptr)) lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        // Add to catalog
        if (*valbuf != '\0') {
            pofile_flags(&pop->nmsg, valbuf);
        }
    } else
    if (strncmp(lptr,"#.",2) == 0)
    { // auto comments
        lptr += 2;
        if (isspace(*lptr)) lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        // Add to catalog
        if (*valbuf != '\0') {
            pofile_comment_dot(&pop->nmsg, valbuf);
        }
    } else
    if (strncmp(lptr,"#:",2) == 0)
    { // references
        lptr += 2;
        while (isspace(*lptr)) lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        int num = 0;
        eptr = strrchr(valbuf,':');
        if (eptr > valbuf) {
            num = strtol(eptr,NULL,10);
            *eptr = '\0';
        }
        // Add to catalog
        if (*valbuf != '\0') {
            pofile_comment_filepos(&pop->nmsg, valbuf, num);
        }
    } else
    if (strncmp(lptr,"#|",2) == 0)
    { // previous msgid value
        lptr += 2;
        while (isspace(*lptr)) lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        // Add to catalog
        if (*valbuf != '\0') {
            pofile_comment_previd(pop, valbuf);
        }
    } else
    if (strncmp(lptr,"#~",2) == 0)
    { // deleted lines
        lptr += 2;
        while (isspace(*lptr)) lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        // Add to catalog
        if (*valbuf != '\0') {
            pofile_comment_del(pop, valbuf);
        }
    } else
    if (strncmp(lptr,"#",1) == 0)
    { // ordinary comment
        lptr += 2;
        if (isspace(*lptr)) lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        // Add to catalog
        if (*valbuf != '\0') {
            pofile_comment_ord(&pop->nmsg, valbuf);
        }
    } else
    if (strncmp(lptr,"msgctxt ",8) == 0)
    { // Message context description
        pop->kw = MSGCTXT;
        has_string = true;
    } else
    if (strncmp(lptr,"msgid ",6) == 0)
    { // message id (original language text)
        pop->kw = MSGID;
        has_string = true;
    } else
    if (strncmp(lptr,"msgid_plural ",13) == 0)
    { // plural
        pop->kw = MSGID_PLURAL;
        has_string = true;
    } else
    if (strncmp(lptr,"msgstr ",7) == 0)
    { // msgstr - translation
        pop->kw = MSGSTR;
        has_string = true;
    } else
    if (strncmp(lptr,"msgstr[",7) == 0)
    { // msgstr[i] - plural translation
        pop->kw = MSGSTR;
        has_string = true;
    } else
    if (strncmp(lptr,"\"",1) == 0)
    { // continuation of string
        has_string = true;
    } else
    {
        // Allow empty lines
        if (*lptr == '\0')
            return ERR_OK;
        //TODO better error message would be nice
        printf("Unrecognized line\n");
        return ERR_BAD_FORMAT;
    }
    if (has_string)
    {
        lptr += 2;
        while (isspace(*lptr)) lptr++;
        if (*lptr == '"') lptr++;
        strcpy(valbuf,lptr);
        // Trim the end of the comment
        eptr = valbuf + strlen(valbuf)-1;
        while (isspace(*eptr) && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        if ((*eptr == '"') && (eptr >= valbuf)) { *eptr='\0'; eptr--; }
        // Add to catalog
        if (*valbuf != '\0') {
            switch (pop->kw) {
            case MSGCTXT:
                pop->nmsg.msgctxt = strdup(valbuf);
                break;
            case MSGID:
                pop->nmsg.msgid = strdup(valbuf);
                break;
            case MSGID_PLURAL:
                return ERR_BAD_FORMAT; // not supported
                break;
            case MSGSTR:
                pop->nmsg.msgstr = strdup(valbuf);
                break;
            case PREV_MSGCTXT:
                pop->nmsg.prev_msgctxt = strdup(valbuf);
                break;
            case PREV_MSGID:
                pop->nmsg.prev_msgid = strdup(valbuf);
                break;
            case PREV_MSGID_PLURAL:
                return ERR_BAD_FORMAT; // not supported
                break;
            default:
                break;
            }
        }
    }
    return ERR_OK;
}

short pofile_catalog_reader_parse(pofile_catalog_reader_ty *pop, FILE *fp, const char *filename)
{
  /* Parse the stream's content.  */
    lex_pos_ty curfpos;
    char iline[MAX_MESSAGE_SIZE+1];
    curfpos.file_name = (char *)filename;
    curfpos.line_number = 1;
    pofile_directive_domain (pop, "KeeperFX");
    while(fgets(iline, MAX_MESSAGE_SIZE, fp) != NULL)
    {
        iline[MAX_MESSAGE_SIZE]='\0';
        short errcode;
        errcode = pofile_catalog_reader_parse_line(pop, iline, &curfpos);
        if (errcode != ERR_OK) {
            printf("Error was detected while parsing line %d of file '%s'.\n",curfpos.line_number,curfpos.file_name);
            return errcode;
        }
        curfpos.line_number++;
    }
    return ERR_OK;
}
