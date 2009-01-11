#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_EXPORT_NAMELEN 255
#define MAX_EXPORTS 0xc00
#define MAX_FNAMES_SIZE 0x87C8
 
#define EXP_SEC_HDROFS 0x00163400
#define ARR_POS_FUNC   0x00163428
#define ARR_POS_FNAMES 0x00167c28
#define ARR_POS_FORDIN 0x00166428
#define ARR_POS_NMSTRS 0x0016ac38

#define ARR_RAW_TO_RVA 0x00251C00

#define SECTIONS_NUM 10
unsigned long sections_va[] = {0x0,0x00001000,0x00100000,0x00103000,
                      0x0037E000,0x00380000,0x00398000,0x0039B000,
                      0x0039C000,0x003B5000,0x00ff0000,0x00ff0000};

#define FUNC_HEADSTR "_DK_"
#define END_STR   "Blessed are those who have not seen and yet believe"

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

struct export_entry {
       unsigned short seg;
       unsigned long offs;
       unsigned long nmoffs;
       char srcname[MAX_EXPORT_NAMELEN+1];
       char dstname[MAX_EXPORT_NAMELEN+1];
       };


void export_sort(struct export_entry *exp[],unsigned int exp_size)
{
   int sorted=false;
   unsigned int i;
   struct export_entry *pTemp;
   /* Sort the strings in ascending order */
   while(!sorted)
   {
     sorted = true;
     for (i = 0 ; i < exp_size-1 ; i++)
       if(strcmp(exp[i]->dstname, exp[i+1]->dstname) > 0)
       {
         sorted = false;     /* We were out of order */
         pTemp= exp[i];       /* Swap pointers exp[i]  */
         exp[i] = exp[i+1];  /*       and            */
         exp[i+1]  = pTemp; /*     exp[i + 1]        */
       } 
   }
}

int find_dupename(struct export_entry *exp[],unsigned int exp_size)
{
   int i;
   for (i = 0 ; i < exp_size-1 ; i++)
       if(strcasecmp(exp[i]->dstname, exp[i+1]->dstname) == 0)
       {
         return i;
       } 
   return -1;
}

int find_dupeoffs(struct export_entry *exp[],unsigned int exp_size)
{
   int i,k;
   for (i = 0 ; i < exp_size ; i++)
     for (k = 0 ; k < exp_size ; k++)
       if ((i!=k) && (exp[i]->seg==exp[k]->seg) && (exp[i]->offs==exp[k]->offs))
       {
         return i;
       } 
   return -1;
}

char *get_name_with_prefix(char *dname,char *sname)
{
  int name_begin;
  if (strlen(sname)<2)
  {
    strcpy(dname,FUNC_HEADSTR);
    int dname_begin=strlen(dname);
    strcpy(dname+dname_begin,sname);
    return dname;
  }
  char name0=sname[0];
  char name1=sname[1];
  if ((name1=='@')||(name1=='?'))
  {
    dname[0]=name0;
    dname[1]=name1;
    name_begin=2;
  } else
  if ((name0=='@')||(name0=='?'))
  {
    dname[0]=name0;
    name_begin=1;
  } else
  {
    name_begin=0;
  }
  strcpy(dname+name_begin,FUNC_HEADSTR);
  int dname_begin=strlen(dname);
  strncpy(dname+dname_begin,sname+name_begin,MAX_EXPORT_NAMELEN-dname_begin-1);
  dname[MAX_EXPORT_NAMELEN]='\0';
  return dname;
}

int main(int argc, char *argv[])
{
  struct export_entry *exports[MAX_EXPORTS];
  int idx;
  for (idx=0;idx<MAX_EXPORTS;idx++)
      exports[idx]=NULL;
  // Reading functions
  FILE *fhndl=fopen("lib/keeper95_gold.map","rb");
  if (fhndl==NULL)
  {
    printf("Can't open .MAP file!\n");
    return 1;
  }
  idx=0;
  while (!feof(fhndl))
  {
    exports[idx]=malloc(sizeof(struct export_entry));
    if (exports[idx]==NULL)
    {
      printf("Memory allocation error!\n");
      return 4;
    }
    int nread=fscanf(fhndl," %hx:%lx %255s",&(exports[idx]->seg),&(exports[idx]->offs),exports[idx]->srcname);
    if ((nread<3)||(strlen(exports[idx]->srcname)<1))
    {
      if ((nread<=0) && feof(fhndl))
      {
        free(exports[idx]);
        exports[idx]=NULL;
        break;
      }
      printf("Error reading entry %d!\n",idx);
      return 2;
    } else
    {
      get_name_with_prefix(exports[idx]->dstname,exports[idx]->srcname);
      exports[idx]->nmoffs=0;
      idx++;
      if (idx>=MAX_EXPORTS)
      {
        printf("Too many exports in .MAP file!\n");
        return 3;
      }
    }
  }
  fclose(fhndl);
  printf("Got %d entries from .MAP file.\n",idx);
  //Sorting functions
  export_sort(exports,idx);
  printf("Entries are now sorted.\n");
  // Checking
  int dupidx;
  dupidx=find_dupename(exports,idx);
  if (dupidx>=0)
  {
    printf("Duplicate entry name found!\n");
    printf("Entry \"%s\" duplicates. Aborting.\n",exports[dupidx]->dstname);
    return 7;
  }
  dupidx=find_dupeoffs(exports,idx);
  if (dupidx>=0)
  {
    printf("Duplicate entry offset found!\n");
    printf("Offset 0x%08lX duplicates. Aborting.\n",exports[dupidx]->offs);
    return 8;
  }
  //Saving the entries
  fhndl=fopen("bin/keeperfx.dll","r+b");
  if (fhndl==NULL)
  {
    printf("Can't open .DLL file!\n");
    return 5;
  }
  fseek(fhndl,ARR_POS_NMSTRS,SEEK_SET);
  for (idx=0;idx<MAX_EXPORTS;idx++)
  {
      if (exports[idx]==NULL) break;
      unsigned long nmpos=ftell(fhndl);
      char *name=exports[idx]->dstname;
      exports[idx]->nmoffs=nmpos+ARR_RAW_TO_RVA;
      if (nmpos+sizeof(name)>=ARR_POS_NMSTRS+MAX_FNAMES_SIZE)
      {
        printf("Function names space exceeded on func. %d!\n",idx);
        exports[idx]=NULL;
        break;
      }
      fputs(name,fhndl);
      fputc('\0',fhndl);
  }
  long remain_bts=ARR_POS_NMSTRS+MAX_FNAMES_SIZE-ftell(fhndl)-strlen(END_STR);
  if (remain_bts>=0)
  {
    while (remain_bts>0)
    {
      fputc('\0',fhndl);
      remain_bts--;
    }
    fputs(END_STR,fhndl);
  }
  printf("Written %d function export names into .DLL.\n",idx);
  fseek(fhndl,EXP_SEC_HDROFS+0x14,SEEK_SET);
  {
        unsigned long val=idx;
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
  }
  {
        unsigned long val=idx;
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
  }
  {
        unsigned long val=ARR_RAW_TO_RVA+ARR_POS_FUNC;
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
  }
  {
        unsigned long val=ARR_RAW_TO_RVA+ARR_POS_FNAMES;
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
  }
  {
        unsigned long val=ARR_RAW_TO_RVA+ARR_POS_FORDIN;
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
  }
  printf("Export section header updated.\n");
  fseek(fhndl,ARR_POS_FORDIN,SEEK_SET);
  for (idx=0;idx<MAX_EXPORTS;idx++)
  {
      if (exports[idx]==NULL)
      {
        fputc('\0',fhndl);
        fputc('\0',fhndl);
      } else
      {
        fputc(((idx)&0xff),fhndl);
        fputc(((idx>>8)&0xff),fhndl);
      }
  }
  printf("Written %d function export ordinals into .DLL.\n",idx);
  fseek(fhndl,ARR_POS_FUNC,SEEK_SET);
  for (idx=0;idx<MAX_EXPORTS;idx++)
  {
      if (exports[idx]==NULL)
      {
        fputc('\0',fhndl);
        fputc('\0',fhndl);
        fputc('\0',fhndl);
        fputc('\0',fhndl);
      } else
      {
        unsigned long val=exports[idx]->offs+sections_va[(exports[idx]->seg)%SECTIONS_NUM];
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
      }
  }
  printf("Written %d function offsets into .DLL.\n",idx);
  fseek(fhndl,ARR_POS_FNAMES,SEEK_SET);
  for (idx=0;idx<MAX_EXPORTS;idx++)
  {
      if (exports[idx]==NULL)
      {
        fputc('\0',fhndl);
        fputc('\0',fhndl);
        fputc('\0',fhndl);
        fputc('\0',fhndl);
      } else
      {
        unsigned long val=exports[idx]->nmoffs;
        fputc(((val)&0xff),fhndl);
        fputc(((val>>8)&0xff),fhndl);
        fputc(((val>>16)&0xff),fhndl);
        fputc(((val>>24)&0xff),fhndl);
      }
  }
  printf("Written %d function name offsets into .DLL.\n",idx);
  fclose(fhndl);
  fhndl=fopen("lib/keeperfx.def","wb");
  if (fhndl==NULL)
  {
    printf("Can't open .DEF file!\n");
    return 10;
  }
  fprintf(fhndl,"LIBRARY     KEEPERFX.DLL\n");
  fprintf(fhndl,"\nEXPORTS\n");
  for (idx=0;idx<MAX_EXPORTS;idx++)
  {
      if (exports[idx]==NULL)
        break;
      char *name=exports[idx]->dstname;
      unsigned long val=exports[idx]->offs+sections_va[(exports[idx]->seg)%SECTIONS_NUM];
      fprintf(fhndl,"    %-36s ; RVA=0x%08lX\n",name,val);
  }
  fclose(fhndl);
  printf("Written %d names into .DEF file.\n",idx);
  return 0;
}
