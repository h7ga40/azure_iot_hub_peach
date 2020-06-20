#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <direct.h>
#include <math.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/compile.h>
#include <mruby/dump.h>
#include <mruby/variable.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <mruby/hash.h>
#include <mruby/range.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/error.h>
#include <mruby/gc.h>
#include <mruby/debug.h>
#include <mruby/istruct.h>

#ifndef _MSC_VER
#include <libgen.h>
#define sprintf_s(buf, len, ...) sprintf(buf, __VA_ARGS__)
#define __ImageBase __executable_start
#define IMAGE_BASE_NAME "__executable_start"
#else
#define IMAGE_BASE_NAME "___ImageBase"
#define PATH_MAX 256
#define getcwd _getcwd
char *dirname(char *s)
{
  size_t i;
  if (!s || !*s) return ".";
  i = strlen(s)-1;
  for (; s[i]=='\\'; i--) if (!i) return "\\";
  for (; s[i]!='\\'; i--) if (!i) return ".";
  for (; s[i]=='\\'; i--) if (!i) return "\\";
  s[i+1] = 0;
  return s;
}
#endif

struct dump_args {
  int argc_start;
  int argc;
  char **argv;
  mrb_bool lvar;
};

KHASH_DECLARE(iv, mrb_sym, mrb_value, TRUE)
KHASH_DECLARE(n2s, mrb_sym, mrb_sym, FALSE)

//#include <mruby/boxing_word.h>
struct RFloat {
  MRB_OBJECT_HEADER;
  mrb_float f;
};

struct RCptr {
  MRB_OBJECT_HEADER;
  void *p;
};

typedef union {
  struct RBasic basic;
  struct RObject object;
  struct RClass klass;
  struct RString string;
  struct RArray array;
  struct RHash hash;
  struct RRange range;
  struct RData data;
  struct RProc proc;
  struct RException exc;
  struct RFiber fiber;
  struct REnv env;
  struct RIstruct istruct;
  struct RBreak brk;
  struct RFloat floatv;
  struct RCptr cptr;
} RVALUE;

struct queue_t {
  struct queue_t *next;
};

struct symbol_list {
  struct symbol_list *next;
  intptr_t addr;
  char T;
  int func;
  int data;
  char name[1];
};

struct string_list {
  struct string_list *next;
  int no;
  const char *string;
  int length;
};

struct obj_list {
  struct obj_list *next;
  struct obj_list *gcnext;
  int no;
  RVALUE *object;
};

struct irep_list {
  struct irep_list *next;
  int no;
  mrb_irep *irep;
};

struct debug_info_list {
  struct debug_info_list *next;
  int no;
  mrb_irep_debug_info *debug_info;
};

struct debug_info_file_list {
  struct debug_info_file_list *next;
  int no;
  mrb_irep_debug_info_file *debug_info_file;
};

struct array_list {
  struct array_list *next;
  int no;
  struct RArray *array;
};

struct stack_list {
  struct stack_list *next;
  int no;
  mrb_value *base;
  mrb_value *end;
};

struct iv_tbl_list {
  struct iv_tbl_list *next;
  int no;
  struct iv_tbl *iv;
};

struct kh_mt_list {
  struct kh_mt_list *next;
  int no;
  struct kh_mt *mt;
};

struct context_list {
  struct context_list *next;
  int no;
  struct mrb_context *context;
};

struct callinfo_list {
  struct callinfo_list *next;
  int no;
  mrb_callinfo *base;
  mrb_callinfo *end;
};

struct os_each_object_data {
  mrb_state *mrb;
  FILE *wfile;
  struct symbol_list *symbols;
  intptr_t offset;
  int string_count;
  struct string_list *strings;
  int obj_count;
  struct obj_list *objs;
  int irep_count;
  struct irep_list *ireps;
  int debug_info_count;
  struct debug_info_list *debug_infos;
  int debug_info_file_count;
  struct debug_info_file_list *debug_info_files;
  int array_count;
  struct array_list *arrays;
  int stack_count;
  struct stack_list *stacks;
  int iv_tbl_count;
  struct iv_tbl_list *iv_tbls;
  int kh_mt_count;
  struct kh_mt_list *kh_mts;
  int context_count;
  struct context_list *contexts;
  int callinfo_count;
  struct callinfo_list *callinfos;
  struct obj_list *toplevels;
};

extern int __ImageBase;
#define OFFSET_FROM_IMAGE_BASE(x) (((x) == NULL) ? 0 : ((intptr_t)(x) - (intptr_t)(&__ImageBase)))

static void add_obj(struct os_each_object_data *d, struct RBasic *obj);

static void
enqueue(struct queue_t **list, struct queue_t *item)
{
  if (*list == NULL) {
    *list = item;
    item->next = item;
  }
  else {
    item->next = (*list)->next/*first*/;
    (*list)->next = item;
    *list = item;
  }
}

static void
clear_queue(struct queue_t **list)
{
  if (*list == NULL)
    return;

  struct symbol_list *first = (struct symbol_list *)(*list)->next;
  struct symbol_list *item = (struct symbol_list *)first->next;
  while (item != first) {
    struct symbol_list *next = item->next;
    free(item);
    item = next;
  }
  free(first);
}

static char *
escape_string(const char *str, int len)
{
  if ((str == NULL) || (len <= 0))
    return NULL;

  const unsigned char *src = str, *end = &str[len];
  for (unsigned char c = *src++; src < end; c = *src++) {
    if ((c == '\\') || (c == '"') || (c == '\r') || (c == '\n') || (c == '\t')) {
      len++;
    }
    else if (!isprint(c)) {
      len += 3;
    }
  }

  unsigned char *ret = malloc(len + 1);
  unsigned char *dst = ret;
  src = str;
  for (unsigned char c = *src++; src < end; c = *src++) {
    if ((c == '\\') || (c == '"')) {
      *dst++ = '\\';
      *dst++ = c;
    }
    else if (c == '\r') {
      *dst++ = '\\';
      *dst++ = 'r';
    }
    else if (c == '\n') {
      *dst++ = '\\';
      *dst++ = 'n';
    }
    else if (c == '\t') {
      *dst++ = '\\';
      *dst++ = 't';
    }
    else if (!isprint(c)) {
      *dst++ = '\\';
      *dst++ = 'x';
      *dst++ = ((c >> 4) < 10) ? ((c >> 4) + '0') : ((c >> 4) - 10 + 'A');
      *dst++ = ((c & 0xF) < 10) ? ((c & 0xF) + '0') : ((c & 0xF) - 10 + 'A');
    }
    else {
      *dst++ = c;
    }
  }
  *dst = '\0';

  return ret;
}

static char *
escape_string2(const char *str, int len)
{
  if ((str == NULL) || (len <= 0))
    return NULL;

  const unsigned char *src = str, *end = &str[len];
  for (unsigned char c = *src++; src < end; c = *src++) {
    if ((c == '\r') || (c == '\n') || (c == '\t')) {
      len++;
    }
    else if (!isprint(c)) {
      len += 3;
    }
  }

  unsigned char *ret = malloc(len + 1);
  unsigned char *dst = ret;
  src = str;
  for (unsigned char c = *src++; src < end; c = *src++) {
    if (c == '\r') {
      *dst++ = '\\';
      *dst++ = 'r';
    }
    else if (c == '\n') {
      *dst++ = '\\';
      *dst++ = 'n';
    }
    else if (c == '\t') {
      *dst++ = '\\';
      *dst++ = 't';
    }
    else if (!isprint(c)) {
      *dst++ = '\\';
      *dst++ = 'x';
      *dst++ = ((c >> 4) < 10) ? ((c >> 4) + '0') : ((c >> 4) - 10 + 'A');
      *dst++ = ((c & 0xF) < 10) ? ((c & 0xF) + '0') : ((c & 0xF) - 10 + 'A');
    }
    else {
      *dst++ = c;
    }
  }
  *dst = '\0';

  return ret;
}

static struct symbol_list *
search_symbol_item(struct os_each_object_data *d, intptr_t addr)
{
  struct symbol_list *symbols = d->symbols;
  if (symbols == NULL)
    return NULL;

  struct symbol_list *prev = NULL, *first = symbols->next, *item = first;
  for (;;) {
    if (addr > item->addr) {
      prev = item;
      item = item->next;
      if (item == first)
        break;
      continue;
    }

    if (addr == item->addr) {
      return item;
    }
    else if (prev != NULL) {
      return prev;
    }
    else
      return NULL;
  }

  return NULL;
}

char syms_path[PATH_MAX];

static int
read_symbol_file(struct os_each_object_data *d)
{
  FILE *rfile;
  int ret;
  intptr_t addr;
#ifndef _MSC_VER
  char temp[20], T, name[256];
#else
  char T, name[PATH_MAX];
#endif

#ifndef _MSC_VER
  rfile = fopen(syms_path, "rb");
  if (rfile == NULL) {
    fprintf(stderr, "open failed. %s file\n", syms_path);
    return -1;
  }
#else
  ret = fopen_s(&rfile, syms_path, "rb");
  if (ret != 0) {
    fprintf(stderr, "open failed. %s file\n", syms_path);
    return -1;
  }
#endif

  for (;;) {
    addr = 0;
    T = 0;
    memset(name, 0, sizeof(name));
#ifndef _MSC_VER
    memset(temp, 0, sizeof(temp));
    ret = fscanf(rfile, "%16s %c %256s", &temp, &T, &name);
    addr = strtol(temp, NULL, 16);
#else
    ret = fscanf_s(rfile, "%x %c %s", &addr, &T, 1, &name, sizeof(name));
#endif
    if (ret == EOF)
      break;
    if (addr == 0)
      continue;

    struct symbol_list *prev = search_symbol_item(d, addr);

    int len = strlen(name);
    struct symbol_list *item = malloc(sizeof(struct symbol_list) + len);
    item->addr = addr;
    item->T = T;
    item->name[0] = '\0';
    item->func = 0;
    item->data = 0;
#ifndef _MSC_VER
    strcpy(item->name, name);
#else
    strcpy_s(item->name, len + 1, &name[1]);
#endif

    if ((prev == NULL) || (prev == d->symbols))
      enqueue((struct queue_t **)&d->symbols, (struct queue_t *)item);
    else {
      item->next = prev->next;
      prev->next = item;
    }

    if (strcmp(name, IMAGE_BASE_NAME) == 0) {
      d->offset = addr;
    }
  }

  fclose(rfile);

  return 0;
}

static int
search_stringno(struct os_each_object_data *d, const char *string, int length)
{
  struct string_list *strings = d->strings;
  if (strings == NULL)
    return -1;

  struct string_list *first = strings->next, *item = first;
  do {
    if (memcmp(item->string, string, length) == 0)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
add_string(struct os_each_object_data *d, const char *string, int length)
{
  struct string_list *item;

  int no = search_stringno(d, string, length);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct string_list));
  item->next = NULL;
  item->string = string;
  item->length = length;
  item->no = d->string_count;

  d->string_count++;
  enqueue((struct queue_t **)&d->strings, (struct queue_t *)item);
}

static int
search_objno(struct os_each_object_data *d, struct RBasic *obj)
{
  struct obj_list *objs = d->objs;
  if (objs == NULL)
    return -1;

  struct obj_list *first = objs->next, *item = first;
  do {
    if (&item->object->basic == obj)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static int
search_irepno(struct os_each_object_data *d, const mrb_irep *irep)
{
  struct irep_list *ireps = d->ireps;
  if (ireps == NULL)
    return -1;

  struct irep_list *first = ireps->next, *item = first;
  do {
    if (item->irep == irep)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
prepare_value(struct os_each_object_data *d, mrb_value val)
{
  int no = search_objno(d, mrb_ptr(val));
  if (no >= 0)
    return;

  switch (mrb_type(val)) {
  case MRB_TT_FALSE:
  case MRB_TT_FREE:
  case MRB_TT_TRUE:
  case MRB_TT_FIXNUM:
  case MRB_TT_SYMBOL:
  case MRB_TT_UNDEF:
  case MRB_TT_FLOAT:
  case MRB_TT_CPTR:
    break;
  case MRB_TT_OBJECT:
  case MRB_TT_CLASS:
  case MRB_TT_MODULE:
  case MRB_TT_ICLASS:
  case MRB_TT_SCLASS:
  case MRB_TT_PROC:
  case MRB_TT_ARRAY:
  case MRB_TT_HASH:
  case MRB_TT_RANGE:
  case MRB_TT_EXCEPTION:
  case MRB_TT_FILE:
  case MRB_TT_ENV:
  case MRB_TT_DATA:
  case MRB_TT_FIBER:
  case MRB_TT_ISTRUCT:
  case MRB_TT_BREAK:
    add_obj(d, mrb_ptr(val));
    break;
  case MRB_TT_STRING:
    add_obj(d, mrb_ptr(val));
    if (!RSTR_EMBED_P(mrb_str_ptr(val))
      && (mrb_str_ptr(val)->as.heap.ptr != NULL) && (mrb_str_ptr(val)->as.heap.len > 0)) {
      add_string(d, mrb_str_ptr(val)->as.heap.ptr, mrb_str_ptr(val)->as.heap.len);
    }
    break;
  case MRB_TT_MAXDEFINE:
    break;
  default:
    break;
  }
}

static void
add_irep(struct os_each_object_data *d, mrb_irep *irep)
{
  struct irep_list *item;

  int no = search_irepno(d, irep);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct irep_list));
  item->next = NULL;
  item->irep = irep;
  item->no = d->irep_count;

  d->irep_count++;
  enqueue((struct queue_t **)&d->ireps, (struct queue_t *)item);

  for (int i = 0; i < irep->plen; i++) {
    prepare_value(d, irep->pool[i]);
  }

  mrb_irep_debug_info *debug_info = irep->debug_info;
  if (debug_info != NULL) {
    {
      struct debug_info_list *item = malloc(sizeof(struct debug_info_list));
      item->next = NULL;
      item->debug_info = debug_info;
      item->no = d->debug_info_count;

      d->debug_info_count++;
      enqueue((struct queue_t **)&d->debug_infos, (struct queue_t *)item);
    }

    if (debug_info->files != NULL) {
      for (int i = 0; i < debug_info->flen; i++) {
        mrb_irep_debug_info_file *file = debug_info->files[i];
        {
          struct debug_info_file_list *item = malloc(sizeof(struct debug_info_file_list));
          item->next = NULL;
          item->debug_info_file = file;
          item->no = d->debug_info_file_count;

          d->debug_info_file_count++;
          enqueue((struct queue_t **)&d->debug_info_files, (struct queue_t *)item);

          if (file->filename != NULL)
            add_string(d, file->filename, strlen(file->filename) + 1);
        }
      }
    }
  }

  for (int i = 0; i < irep->rlen; i++) {
    struct mrb_irep *rep = irep->reps[i];
    add_irep(d, rep);
  }
}

static struct stack_list *
search_stackno(struct os_each_object_data *d, mrb_value *stack)
{
  struct stack_list *stacks = d->stacks;
  if (stacks == NULL)
    return NULL;

  struct stack_list *first = stacks->next, *item = first;
  do {
    if ((stack >= item->base) && (stack <= item->end))
      return item;
    item = item->next;
  } while (item != first);

  return NULL;
}

static void
add_stack(struct os_each_object_data *d, mrb_value *base, mrb_value *end)
{
  struct stack_list *item;

  if (base == NULL)
    return;

  item = search_stackno(d, base);
  if (item != NULL)
    return;

  item = malloc(sizeof(struct stack_list));
  item->next = NULL;
  item->base = base;
  item->end = end;
  item->no = d->stack_count;

  d->stack_count++;
  enqueue((struct queue_t **)&d->stacks, (struct queue_t *)item);

  for (mrb_value *pos = base; pos < end; pos++) {
    prepare_value(d, *pos);
  }
}

static struct callinfo_list *
search_callinfono(struct os_each_object_data *d, mrb_callinfo *callinfo)
{
  struct callinfo_list *callinfos = d->callinfos;
  if (callinfos == NULL)
    return NULL;

  struct callinfo_list *first = callinfos->next, *item = first;
  do {
    if ((callinfo >= item->base) && (callinfo <= item->end))
      return item;
    item = item->next;
  } while (item != first);

  return NULL;
}

static void
add_callinfo(struct os_each_object_data *d, mrb_callinfo *base, mrb_callinfo *end)
{
  struct callinfo_list *item;

  if (base == NULL)
    return;

  item = search_callinfono(d, base);
  if (item != NULL)
    return;

  item = malloc(sizeof(struct callinfo_list));
  item->next = NULL;
  item->base = base;
  item->end = end;
  item->no = d->callinfo_count;

  d->callinfo_count++;
  enqueue((struct queue_t **)&d->callinfos, (struct queue_t *)item);

  for (mrb_callinfo *pos = base; pos < end; pos++) {
    add_obj(d, (struct RBasic *)pos->proc);
    add_obj(d, (struct RBasic *)pos->env);
    add_obj(d, (struct RBasic *)pos->target_class);
  }
}

static int
search_contextno(struct os_each_object_data *d, struct mrb_context *context)
{
  struct context_list *contexts = d->contexts;
  if (contexts == NULL)
    return -1;

  struct context_list *first = contexts->next, *item = first;
  do {
    if (item->context == context)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
add_context(struct os_each_object_data *d, struct mrb_context *context)
{
  struct context_list *item;

  int no = search_contextno(d, context);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct context_list));
  item->next = NULL;
  item->context = context;
  item->no = d->context_count;

  d->context_count++;
  enqueue((struct queue_t **)&d->contexts, (struct queue_t *)item);

  add_stack(d, context->stbase, context->stend);
  add_callinfo(d, context->cibase, context->ciend);
}

static int
search_arrayno(struct os_each_object_data *d, struct RArray *array)
{
  struct array_list *arrays = d->arrays;
  if (arrays == NULL)
    return -1;

  struct array_list *first = arrays->next, *item = first;
  do {
    if (item->array == array)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
add_array(struct os_each_object_data *d, struct RArray *array)
{
  struct array_list *item;

  int no = search_arrayno(d, array);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct array_list));
  item->next = NULL;
  item->array = array;
  item->no = d->array_count;

  d->array_count++;
  enqueue((struct queue_t **)&d->arrays, (struct queue_t *)item);

  if (ARY_SHARED_P(array)) {
    mrb_shared_array *shared = array->as.heap.aux.shared;
    for (int i = 0; i < shared->len; i++) {
      mrb_value *val = &shared->ptr[i];
      if (val->tt == MRB_TT_ARRAY) {
        add_array(d, mrb_ary_ptr(*val));
      }
    }
  }
  else {
    for (int i = 0; i < array->as.heap.len; i++) {
      mrb_value *val = &array->as.heap.ptr[i];
      if (val->tt == MRB_TT_ARRAY) {
        add_array(d, mrb_ary_ptr(*val));
      }
    }
  }
}

static int
search_iv_tblno(struct os_each_object_data *d, struct iv_tbl *iv)
{
  struct iv_tbl_list *iv_tbls = d->iv_tbls;
  if (iv_tbls == NULL)
    return -1;

  struct iv_tbl_list *first = iv_tbls->next, *item = first;
  do {
    if (item->iv == iv)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
add_iv(struct os_each_object_data *d, struct iv_tbl *iv)
{
  struct iv_tbl_list *item;

  int no = search_iv_tblno(d, iv);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct iv_tbl_list));
  item->next = NULL;
  item->iv = iv;
  item->no = d->iv_tbl_count;

  d->iv_tbl_count++;
  enqueue((struct queue_t **)&d->iv_tbls, (struct queue_t *)item);

  khint_t i;

  khash_t(iv) *h = (khash_t(iv) *)iv;
  if (!h || kh_size(h) == 0)
    return;
  for (i = 0; i < kh_end(h); i++) {
    if (kh_exist(h, i)) {
      mrb_value v = kh_value(h, i);
      prepare_value(d, v);
    }
  }
}

static int
search_kh_mtno(struct os_each_object_data *d, struct kh_mt *mt)
{
  struct kh_mt_list *kh_mts = d->kh_mts;
  if (kh_mts == NULL)
    return -1;

  struct kh_mt_list *first = kh_mts->next, *item = first;
  do {
    if (item->mt == mt)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
add_mt(struct os_each_object_data *d, struct kh_mt *mt)
{
  struct kh_mt_list *item;

  int no = search_kh_mtno(d, mt);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct kh_mt_list));
  item->next = NULL;
  item->mt = mt;
  item->no = d->kh_mt_count;

  d->kh_mt_count++;
  enqueue((struct queue_t **)&d->kh_mts, (struct queue_t *)item);

  khint_t i;

  khash_t(mt) *h = mt;
  if (!h || kh_size(h) == 0)
    return;
  for (i = 0; i < kh_end(h); i++) {
    if (kh_exist(h, i)) {
      mrb_method_t m = kh_value(h, i);
      if (MRB_METHOD_UNDEF_P(m) || m.func_p)
        continue;

      add_obj(d, (struct RBasic *)m.proc);
    }
  }
}

static int
search_debug_info_no(struct os_each_object_data *d, const mrb_irep_debug_info *debug_info)
{
  struct debug_info_list *debug_infos = d->debug_infos;
  if (debug_infos == NULL)
    return -1;

  struct debug_info_list *first = debug_infos->next, *item = first;
  do {
    if (item->debug_info == debug_info)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static int
search_debug_info_file_no(struct os_each_object_data *d, const mrb_irep_debug_info_file *debug_info_file)
{
  struct debug_info_file_list *debug_info_files = d->debug_info_files;
  if (debug_info_files == NULL)
    return -1;

  struct debug_info_file_list *first = debug_info_files->next, *item = first;
  do {
    if (item->debug_info_file == debug_info_file)
      return item->no;
    item = item->next;
  } while (item != first);

  return -1;
}

static void
add_gcnext(struct os_each_object_data *d, struct obj_list *item)
{
  if (d->toplevels == NULL) {
    d->toplevels = item;
    item->gcnext = item;
  }
  else {
    item->gcnext = d->toplevels->gcnext/*first*/;
    d->toplevels->gcnext = item;
    d->toplevels = item;
  }
}

static void
add_obj(struct os_each_object_data *d, struct RBasic *obj)
{
  struct obj_list *item;

  if (obj == NULL)
    return;

  int no = search_objno(d, obj);
  if (no >= 0)
    return;

  item = malloc(sizeof(struct obj_list));
  item->next = NULL;
  item->gcnext = NULL;
  item->object = (RVALUE *)obj;
  item->no = d->obj_count;

  d->obj_count++;
  enqueue((struct queue_t **)&d->objs, (struct queue_t *)item);

  RVALUE *val = (RVALUE *)obj;

  /* filter dead objects */
  if ((obj == NULL) || mrb_object_dead_p(d->mrb, obj)) {
    return;
  }

  switch (obj->tt) {
  case MRB_TT_PROC:
    if (MRB_PROC_CFUNC_P(obj)) {
      if (val->proc.upper != NULL)
        add_obj(d, (struct RBasic *)val->proc.upper);
      if (val->proc.e.target_class != NULL)
        add_obj(d, (struct RBasic *)val->proc.e.target_class);
    }
    else {
      mrb_irep *irep = val->proc.body.irep;
      if (irep != NULL) {
        add_irep(d, irep);
      }
    }
    break;
  case MRB_TT_ENV:
    //if(obj->flags != 0)
    //	add_stack(d, &val->env, &val->env.stack[0], &val->env.stack[obj->flags]);
    add_context(d, val->env.cxt);
    break;
  case MRB_TT_ARRAY:
    add_array(d, &val->array);
    break;
  case MRB_TT_OBJECT:
  case MRB_TT_CLASS:
  case MRB_TT_MODULE:
  case MRB_TT_ICLASS:
  case MRB_TT_SCLASS:
  case MRB_TT_HASH:
  case MRB_TT_EXCEPTION:
  case MRB_TT_DATA:
    if (val->object.iv == NULL) {
      val->object.iv = (struct iv_tbl *)kh_init_iv_size(d->mrb, KHASH_DEFAULT_SIZE);
    }
    add_iv(d, val->object.iv);
    if (val->basic.c != NULL)
      add_obj(d, (struct RBasic *)val->basic.c);
    add_gcnext(d, item);
    break;
  case MRB_TT_STRING:
    if (!RSTR_EMBED_P(obj)
      && (val->string.as.heap.ptr != NULL) && (val->string.as.heap.len > 0)) {
      add_string(d, val->string.as.heap.ptr, val->string.as.heap.len);
    }
    break;
  }

  switch (obj->tt) {
  case MRB_TT_CLASS:
  case MRB_TT_MODULE:
  case MRB_TT_ICLASS:
  case MRB_TT_SCLASS:
    if (val->klass.mt != NULL)
      add_mt(d, val->klass.mt);
    if (val->klass.super != NULL)
      add_obj(d, (struct RBasic *)val->klass.super);
    break;
  }
}

static void
print_each_uint8(struct os_each_object_data *d, uint8_t *data, int len)
{
  for (int j = 0; j < len; j += 16) {
    int i, e = j + 16;
    if (e > len)
      e = len;

    i = j;
    fprintf(d->wfile, "\t0x%02x, ", data[i]);
    i++;
    if (i >= e)
      break;

    for (; i < e - 1; i++) {
      fprintf(d->wfile, "0x%02x, ", data[i]);
    }
    fprintf(d->wfile, "0x%02x,\n", data[i]);
  }
}

static void
print_each_sym(struct os_each_object_data *d, mrb_sym *data, int len)
{
  for (int j = 0; j < len; j += 16) {
    int i, e = j + 16;
    if (e > len)
      e = len;

    i = j;
    fprintf(d->wfile, "\t%d, ", data[i]);
    i++;
    if (i >= e)
      break;

    for (; i < e - 1; i++) {
      fprintf(d->wfile, "%d, ", data[i]);
    }
    fprintf(d->wfile, "%d,\n", data[i]);
  }
}

static const char *
get_type_str(enum mrb_vtype tt)
{
  switch (tt) {
  case MRB_TT_FALSE:
    return "MRB_TT_FALSE";
  case MRB_TT_FREE:
    return "MRB_TT_FREE";
  case MRB_TT_TRUE:
    return "MRB_TT_TRUE";
  case MRB_TT_FIXNUM:
    return "MRB_TT_FIXNUM";
  case MRB_TT_SYMBOL:
    return "MRB_TT_SYMBOL";
  case MRB_TT_UNDEF:
    return "MRB_TT_UNDEF";
  case MRB_TT_FLOAT:
    return "MRB_TT_FLOAT";
  case MRB_TT_CPTR:
    return "MRB_TT_CPTR";
  case MRB_TT_OBJECT:
    return "MRB_TT_OBJECT";
  case MRB_TT_CLASS:
    return "MRB_TT_CLASS";
  case MRB_TT_MODULE:
    return "MRB_TT_MODULE";
  case MRB_TT_ICLASS:
    return "MRB_TT_ICLASS";
  case MRB_TT_SCLASS:
    return "MRB_TT_SCLASS";
  case MRB_TT_PROC:
    return "MRB_TT_PROC";
  case MRB_TT_ARRAY:
    return "MRB_TT_ARRAY";
  case MRB_TT_HASH:
    return "MRB_TT_HASH";
  case MRB_TT_STRING:
    return "MRB_TT_STRING";
  case MRB_TT_RANGE:
    return "MRB_TT_RANGE";
  case MRB_TT_EXCEPTION:
    return "MRB_TT_EXCEPTION";
  case MRB_TT_FILE:
    return "MRB_TT_FILE";
  case MRB_TT_ENV:
    return "MRB_TT_ENV";
  case MRB_TT_DATA:
    return "MRB_TT_DATA";
  case MRB_TT_FIBER:
    return "MRB_TT_FIBER";
  case MRB_TT_ISTRUCT:
    return "MRB_TT_ISTRUCT";
  case MRB_TT_BREAK:
    return "MRB_TT_BREAK";
  case MRB_TT_MAXDEFINE:
    return "MRB_TT_MAXDEFINE";
  default:
    return NULL;
  }
}

static int
sprint_value_str(struct os_each_object_data *d, char *buf, int len, mrb_value val)
{
  int no;

  switch (mrb_type(val)) {
  case MRB_TT_FALSE:
    return sprintf_s(buf, len, ".i = %d", mrb_fixnum(val));
  case MRB_TT_FREE:
    return sprintf_s(buf, len, ".i = %d", mrb_fixnum(val));
  case MRB_TT_TRUE:
    return sprintf_s(buf, len, ".i = %d", mrb_fixnum(val));
  case MRB_TT_FIXNUM:
    return sprintf_s(buf, len, ".i = %d", mrb_fixnum(val));
  case MRB_TT_SYMBOL:
    return sprintf_s(buf, len, ".sym = (mrb_sym)%d", mrb_symbol(val));
  case MRB_TT_UNDEF:
    return sprintf_s(buf, len, ".i = %d", mrb_fixnum(val));
  case MRB_TT_FLOAT:
    if (mrb_float(val) == INFINITY)
      return sprintf_s(buf, len, ".f = INFINITY");
    else if (mrb_float(val) == NAN)
      return sprintf_s(buf, len, ".f = NAN");
    else
      return sprintf_s(buf, len, ".f = %G", mrb_float(val));
  case MRB_TT_CPTR:
    return sprintf_s(buf, len, ".p = 0x%p", mrb_cptr(val));
  case MRB_TT_OBJECT:
  case MRB_TT_CLASS:
  case MRB_TT_MODULE:
  case MRB_TT_ICLASS:
  case MRB_TT_SCLASS:
  case MRB_TT_PROC:
  case MRB_TT_ARRAY:
  case MRB_TT_HASH:
  case MRB_TT_STRING:
  case MRB_TT_RANGE:
  case MRB_TT_EXCEPTION:
  case MRB_TT_FILE:
  case MRB_TT_ENV:
  case MRB_TT_DATA:
  case MRB_TT_FIBER:
  case MRB_TT_ISTRUCT:
  case MRB_TT_BREAK:
    if ((no = search_objno(d, mrb_ptr(val))) >= 0)
      return sprintf_s(buf, len, ".p = (void *)&mrb_preset_object_%d", no);
    else
      return sprintf_s(buf, len, ".p = (void *)0x%p", mrb_ptr(val));
  case MRB_TT_MAXDEFINE:
    return sprintf_s(buf, len, ".p = (void *)0x%p", mrb_ptr(val));
  default:
    return sprintf_s(buf, len, ".p = (void *)0x%p", mrb_ptr(val));
  }
}

static void
print_each_irep_pool(struct os_each_object_data *d, struct irep_list *item)
{
  mrb_irep *irep = item->irep;

  if (irep->plen <= 0)
    return;

  fprintf(d->wfile, "PRESET_CONST mrb_value mrb_preset_irep_%d_pool[] = {\n", item->no);

  for (int i = 0; i < irep->plen; i++) {
    mrb_value val = irep->pool[i];
    char temp[256];
    const char *tt = get_type_str(mrb_type(val));

    temp[0] = '\0';
    sprint_value_str(d, temp, sizeof(temp), val);

    if (tt == NULL)
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = enum_mrb_vtype_%u },\n", temp, mrb_type(val));
    else
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = %s },\n", temp, tt);
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_irep_syms(struct os_each_object_data *d, struct irep_list *item)
{
  mrb_irep *irep = item->irep;

  if (irep->slen <= 0)
    return;

  fprintf(d->wfile, "PRESET_CONST mrb_sym mrb_preset_irep_%d_syms[] = { ", item->no);

  for (int i = 0; i < irep->slen; i++) {
    mrb_sym sym = irep->syms[i];
    fprintf(d->wfile, "%d, ", sym);
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_irep_reps(struct os_each_object_data *d, struct irep_list *item)
{
  mrb_irep *irep = item->irep;

  if (irep->rlen <= 0)
    return;

  fprintf(d->wfile, "PRESET_CONST struct mrb_irep *const mrb_preset_irep_%d_reps[] = { ", item->no);

  for (int i = 0; i < irep->rlen; i++) {
    struct mrb_irep *rep = irep->reps[i];
    int no = search_irepno(d, rep);
    if (no >= 0)
      fprintf(d->wfile, "&mrb_preset_irep_%d, ", no);
    else
      fprintf(d->wfile, "(mrb_irep *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(rep));
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_irep_lvs(struct os_each_object_data *d, struct irep_list *item)
{
  mrb_irep *irep = item->irep;

  int len = irep->nlocals - 1;
  if (len <= 0)
    return;

  fprintf(d->wfile, "PRESET_CONST struct mrb_locals mrb_preset_irep_%d_lvs[] = {\n", item->no);

  for (int i = 0; i < len; i++) {
    struct mrb_locals *lv = &irep->lv[i];
    fprintf(d->wfile, "\t{ .name = (mrb_sym)%d, .r = %u },\n", lv->name, lv->r);
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_env_stacks(struct os_each_object_data *d, struct obj_list *item)
{
  struct REnv *env = &item->object->env;

  if (env == NULL || env->tt != MRB_TT_ENV)
    return;

  mrb_int len = MRB_ENV_STACK_LEN(env);
  if (len <= 0)
    return;

  fprintf(d->wfile, "PRESET_CONST mrb_value mrb_preset_env_stacks_%d[] = {\n", item->no);

  for (int i = 0; i < len; i++) {
    mrb_value stack = env->stack[i];
    char temp[256];
    const char *tt = get_type_str(mrb_type(stack));

    temp[0] = '\0';
    sprint_value_str(d, temp, sizeof(temp), stack);

    if (tt == NULL)
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = enum_mrb_vtype_%u },\n", temp, mrb_type(stack));
    else
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = %s },\n", temp, tt);
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_array(struct os_each_object_data *d, struct array_list *item)
{
  struct RArray *array = item->array;

  if (ARY_SHARED_P(array)) {
    mrb_shared_array *shared = array->as.heap.aux.shared;

    fprintf(d->wfile, "const mrb_value array_datas_%d[] = {\n", item->no);

    for (int i = 0; i < shared->len; i++) {
      mrb_value value = shared->ptr[i];
      char temp[256];
      const char *tt = get_type_str(mrb_type(value));

      temp[0] = '\0';
      sprint_value_str(d, temp, sizeof(temp), value);

      if (tt == NULL)
        fprintf(d->wfile, "\t{ .value = { %s }, .tt = enum_mrb_vtype_%u },\n", temp, mrb_type(value));
      else
        fprintf(d->wfile, "\t{ .value = { %s }, .tt = %s },\n", temp, tt);
    }

    fprintf(d->wfile, "};\n");
  }
  else {
    fprintf(d->wfile, "const mrb_value array_datas_%d[%d] = {\n", item->no, array->as.heap.aux.capa);

    for (int i = 0; i < array->as.heap.len; i++) {
      mrb_value value = array->as.heap.ptr[i];
      char temp[256];
      const char *tt = get_type_str(mrb_type(value));

      temp[0] = '\0';
      sprint_value_str(d, temp, sizeof(temp), value);

      if (tt == NULL)
        fprintf(d->wfile, "\t{ .value = { %s }, .tt = enum_mrb_vtype_%u },\n", temp, mrb_type(value));
      else
        fprintf(d->wfile, "\t{ .value = { %s }, .tt = %s },\n", temp, tt);
    }

    fprintf(d->wfile, "};\n");
  }
}

static void
print_each_n2s_tbl(mrb_state *mrb, struct os_each_object_data *d)
{
  kh_n2s_t *n2s = (kh_n2s_t *)mrb->name2sym;

  fprintf(d->wfile, "const uint8_t n2s_tbl_ed_flags[] = {\n");

  print_each_uint8(d, n2s->ed_flags, n2s->n_buckets / 4);

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "const mrb_sym n2s_tbl_keys[] = {\n");

  print_each_sym(d, n2s->keys, n2s->n_buckets);

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "const kh_n2sp_t n2s_tbl = { ");

  fprintf(d->wfile, ".n_buckets = %u, ", n2s->n_buckets);
  fprintf(d->wfile, ".size = %u, ", n2s->size);
  fprintf(d->wfile, ".n_occupied = %u, ", n2s->n_occupied);
  fprintf(d->wfile, ".ed_flags = (uint8_t *)&n2s_tbl_ed_flags, ");
  fprintf(d->wfile, ".keys = (mrb_sym *)&n2s_tbl_keys, ");
  fprintf(d->wfile, ".vals = NULL ");

  fprintf(d->wfile, "};\n\n");
}

static void
print_each_iv_tbl(struct os_each_object_data *d, struct iv_tbl_list *item)
{
  kh_iv_t *iv = (kh_iv_t *)item->iv;

  fprintf(d->wfile, "PRESET_CONST uint8_t mrb_preset_iv_tbl_%d_ed_flags[] = {\n", item->no);

  print_each_uint8(d, iv->ed_flags, iv->n_buckets / 4);

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "PRESET_CONST mrb_sym mrb_preset_iv_tbl_%d_keys[] = {\n", item->no);

  print_each_sym(d, iv->keys, iv->n_buckets);

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "PRESET_CONST mrb_value mrb_preset_iv_tbl_%d_vals[] = {\n", item->no);

  for (int i = 0; i < iv->n_buckets; i++) {
    mrb_value val = iv->vals[i];
    char temp[256];
    const char *tt = get_type_str(mrb_type(val));

    temp[0] = '\0';
    sprint_value_str(d, temp, sizeof(temp), val);

    if (tt == NULL)
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = enum_mrb_vtype_%u },\n", temp, mrb_type(val));
    else
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = %s },\n", temp, tt);
  }

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "PRESET_DATA kh_iv_t mrb_preset_iv_tbl_%d = { ", item->no);

  fprintf(d->wfile, ".n_buckets = %u, ", iv->n_buckets);
  fprintf(d->wfile, ".size = %u, ", iv->size);
  fprintf(d->wfile, ".n_occupied = %u, ", iv->n_occupied);
  fprintf(d->wfile, ".ed_flags = (uint8_t *)&mrb_preset_iv_tbl_%d_ed_flags, ", item->no);
  fprintf(d->wfile, ".keys = (mrb_sym *)&mrb_preset_iv_tbl_%d_keys, ", item->no);
  fprintf(d->wfile, ".vals = (mrb_value *)&mrb_preset_iv_tbl_%d_vals ", item->no);

  fprintf(d->wfile, "};\n\n");
}

static void
print_each_kh_mt(struct os_each_object_data *d, struct kh_mt_list *item)
{
  kh_mt_t *mt = (kh_mt_t *)item->mt;

  fprintf(d->wfile, "PRESET_CONST uint8_t mrb_preset_kh_mt_%d_ed_flags[] = {\n", item->no);

  print_each_uint8(d, mt->ed_flags, mt->n_buckets / 4);

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "PRESET_CONST mrb_sym mrb_preset_kh_mt_%d_keys[] = {\n", item->no);

  print_each_sym(d, mt->keys, mt->n_buckets);

  fprintf(d->wfile, "};\n");

  fprintf(d->wfile, "PRESET_CONST mrb_method_t mrb_preset_kh_mt_%d_vals[] = {\n\t", item->no);

  for (int i = 0; i < mt->n_buckets; i++) {
    mrb_method_t val = mt->vals[i];

    if (val.func_p) {
      struct symbol_list *si;

      if (val.func == NULL)
        fprintf(d->wfile, "{ .func_p = 1, { .func = NULL } }, ");
      else if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(val.func))) != NULL) {
        intptr_t offset = OFFSET_FROM_IMAGE_BASE(val.func) - si->addr;
        si->func = 1;
        if (offset == 0)
          fprintf(d->wfile, "{ .func_p = 1, { .func = (mrb_func_t)&%s } }, ", &si->name[0]);
        else
          fprintf(d->wfile, "{ .func_p = 1, { .func = (mrb_func_t)&((uint8_t *)%s)[%d] } }, ", &si->name[0], offset);
      }
      else
        fprintf(d->wfile, "{ .func_p = 1, { .func = (mrb_func_t)0x%p } }, ", (void *)OFFSET_FROM_IMAGE_BASE(val.func));
    }
    else {
      int no;
      if (val.proc == NULL)
        fprintf(d->wfile, "{ .func_p = 0, { .proc = NULL } }, ");
      else if ((no = search_objno(d, (struct RBasic *)val.proc)) >= 0)
        fprintf(d->wfile, "{ .func_p = 0, { .proc = (struct RProc *)&mrb_preset_object_%d.proc } }, ", no);
      else
        fprintf(d->wfile, "{ .func_p = 0, { .proc = (struct RProc *)0x%p } }, ", (void *)OFFSET_FROM_IMAGE_BASE(val.proc));
    }
  }

  fprintf(d->wfile, "\n};\n");

  fprintf(d->wfile, "PRESET_DATA kh_mt_t mrb_preset_kh_mt_%d = { ", item->no);

  fprintf(d->wfile, ".n_buckets = %u, ", mt->n_buckets);
  fprintf(d->wfile, ".size = %u, ", mt->size);
  fprintf(d->wfile, ".n_occupied = %u, ", mt->n_occupied);
  fprintf(d->wfile, ".ed_flags = (uint8_t *)&mrb_preset_kh_mt_%d_ed_flags, ", item->no);
  fprintf(d->wfile, ".keys = (mrb_sym *)&mrb_preset_kh_mt_%d_keys, ", item->no);
  fprintf(d->wfile, ".vals = (mrb_method_t *)&mrb_preset_kh_mt_%d_vals ", item->no);

  fprintf(d->wfile, "};\n\n");
}

static void
print_each_stack(struct os_each_object_data *d, struct stack_list *item)
{
  fprintf(d->wfile, "PRESET_DATA mrb_value mrb_preset_stack_%d[] = { ", item->no);

  for (mrb_value *val = item->base; val < item->end; val++) {
    char temp[256];
    const char *tt = get_type_str(mrb_type(*val));

    temp[0] = '\0';
    sprint_value_str(d, temp, sizeof(temp), *val);

    if (tt == NULL)
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = enum_mrb_vtype_%u },\n", temp, mrb_type(*val));
    else
      fprintf(d->wfile, "\t{ .value = { %s }, .tt = %s },\n", temp, tt);
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_callinfo(struct os_each_object_data *d, struct callinfo_list *item)
{
  struct stack_list *st;

  fprintf(d->wfile, "PRESET_DATA mrb_callinfo mrb_preset_callinfo_%d[] = {\n", item->no);

  for (mrb_callinfo *ci = item->base; ci < item->end; ci++) {
    int no;
    struct symbol_list *si;

    fprintf(d->wfile, "\t{ .mid = (mrb_sym)%d, ", ci->mid);
    if (ci->proc == NULL)
      fprintf(d->wfile, ".proc = NULL, ");
    else if ((no = search_objno(d, (struct RBasic *)ci->proc)) >= 0)
      fprintf(d->wfile, ".proc = (struct RProc *)&mrb_preset_object_%d.proc, ", no);
    else
      fprintf(d->wfile, ".proc = (struct RProc *)0x%p, ", ci->proc);
    st = search_stackno(d, ci->stackent);
    if (ci->stackent == NULL)
      fprintf(d->wfile, ".stackent = NULL, ");
    else if (st != NULL)
      fprintf(d->wfile, ".stackent = (mrb_value *)&mrb_preset_stack_%d[%d], ", st->no, ((intptr_t)ci->stackent - (intptr_t)st->base) / sizeof(mrb_value));
    else
      fprintf(d->wfile, ".stackent = (mrb_value *)0x%p, ", ci->stackent);
    fprintf(d->wfile, ".nregs = %d, ", ci->nregs);
    fprintf(d->wfile, ".ridx = %d, ", ci->ridx);
    fprintf(d->wfile, ".epos = %d, ", ci->epos);
    if (ci->env == NULL)
      fprintf(d->wfile, ".env = NULL, ");
    else if ((no = search_objno(d, (struct RBasic *)ci->env)) >= 0)
      fprintf(d->wfile, ".env = (struct REnv *)&mrb_preset_object_%d.env, ", no);
    else
      fprintf(d->wfile, ".env = (struct REnv *)0x%p, ", ci->env);
    if (ci->pc == NULL)
      fprintf(d->wfile, ".pc = NULL, ");
    else if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(ci->pc))) != NULL) {
      int offset = OFFSET_FROM_IMAGE_BASE(ci->pc) - si->addr;
      si->data = 1;
      if (offset == 0)
        fprintf(d->wfile, ".pc = (mrb_code *)&%s, ", &si->name[0]);
      else
        fprintf(d->wfile, ".pc = (mrb_code *)&((uint8_t *)%s)[%d], ", &si->name[0], offset);
    }
    else
      fprintf(d->wfile, ".pc = (mrb_code *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(ci->pc));
    if (ci->err == NULL)
      fprintf(d->wfile, ".err = NULL, ");
    else if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(ci->err))) != NULL) {
      int offset = OFFSET_FROM_IMAGE_BASE(ci->err) - si->addr;
      si->data = 1;
      if (offset == 0)
        fprintf(d->wfile, ".err = (mrb_code *)&%s, ", &si->name[0]);
      else
        fprintf(d->wfile, ".err = (mrb_code *)&((uint8_t *)%s)[%d], ", &si->name[0], offset);
    }
    else
      fprintf(d->wfile, ".err = (mrb_code *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(ci->err));
    fprintf(d->wfile, ".argc = %d, ", ci->argc);
    fprintf(d->wfile, ".acc = %d, ", ci->acc);
    if (ci->target_class == NULL)
      fprintf(d->wfile, ".target_class = NULL },\n");
    else if ((no = search_objno(d, (struct RBasic *)ci->target_class)) >= 0)
      fprintf(d->wfile, ".target_class = (struct RClass *)&mrb_preset_object_%d.klass },\n", no);
    else
      fprintf(d->wfile, ".target_class = (struct RClass *)0x%p },\n", ci->target_class);
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_context(struct os_each_object_data *d, struct context_list *item)
{
  struct mrb_context *ctx = (struct mrb_context *)item->context;
  struct stack_list *st = search_stackno(d, ctx->stbase);
  struct callinfo_list *ci = search_callinfono(d, ctx->cibase);
  int no;

  fprintf(d->wfile, "PRESET_DATA struct mrb_context mrb_preset_context_%d = {\n", item->no);

  fprintf(d->wfile, "\t.prev = (struct mrb_context *)0x%p,\n", ctx->prev);
  if (ctx->stack == NULL)
    fprintf(d->wfile, "\t.stack = NULL,\n");
  else if (st != NULL)
    fprintf(d->wfile, "\t.stack = (mrb_value *)&mrb_preset_stack_%d[%d],\n", st->no, ((intptr_t)ctx->stack - (intptr_t)st->base) / sizeof(mrb_value));
  else
    fprintf(d->wfile, "\t.stack = (mrb_value *)0x%p,\n", ctx->stack);
  if (ctx->stbase == NULL)
    fprintf(d->wfile, "\t.stbase = NULL,\n");
  else if (st != NULL)
    fprintf(d->wfile, "\t.stbase = (mrb_value *)mrb_preset_stack_%d,\n", st->no);
  else
    fprintf(d->wfile, "\t.stbase = (mrb_value *)0x%p,\n", ctx->stbase);
  if (ctx->stend == NULL)
    fprintf(d->wfile, "\t.stend = NULL,\n");
  else if (st != NULL)
    fprintf(d->wfile, "\t.stend = (mrb_value *)&mrb_preset_stack_%d[%d],\n", st->no, ((intptr_t)ctx->stend - (intptr_t)st->base) / sizeof(mrb_value));
  else
    fprintf(d->wfile, "\t.stend = (mrb_value *)0x%p,\n", ctx->stend);
  if (ctx->ci == NULL)
    fprintf(d->wfile, "\t.ci = NULL,\n");
  else if (ci != NULL)
    fprintf(d->wfile, "\t.ci = (mrb_callinfo *)&mrb_preset_callinfo_%d[%d],\n", ci->no, ((intptr_t)ctx->ci - (intptr_t)ci->base) / sizeof(mrb_callinfo));
  else
    fprintf(d->wfile, "\t.ci = (mrb_callinfo *)0x%p,\n", ctx->ci);
  if (ctx->cibase == NULL)
    fprintf(d->wfile, "\t.cibase = NULL,\n");
  else if (ci != NULL)
    fprintf(d->wfile, "\t.cibase = (mrb_callinfo *)mrb_preset_callinfo_%d,\n", ci->no);
  else
    fprintf(d->wfile, "\t.cibase = (mrb_callinfo *)0x%p,\n", ctx->cibase);
  if (ctx->ciend == NULL)
    fprintf(d->wfile, "\t.ciend = NULL,\n");
  else if (ci != NULL)
    fprintf(d->wfile, "\t.ciend = (mrb_callinfo *)&mrb_preset_callinfo_%d[%d],\n", ci->no, ((intptr_t)ctx->ciend - (intptr_t)ci->base) / sizeof(mrb_callinfo));
  else
    fprintf(d->wfile, "\t.ciend = (mrb_callinfo *)0x%p,\n", ctx->ciend);
  fprintf(d->wfile, "\t.rescue = (mrb_code **)0x%p,\n", ctx->rescue);
  fprintf(d->wfile, "\t.rsize = %d,\n", ctx->rsize);
  fprintf(d->wfile, "\t.ensure = (struct RProc **)0x%p,\n", ctx->ensure);
  fprintf(d->wfile, "\t.esize = %d, .eidx = %d,\n", ctx->esize, ctx->eidx);
  fprintf(d->wfile, "\t.status = (enum mrb_fiber_state)%d,\n", ctx->status);
  fprintf(d->wfile, "\t.vmexec = (mrb_bool)%d,\n", ctx->vmexec);
  if (ctx->fib == NULL)
    fprintf(d->wfile, "\t.fib = NULL,\n");
  else if ((no = search_objno(d, (struct RBasic *)ctx->fib)) >= 0)
    fprintf(d->wfile, "\t.fib = (struct RFiber *)&mrb_preset_object_%d.fiber,\n", no);
  else
    fprintf(d->wfile, "\t.fib = (struct RFiber *)0x%p,\n", ctx->fib);

  fprintf(d->wfile, "};\n");
}

static void
print_each_debug_info_line(struct os_each_object_data *d, struct debug_info_file_list *item)
{
  mrb_irep_debug_info_file *file = item->debug_info_file;
  uint8_t *data = (uint8_t *)file->lines.ptr;
  int len = 0;

  switch (file->line_type) {
  case mrb_debug_line_ary:
    len += sizeof(uint16_t) * (size_t)(file->line_entry_count);
    break;
  case mrb_debug_line_flat_map:
    len += (sizeof(uint32_t) + sizeof(uint16_t)) * (size_t)(file->line_entry_count);
    break;
  default:
    break;
  }

  if (len == 0)
    return;

  fprintf(d->wfile, "PRESET_CONST uint8_t mrb_preset_debug_info_lines_%d[] = {\n", item->no);

  print_each_uint8(d, data, len);

  fprintf(d->wfile, "};\n");
}

static void
print_each_debug_info_file(struct os_each_object_data *d, struct debug_info_file_list *item)
{
  mrb_irep_debug_info_file *debug_info_file = item->debug_info_file;
  int no;

  fprintf(d->wfile, "PRESET_CONST mrb_irep_debug_info_file mrb_preset_debug_info_file_%d = { ", item->no);

  fprintf(d->wfile, ".start_pos = %u, ", debug_info_file->start_pos);
  if (debug_info_file->filename == NULL) {
    fprintf(d->wfile, ".filename = NULL, ");
  }
  else if ((no = search_stringno(d, debug_info_file->filename, strlen(debug_info_file->filename) + 1)) >= 0) {
    fprintf(d->wfile, ".filename = (const char *)&mrb_preset_strings_%d, ", no);
  }
  else {
    char *str = escape_string(debug_info_file->filename, strlen(debug_info_file->filename) + 1);
    fprintf(d->wfile, ".filename = \"%s\", ", str);
    free(str);
  }
  fprintf(d->wfile, ".filename_sym = %d, ", debug_info_file->filename_sym);
  fprintf(d->wfile, ".line_entry_count = %u, ", debug_info_file->line_entry_count);

  int len = 0;
  switch (debug_info_file->line_type) {
  case mrb_debug_line_ary:
    len += sizeof(uint16_t) * (size_t)(debug_info_file->line_entry_count);
    fprintf(d->wfile, ".line_type = mrb_debug_line_ary, ");
    break;
  case mrb_debug_line_flat_map:
    len += (sizeof(uint32_t) + sizeof(uint16_t)) * (size_t)(debug_info_file->line_entry_count);
    fprintf(d->wfile, ".line_type = mrb_debug_line_flat_map, ");
    break;
  default:
    fprintf(d->wfile, ".line_type = %d, ", debug_info_file->line_type);
    break;
  }

  if (len > 0)
    fprintf(d->wfile, ".lines = { .ptr = (void *)&mrb_preset_debug_info_lines_%d, }, ", item->no);
  else
    fprintf(d->wfile, ".lines = { .ptr = NULL, }, ");

  fprintf(d->wfile, " };\n");
}

static void
print_each_debug_info_files(struct os_each_object_data *d, struct debug_info_list *item)
{
  mrb_irep_debug_info *debug_info = item->debug_info;

  fprintf(d->wfile, "PRESET_CONST mrb_irep_debug_info_file *const mrb_preset_debug_info_files_%d[] = { ", item->no);

  for (int i = 0; i < debug_info->flen; i++) {
    int no;
    if (debug_info->files[i] == NULL)
      fprintf(d->wfile, "NULL, ");
    else if ((no = search_debug_info_file_no(d, debug_info->files[i])) >= 0)
      fprintf(d->wfile, "(mrb_irep_debug_info_file *)&mrb_preset_debug_info_file_%d, ", no);
    else
      fprintf(d->wfile, "(mrb_irep_debug_info_file *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(debug_info->files[i]));
  }

  fprintf(d->wfile, "};\n");
}

static void
print_each_debug_info(struct os_each_object_data *d, struct debug_info_list *item)
{
  mrb_irep_debug_info *debug_info = item->debug_info;

  fprintf(d->wfile, "PRESET_CONST mrb_irep_debug_info mrb_preset_debug_info_%d = { ", item->no);

  fprintf(d->wfile, ".pc_count = %u, ", debug_info->pc_count);
  fprintf(d->wfile, ".flen = %u, ", debug_info->flen);

  if (debug_info->files == NULL)
    fprintf(d->wfile, ".files = NULL };\n");
  else
    fprintf(d->wfile, ".files = (mrb_irep_debug_info_file **)&mrb_preset_debug_info_files_%d };\n", item->no);
}

static void
print_each_irep(struct os_each_object_data *d, struct irep_list *item)
{
  mrb_irep *irep = item->irep;
  int no;

  fprintf(d->wfile, "PRESET_CONST mrb_irep mrb_preset_irep_%d = {\n", item->no);

  fprintf(d->wfile, "\t.nlocals = %u,\n", irep->nlocals);
  fprintf(d->wfile, "\t.nregs = %u,\n", irep->nregs);
  fprintf(d->wfile, "\t.flags = 0x%x,\n", irep->flags);

  struct symbol_list *si;

  if (irep->iseq == NULL)
    fprintf(d->wfile, "\t.iseq = NULL,\n");
  else if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(irep->iseq))) != NULL) {
    int offset = OFFSET_FROM_IMAGE_BASE(irep->iseq) - si->addr;
    si->data = 1;
    if (offset == 0)
      fprintf(d->wfile, "\t.iseq = (mrb_code *)&%s,\n", &si->name[0]);
    else
      fprintf(d->wfile, "\t.iseq = (mrb_code *)&((uint8_t *)%s)[%d],\n", &si->name[0], offset);
  }
  else
    fprintf(d->wfile, "\t.iseq = (mrb_code *)0x%p,\n", (void *)OFFSET_FROM_IMAGE_BASE(irep->iseq));

  if (irep->pool == NULL)
    fprintf(d->wfile, "\t.pool = NULL,\n");
  else
    fprintf(d->wfile, "\t.pool = (mrb_value *)&mrb_preset_irep_%d_pool,\n", item->no);
  if (irep->syms == NULL)
    fprintf(d->wfile, "\t.syms = NULL,\n");
  else
    fprintf(d->wfile, "\t.syms = (mrb_sym *)&mrb_preset_irep_%d_syms,\n", item->no);
  if (irep->reps == NULL)
    fprintf(d->wfile, "\t.reps = NULL,\n");
  else
    fprintf(d->wfile, "\t.reps = (struct mrb_irep **)&mrb_preset_irep_%d_reps,\n", item->no);
  if (irep->lv == NULL)
    fprintf(d->wfile, "\t.lv = NULL,\n");
  else
    fprintf(d->wfile, "\t.lv = (struct mrb_locals *)&mrb_preset_irep_%d_lvs,\n", item->no);
  fprintf(d->wfile, "\t.own_filename = (mrb_bool)%d,\n", irep->own_filename);
  if (irep->filename == NULL) {
    fprintf(d->wfile, "\t.filename = NULL,\n");
  }
  else if ((no = search_stringno(d, irep->filename, strlen(irep->filename) + 1)) >= 0) {
    fprintf(d->wfile, "\t.filename = (char *)&mrb_preset_strings_%d } }, ", no);
  }
  else {
    char *str = escape_string(irep->filename, strlen(irep->filename) + 1);
    fprintf(d->wfile, "\t.filename = %s,\n", str);
    free(str);
  }
  if (irep->lines == NULL)
    fprintf(d->wfile, "\t.lines = NULL,\n");
  else {
    fprintf(d->wfile, "\t.lines = (uint16_t *)0x%p,\n", OFFSET_FROM_IMAGE_BASE(irep->lines));
  }
  if (irep->debug_info == NULL)
    fprintf(d->wfile, "\t.debug_info = NULL,\n");
  else if ((no = search_debug_info_no(d, irep->debug_info)) >= 0)
    fprintf(d->wfile, "\t.debug_info = (struct mrb_irep_debug_info *)&mrb_preset_debug_info_%d,\n", no);
  else
    fprintf(d->wfile, "\t.debug_info = (struct mrb_irep_debug_info *)0x%p,\n", (void *)OFFSET_FROM_IMAGE_BASE(irep->debug_info));
  fprintf(d->wfile, "\t.ilen = %u, .plen = %u, .slen = %u, .rlen = %u, .refcnt = %u\n};\n",
    irep->ilen, irep->plen, irep->slen, irep->rlen, irep->refcnt);
}

static void
print_each_object_cb(struct os_each_object_data *d, struct obj_list *item)
{
  RVALUE *val = item->object;
  struct RBasic *obj = &val->basic;
  struct symbol_list *si;

  if ((item->object->basic.tt != MRB_TT_DATA)
    && (item->object->basic.tt != MRB_TT_ENV)
    && (item->object->basic.tt != MRB_TT_ISTRUCT))
    fprintf(d->wfile, "PRESET_CONST RVALUE mrb_preset_object_%d = { ", item->no);
  else
    fprintf(d->wfile, "PRESET_DATA RVALUE mrb_preset_object_%d = { ", item->no);

  /* tt */
  switch (obj->tt) {
  case MRB_TT_FALSE:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_FALSE, ");
    break;
  case MRB_TT_FREE:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_FREE, ");
    break;
  case MRB_TT_TRUE:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_TRUE, ");
    break;
  case MRB_TT_FIXNUM:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_FIXNUM, ");
    break;
  case MRB_TT_SYMBOL:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_SYMBOL, ");
    break;
  case MRB_TT_UNDEF:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_UNDEF, ");
    break;
  case MRB_TT_FLOAT:
    fprintf(d->wfile, ".floatv = { .tt = MRB_TT_FLOAT, ");
    break;
  case MRB_TT_CPTR:
    fprintf(d->wfile, ".cptr = { .tt = MRB_TT_CPTR, ");
    break;
  case MRB_TT_OBJECT:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_OBJECT, ");
    break;
  case MRB_TT_CLASS:
    fprintf(d->wfile, ".klass = { .tt = MRB_TT_CLASS, ");
    break;
  case MRB_TT_MODULE:
    fprintf(d->wfile, ".klass = { .tt = MRB_TT_MODULE, ");
    break;
  case MRB_TT_ICLASS:
    fprintf(d->wfile, ".klass = { .tt = MRB_TT_ICLASS, ");
    break;
  case MRB_TT_SCLASS:
    fprintf(d->wfile, ".klass = { .tt = MRB_TT_SCLASS, ");
    break;
  case MRB_TT_PROC:
    fprintf(d->wfile, ".proc = { .tt = MRB_TT_PROC, ");
    break;
  case MRB_TT_ARRAY:
    fprintf(d->wfile, ".array = { .tt = MRB_TT_ARRAY, ");
    break;
  case MRB_TT_HASH:
    fprintf(d->wfile, ".hash = { .tt = MRB_TT_HASH, ");
    break;
  case MRB_TT_STRING:
    fprintf(d->wfile, ".string = { .tt = MRB_TT_STRING, ");
    break;
  case MRB_TT_RANGE:
    fprintf(d->wfile, ".range = { .tt = MRB_TT_RANGE, ");
    break;
  case MRB_TT_EXCEPTION:
    fprintf(d->wfile, ".exc = { .tt = MRB_TT_EXCEPTION, ");
    break;
  case MRB_TT_FILE:
    fprintf(d->wfile, ".object = { .tt = MRB_TT_FILE, ");
    break;
  case MRB_TT_ENV:
    fprintf(d->wfile, ".env = { .tt = MRB_TT_ENV, ");
    break;
  case MRB_TT_DATA:
    fprintf(d->wfile, ".data = { .tt = MRB_TT_DATA, ");
    break;
  case MRB_TT_FIBER:
    fprintf(d->wfile, ".fiber = { .tt = MRB_TT_FIBER, ");
    break;
  case MRB_TT_ISTRUCT:
    fprintf(d->wfile, ".istruct = { .tt = MRB_TT_ISTRUCT, ");
    break;
  case MRB_TT_BREAK:
    fprintf(d->wfile, ".brk = { .tt = MRB_TT_BREAK, ");
    break;
  default:
    break;
  }

  fprintf(d->wfile, ".color = %d, ", obj->color);
  fprintf(d->wfile, ".flags = 0x%x, ", obj->flags);
  int no;
  if (obj->c == NULL)
    fprintf(d->wfile, ".c = NULL, ");
  else if ((no = search_objno(d, (struct RBasic *)obj->c)) >= 0)
    fprintf(d->wfile, ".c = (struct RClass *)&mrb_preset_object_%d.klass, ", no);
  else
    fprintf(d->wfile, ".c = (struct RClass *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(obj->c));
#if 0
  if (obj->gcnext == NULL)
    fprintf(d->wfile, ".gcnext = NULL, ");
  else if ((no = search_objno(d, (struct RBasic *)obj->gcnext)) >= 0)
    fprintf(d->wfile, ".gcnext = (struct RBasic *)&mrb_preset_object_%d.basic, ", no);
  else
    fprintf(d->wfile, ".gcnext = (struct RBasic *)0x%p, ", OFFSET_FROM_IMAGE_BASE(obj->gcnext));
#else
  if ((item->gcnext == NULL) || (item->gcnext->no == 0))
    fprintf(d->wfile, ".gcnext = NULL, ");
  else if ((no = item->gcnext->no) >= 0)
    fprintf(d->wfile, ".gcnext = (struct RBasic *)&mrb_preset_object_%d.basic, ", no);
  else
    fprintf(d->wfile, ".gcnext = (struct RBasic *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(item->gcnext));
#endif
  switch (obj->tt) {
  case MRB_TT_FALSE:
    break;
  case MRB_TT_FREE:
    break;
  case MRB_TT_TRUE:
    break;
  case MRB_TT_FIXNUM:
    break;
  case MRB_TT_SYMBOL:
    break;
  case MRB_TT_UNDEF:
    break;
  case MRB_TT_FLOAT:
    fprintf(d->wfile, ".f = %.16e, ", val->floatv.f);
    break;
  case MRB_TT_CPTR:
    fprintf(d->wfile, ".p = (void *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->cptr.p));
    break;
  case MRB_TT_OBJECT:
    if (val->object.iv == NULL)
      fprintf(d->wfile, ".iv = NULL, ");
    else if ((no = search_iv_tblno(d, val->object.iv)) >= 0)
      fprintf(d->wfile, ".iv = (struct iv_tbl *)&mrb_preset_iv_tbl_%d, ", no);
    else
      fprintf(d->wfile, ".iv = (struct iv_tbl *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->object.iv));
    break;
  case MRB_TT_CLASS:
  case MRB_TT_MODULE:
  case MRB_TT_ICLASS:
  case MRB_TT_SCLASS:
    if (val->klass.iv == NULL)
      fprintf(d->wfile, ".iv = NULL, ");
    else if ((no = search_iv_tblno(d, val->klass.iv)) >= 0)
      fprintf(d->wfile, ".iv = (struct iv_tbl *)&mrb_preset_iv_tbl_%d, ", no);
    else
      fprintf(d->wfile, ".iv = (struct iv_tbl *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->klass.iv));
    if (val->klass.mt == NULL)
      fprintf(d->wfile, ".mt = NULL, ");
    else if ((no = search_kh_mtno(d, val->klass.mt)) >= 0)
      fprintf(d->wfile, ".mt = (struct kh_mt *)&mrb_preset_kh_mt_%d, ", no);
    else
      fprintf(d->wfile, ".mt = (struct kh_mt *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->klass.mt));
    if (val->klass.super == NULL)
      fprintf(d->wfile, ".super = NULL, ");
    else if ((no = search_objno(d, (struct RBasic *)val->klass.super)) >= 0)
      fprintf(d->wfile, ".super = (struct RClass *)&mrb_preset_object_%d.klass, ", no);
    else
      fprintf(d->wfile, ".super = (struct RClass *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->klass.super));
    break;
  case MRB_TT_PROC:
    if (MRB_PROC_CFUNC_P(obj)) {
      if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(val->proc.body.func))) != NULL) {
        int offset = OFFSET_FROM_IMAGE_BASE(val->proc.body.func) - si->addr;
        si->func = 1;
        if (offset == 0)
          fprintf(d->wfile, ".body = { .func = (mrb_func_t)%s }, ", &si->name[0]);
        else
          fprintf(d->wfile, ".body = { .func = (mrb_func_t)&((uint8_t *)%s)[%d] }, ", &si->name[0], offset);
      }
      else
        fprintf(d->wfile, ".body = { .func = (mrb_func_t)0x%p }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->proc.body.func));
    }
    else {
      no = search_irepno(d, val->proc.body.irep);
      if (no >= 0)
        fprintf(d->wfile, ".body = { .irep = (mrb_irep *)&mrb_preset_irep_%d }, ", no);
      else
        fprintf(d->wfile, ".body = { .irep = (mrb_irep *)0x%p }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->proc.body.irep));
    }
    if (val->proc.upper == NULL)
      fprintf(d->wfile, ".upper = NULL, ");
    else if ((no = search_objno(d, (struct RBasic *)val->proc.e.env)) >= 0)
      fprintf(d->wfile, ".upper = (struct RProc *)&mrb_preset_object_%d.proc, ", no);
    else
      fprintf(d->wfile, ".upper = (struct RProc *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->proc.e.env));
    if (val->proc.e.target_class == NULL)
      fprintf(d->wfile, ".e = { .target_class = NULL }, ");
    else if (val->proc.e.target_class->tt != MRB_TT_ENV) {
      if ((no = search_objno(d, (struct RBasic *)val->proc.e.target_class)) >= 0)
        fprintf(d->wfile, ".e = { .target_class = (struct RClass *)&mrb_preset_object_%d.klass }, ", no);
      else
        fprintf(d->wfile, ".e = { .target_class = (struct RClass *)0x%p }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->proc.e.target_class));
    }
    else {
      if ((no = search_objno(d, (struct RBasic *)val->proc.e.env)) >= 0)
        fprintf(d->wfile, ".e = { .env = (struct REnv *)&mrb_preset_object_%d.env }, ", no);
      else
        fprintf(d->wfile, ".e = { .env = (struct REnv *)0x%p }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->proc.e.target_class));
    }
    break;
  case MRB_TT_ARRAY:
    fprintf(d->wfile, ".as = { .heap = { .len = %d } }, ", val->array.as.heap.len);
    if (ARY_SHARED_P(obj))
      fprintf(d->wfile, ".as = { .heap = { .aux = { .shared = (mrb_shared_array *)0x%p } } } }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->array.as.heap.aux.shared));
    else
      fprintf(d->wfile, "{ .capa = %d }, ", val->array.as.heap.aux.capa);
    if (val->array.as.heap.ptr == NULL)
      fprintf(d->wfile, ".as = { .heap = { .ptr = NULL } }, ");
    else if ((no = search_arrayno(d, &val->array)) >= 0)
      fprintf(d->wfile, ".as = { .heap = { .ptr = (mrb_value *)&array_datas_%d } }, ", no);
    else
      fprintf(d->wfile, ".as = { .heap = { .ptr = (mrb_value *)0x%p } }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->array.as.heap.ptr));
    break;
  case MRB_TT_HASH:
    if (val->hash.iv == NULL)
      fprintf(d->wfile, ".iv = NULL, ");
    else if ((no = search_iv_tblno(d, val->hash.iv)) >= 0)
      fprintf(d->wfile, ".iv = &mrb_preset_iv_tbl_%d, ", no);
    else
      fprintf(d->wfile, ".iv = (struct iv_tbl *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->hash.iv));
    fprintf(d->wfile, ".ht = (struct kh_ht *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->hash.ht));
    break;
  case MRB_TT_STRING:
    if (RSTR_EMBED_P(obj)) {
      char *str = escape_string(val->string.as.ary, strlen(val->string.as.ary) + 1);
      fprintf(d->wfile, ".as = { .ary = \"%s\" }", str);
      free(str);
    }
    else {
      fprintf(d->wfile, ".as = { .heap = { .len = %d,", val->string.as.heap.len);
      if (RSTR_SHARED_P(obj))
        fprintf(d->wfile, ".aux = { .shared = (struct mrb_shared_string *)0x%p }, ", (void *)OFFSET_FROM_IMAGE_BASE(val->string.as.heap.aux.shared));
      else
        fprintf(d->wfile, ".aux = { .capa = %d }, ", val->string.as.heap.aux.capa);
      if ((val->string.as.heap.ptr == NULL) || (val->string.as.heap.len <= 0)) {
        fprintf(d->wfile, ".ptr = NULL } }, ");
      }
      else if ((no = search_stringno(d, val->string.as.heap.ptr, val->string.as.heap.len)) >= 0) {
        fprintf(d->wfile, ".ptr = (char *)&mrb_preset_strings_%d } }, ", no);
      }
      else {
        char *str = escape_string(val->string.as.heap.ptr, val->string.as.heap.len);
        fprintf(d->wfile, ".ptr = \"%s\" } }, ", str);
        free(str);
      }
    }
    break;
  case MRB_TT_RANGE:
    fprintf(d->wfile, ".edges = (mrb_range_edges *)0x%p, .excl = %d, ", (void *)OFFSET_FROM_IMAGE_BASE(val->range.edges), val->range.excl);
    break;
  case MRB_TT_EXCEPTION:
    if (val->exc.iv == NULL)
      fprintf(d->wfile, ".iv = NULL, ");
    else if ((no = search_iv_tblno(d, val->exc.iv)) >= 0)
      fprintf(d->wfile, ".iv = (struct iv_tbl *)&mrb_preset_iv_tbl_%d, ", no);
    else
      fprintf(d->wfile, ".iv = (struct iv_tbl *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->exc.iv));
    break;
  case MRB_TT_FILE:
    break;
  case MRB_TT_ENV:
    fprintf(d->wfile, ".stack = (mrb_value *)&mrb_preset_env_stacks_%d, ", item->no);
    if (val->env.cxt == NULL)
      fprintf(d->wfile, ".cxt = NULL, ");
    else if ((no = search_contextno(d, val->env.cxt)) >= 0)
      fprintf(d->wfile, ".cxt = (struct mrb_context *)&mrb_preset_context_%d, ", no);
    else
      fprintf(d->wfile, ".cxt = (struct mrb_context *)0x%p, ", val->env.cxt);
    fprintf(d->wfile, ".mid = (mrb_sym)%d, ", val->env.mid);
    break;
  case MRB_TT_DATA:
    if (val->data.iv == NULL)
      fprintf(d->wfile, ".iv = NULL, ");
    else if ((no = search_iv_tblno(d, val->data.iv)) >= 0)
      fprintf(d->wfile, ".iv = (struct iv_tbl *)&mrb_preset_iv_tbl_%d, ", no);
    else
      fprintf(d->wfile, ".iv = (struct iv_tbl *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->data.iv));
    if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(val->data.type))) != NULL) {
      int offset = OFFSET_FROM_IMAGE_BASE(val->data.type) - si->addr;
      si->data = 1;
      if (offset == 0)
        fprintf(d->wfile, ".type = (const mrb_data_type *)%s, ", &si->name[0]);
      else
        fprintf(d->wfile, ".type = (const mrb_data_type *)&((uint8_t *)%s)[%d], ", &si->name[0], offset);
    }
    else
      fprintf(d->wfile, ".type = (const mrb_data_type *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->data.type));
    if ((si = search_symbol_item(d, OFFSET_FROM_IMAGE_BASE(val->data.data))) != NULL) {
      int offset = OFFSET_FROM_IMAGE_BASE(val->data.data) - si->addr;
      si->data = 1;
      if (offset == 0)
        fprintf(d->wfile, ".data = (void *)%s, ", &si->name[0]);
      else
        fprintf(d->wfile, ".data = (void *)&((uint8_t *)%s)[%d], ", &si->name[0], offset);
    }
    else
      fprintf(d->wfile, ".data = (void *)0x%p, ", (void *)OFFSET_FROM_IMAGE_BASE(val->data.data));
    break;
  case MRB_TT_FIBER:
    val->fiber;
    fprintf(d->wfile, ".fiber = { 0 ");
    break;
  case MRB_TT_ISTRUCT:
    val->istruct;
    fprintf(d->wfile, ".istruct = { 0 ");
    break;
  case MRB_TT_BREAK:
    val->brk;
    fprintf(d->wfile, ".brk = { 0 ");
    break;
  default:
    fprintf(d->wfile, ".tt%u = { 0 ", obj->tt);
    break;
  }

  fprintf(d->wfile, "} };\n");
}

static void
print_preset_object(struct os_each_object_data *d, const char *name, struct RObject* obj)
{
  int no;
  if (obj == NULL)
    fprintf(d->wfile, "\t.%s = NULL,\n", name);
  else if ((no = search_objno(d, (struct RBasic *)obj)) >= 0)
    fprintf(d->wfile, "\t.%s = (struct RObject *)&mrb_preset_object_%d.object,\n", name, no);
  else
    fprintf(d->wfile, "\t.%s = (struct RObject *)0x%p,\n", name, (void *)OFFSET_FROM_IMAGE_BASE(obj));
}

static void
print_preset_class(struct os_each_object_data *d, const char *name, struct RClass* obj)
{
  int no;
  if (obj == NULL)
    fprintf(d->wfile, "\t.%s = NULL,\n", name);
  else if ((no = search_objno(d, (struct RBasic *)obj)) >= 0)
    fprintf(d->wfile, "\t.%s = (struct RClass *)&mrb_preset_object_%d.klass,\n", name, no);
  else
    fprintf(d->wfile, "\t.%s = (struct RClass *)0x%p,\n", name, (void *)OFFSET_FROM_IMAGE_BASE(obj));
}

static void
print_preset_context(struct os_each_object_data *d, const char *name, struct mrb_context* ctx)
{
  int no;
  if (ctx == NULL)
    fprintf(d->wfile, "\t.%s = NULL,\n", name);
  else if ((no = search_contextno(d, ctx)) >= 0)
    fprintf(d->wfile, "\t.%s = (struct mrb_context *)&mrb_preset_context_%d,\n", name, no);
  else
    fprintf(d->wfile, "\t.%s = (struct mrb_context *)0x%p,\n", name, (void *)OFFSET_FROM_IMAGE_BASE(ctx));
}

static void
print_preset_iv(struct os_each_object_data *d, const char *name, struct iv_tbl *iv)
{
  int no;
  if (iv == NULL)
    fprintf(d->wfile, "\t.%s = NULL,\n", name);
  else if ((no = search_iv_tblno(d, iv)) >= 0)
    fprintf(d->wfile, "\t.%s = (struct iv_tbl *)&mrb_preset_iv_tbl_%d,\n", name, no);
  else
    fprintf(d->wfile, "\t.%s = (struct iv_tbl *)0x%p,\n", name, (void *)OFFSET_FROM_IMAGE_BASE(iv));
}

static int
dump(const char *filename, struct dump_args *args)
{
  mrb_state *mrb = mrb_open();
  int ret = 0;
  mrbc_context *c;
  struct os_each_object_data d;
  struct obj_list *objs;
  struct irep_list *ireps;

  if (mrb == NULL) {
    fputs("Invalid mrb_state, exiting mruby\n", stderr);
    return EXIT_FAILURE;
  }

  c = mrbc_context_new(mrb);

  memset(&d, 0, sizeof(d));
  d.mrb = mrb;

  ret = read_symbol_file(&d);
  if (ret != 0) {
    return EXIT_FAILURE;
  }

  if (d.symbols != NULL) {
    struct symbol_list *first = d.symbols->next, *item = first;
    do {
      item->addr -= d.offset;
      item = item->next;
    } while (item != first);
  }

  //mrb_objspace_each_objects(mrb, count_each_object_cb, &d);
  mrb_full_gc(mrb);

  /* class hierarchy */
  add_obj(&d, (struct RBasic*)mrb->object_class);

  /* built-in classes */
  add_obj(&d, (struct RBasic*)mrb->class_class);
  add_obj(&d, (struct RBasic*)mrb->module_class);
  add_obj(&d, (struct RBasic*)mrb->proc_class);
  add_obj(&d, (struct RBasic*)mrb->string_class);
  add_obj(&d, (struct RBasic*)mrb->array_class);
  add_obj(&d, (struct RBasic*)mrb->hash_class);
  add_obj(&d, (struct RBasic*)mrb->range_class);

#ifndef MRB_WITHOUT_FLOAT
  add_obj(&d, (struct RBasic*)mrb->float_class);
#endif
  add_obj(&d, (struct RBasic*)mrb->fixnum_class);
  add_obj(&d, (struct RBasic*)mrb->true_class);
  add_obj(&d, (struct RBasic*)mrb->false_class);
  add_obj(&d, (struct RBasic*)mrb->nil_class);
  add_obj(&d, (struct RBasic*)mrb->symbol_class);
  add_obj(&d, (struct RBasic*)mrb->kernel_module);

  add_obj(&d, (struct RBasic*)mrb->eException_class);
  add_obj(&d, (struct RBasic*)mrb->eStandardError_class);

  /* top_self */
  add_obj(&d, (struct RBasic*)mrb->top_self);
  /* exception */
  add_obj(&d, (struct RBasic*)mrb->exc);
  /* pre-allocated exception */
  add_obj(&d, (struct RBasic*)mrb->nomem_err);
  add_obj(&d, (struct RBasic*)mrb->stack_err);
#ifdef MRB_GC_FIXED_ARENA
  add_obj(&d, (struct RBasic*)mrb->arena_err);
#endif

  add_context(&d, mrb->c);
  add_context(&d, mrb->root_c);

  if (mrb->globals != NULL) {
    add_iv(&d, mrb->globals);
  }

#ifndef _MSC_VER
  d.wfile = fopen(filename, "wb");
  if (d.wfile == NULL)
    return EXIT_FAILURE;
#else
  ret = fopen_s(&d.wfile, filename, "wb");
  if (ret != 0) {
    fprintf(stderr, "open failed. %s file\n", filename);
    return EXIT_FAILURE;
  }
#endif

  objs = d.objs;
  if (objs != NULL) {
    struct obj_list *first = objs->next, *item = first;
    do {
      if ((item->object->basic.tt != MRB_TT_DATA)
        && (item->object->basic.tt != MRB_TT_ENV)
        && (item->object->basic.tt != MRB_TT_ISTRUCT))
        fprintf(d.wfile, "extern PRESET_CONST RVALUE mrb_preset_object_%d;\n", item->no);
      else
        fprintf(d.wfile, "extern PRESET_DATA RVALUE mrb_preset_object_%d;\n", item->no);
      item = item->next;
    } while (item != first);
  }
  fprintf(d.wfile, "\n");

  ireps = d.ireps;
  if (ireps != NULL) {
    struct irep_list *first = ireps->next, *item = first;
    do {
      fprintf(d.wfile, "extern PRESET_CONST mrb_irep mrb_preset_irep_%d;\n", item->no);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  fprintf(d.wfile, "const char *const mrb_preset_symbols[] = {\n");
  for (mrb_sym i = 1, lim = mrb->symidx + 1; i < lim; i++) {
    const char *name = mrb_sym2name(mrb, i);
    // ???
    char *str = escape_string2(name, strlen(name) + 1);
    if (name[0] == '\"') {
      fprintf(d.wfile, "\t%s,\n", str);
    }
    else {
      fprintf(d.wfile, "\t\"%s\",\n", str);
    }
    free(str);
  }
  fprintf(d.wfile, "};\n\n");

  fprintf(d.wfile, "const int mrb_preset_symbols_count = (sizeof(mrb_preset_symbols) / sizeof(mrb_preset_symbols[0]));\n\n");

  struct string_list *strings = d.strings;
  if (strings != NULL) {
    struct string_list *first = strings->next, *item = first;
    do {
      char *str = escape_string(item->string, item->length);
      fprintf(d.wfile, "const char mrb_preset_strings_%d[] = \"%s\";\n", item->no, str);
      free(str);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  print_each_n2s_tbl(mrb, &d);

  ireps = d.ireps;
  if (ireps != NULL) {
    struct irep_list *first = ireps->next, *item = first;
    do {
      print_each_irep_pool(&d, item);
      item = item->next;
    } while (item != first);
    fprintf(d.wfile, "\n");
    item = first;
    do {
      print_each_irep_syms(&d, item);
      item = item->next;
    } while (item != first);
    fprintf(d.wfile, "\n");
    item = first;
    do {
      print_each_irep_reps(&d, item);
      item = item->next;
    } while (item != first);
    fprintf(d.wfile, "\n");
    item = first;
    do {
      print_each_irep_lvs(&d, item);
      item = item->next;
    } while (item != first);
    fprintf(d.wfile, "\n");
  }

  struct debug_info_file_list *debug_info_files = d.debug_info_files;
  if (debug_info_files != NULL) {
    struct debug_info_file_list *first = debug_info_files->next, *item = first;
    do {
      print_each_debug_info_line(&d, item);
      item = item->next;
    } while (item != first);
  }

  if (debug_info_files != NULL) {
    struct debug_info_file_list *first = debug_info_files->next, *item = first;
    do {
      print_each_debug_info_file(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  struct debug_info_list *debug_infos = d.debug_infos;
  if (debug_infos != NULL) {
    struct debug_info_list *first = debug_infos->next, *item = first;
    do {
      print_each_debug_info_files(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  debug_infos = d.debug_infos;
  if (debug_infos != NULL) {
    struct debug_info_list *first = debug_infos->next, *item = first;
    do {
      print_each_debug_info(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  ireps = d.ireps;
  if (ireps != NULL) {
    struct irep_list *first = ireps->next, *item = first;
    do {
      print_each_irep(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  objs = d.objs;
  if (objs != NULL) {
    struct obj_list *first = objs->next, *item = first;
    do {
      print_each_env_stacks(&d, item);
      item = item->next;
    } while (item != first);
  }

  struct array_list *arrays = d.arrays;
  if (arrays != NULL) {
    struct array_list *first = arrays->next, *item = first;
    do {
      print_each_array(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  struct iv_tbl_list *iv_tbls = d.iv_tbls;
  if (iv_tbls != NULL) {
    struct iv_tbl_list *first = iv_tbls->next, *item = first;
    do {
      print_each_iv_tbl(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  struct kh_mt_list *kh_mts = d.kh_mts;
  if (kh_mts != NULL) {
    struct kh_mt_list *first = kh_mts->next, *item = first;
    do {
      print_each_kh_mt(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  struct stack_list *stacks = d.stacks;
  if (stacks != NULL) {
    struct stack_list *first = stacks->next, *item = first;
    do {
      print_each_stack(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  struct callinfo_list *callinfos = d.callinfos;
  if (callinfos != NULL) {
    struct callinfo_list *first = callinfos->next, *item = first;
    do {
      print_each_callinfo(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  struct context_list *contexts = d.contexts;
  if (contexts != NULL) {
    struct context_list *first = contexts->next, *item = first;
    do {
      print_each_context(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  objs = d.objs;
  if (objs != NULL) {
    struct obj_list *first = objs->next, *item = first;
    do {
      print_each_object_cb(&d, item);
      item = item->next;
    } while (item != first);
  }

  fprintf(d.wfile, "\n");

  fprintf(d.wfile, "PRESET_DATA struct mrb_state mrb_preset_state = {\n");
  fprintf(d.wfile, "\t.jmp = NULL,\n");
  fprintf(d.wfile, "\t.flags = 0,\n");
  fprintf(d.wfile, "\t.allocf = NULL,\n");
  fprintf(d.wfile, "\t.allocf_ud = NULL,\n");

  print_preset_context(&d, "c", mrb->c);
  print_preset_context(&d, "root_c", mrb->root_c);

  print_preset_iv(&d, "globals", mrb->globals);

  print_preset_object(&d, "exc", mrb->exc);

  print_preset_object(&d, "top_self", mrb->top_self);
  print_preset_class(&d, "object_class", mrb->object_class);
  print_preset_class(&d, "class_class", mrb->class_class);
  print_preset_class(&d, "module_class", mrb->module_class);
  print_preset_class(&d, "proc_class", mrb->proc_class);
  print_preset_class(&d, "string_class", mrb->string_class);
  print_preset_class(&d, "array_class", mrb->array_class);
  print_preset_class(&d, "hash_class", mrb->hash_class);
  print_preset_class(&d, "range_class", mrb->range_class);

  fprintf(d.wfile, "#ifndef MRB_WITHOUT_FLOAT\n");
#ifndef MRB_WITHOUT_FLOAT
  print_preset_class(&d, "float_class", mrb->float_class);
#endif
  fprintf(d.wfile, "#endif\n");
  print_preset_class(&d, "fixnum_class", mrb->fixnum_class);
  print_preset_class(&d, "true_class", mrb->true_class);
  print_preset_class(&d, "false_class", mrb->false_class);
  print_preset_class(&d, "nil_class", mrb->nil_class);
  print_preset_class(&d, "symbol_class", mrb->symbol_class);
  print_preset_class(&d, "kernel_module", mrb->kernel_module);

  fprintf(d.wfile, "\t.mems = NULL,\n");
  fprintf(d.wfile, "\t.gc = { 0 },\n");

  fprintf(d.wfile, "#ifdef MRB_METHOD_CACHE\n");
#ifdef MRB_METHOD_CACHE
  fprintf(d.wfile, "\t.cache = { 0 },\n");
#else
  fprintf(d.wfile, "\t.cache = { 0 },\n");
#endif
  fprintf(d.wfile, "#endif\n");

  fprintf(d.wfile, "\t.symidx = (sizeof(mrb_preset_symbols) / sizeof(mrb_preset_symbols[0])),\n");
  fprintf(d.wfile, "\t.name2sym = NULL,\n");
  fprintf(d.wfile, "\t.symtbl = NULL,\n");
  fprintf(d.wfile, "\t.symcapa = 0,\n");

  fprintf(d.wfile, "#ifdef MRB_ENABLE_DEBUG_HOOK\n");
#ifdef MRB_ENABLE_DEBUG_HOOK
  fprintf(d.wfile, "\t.code_fetch_hook = NULL,\n");
  fprintf(d.wfile, "\t.debug_op_hook = NULL,\n");
#else
  fprintf(d.wfile, "\t.code_fetch_hook = NULL,\n");
  fprintf(d.wfile, "\t.debug_op_hook = NULL,\n");
#endif
  fprintf(d.wfile, "#endif\n");

  fprintf(d.wfile, "#ifdef MRB_BYTECODE_DECODE_OPTION\n");
#ifdef MRB_BYTECODE_DECODE_OPTION
  fprintf(d.wfile, "\t.bytecode_decoder = NULL,\n");
#else
  fprintf(d.wfile, "\t.bytecode_decoder = NULL,\n");
#endif
  fprintf(d.wfile, "#endif\n");

  print_preset_class(&d, "eException_class", mrb->eException_class);
  print_preset_class(&d, "eStandardError_class", mrb->eStandardError_class);
  print_preset_object(&d, "nomem_err", mrb->nomem_err);
  print_preset_object(&d, "stack_err", mrb->stack_err);
  fprintf(d.wfile, "#ifdef MRB_GC_FIXED_ARENA\n");
#ifdef MRB_GC_FIXED_ARENA
  print_preset_object(&d, "arena_err", mrb->arena_err);
#else
  fprintf(d.wfile, "\t.arena_err = NULL,\n");
#endif
  fprintf(d.wfile, "#endif\n");
  fprintf(d.wfile, "\t.ud = NULL,\n");
  fprintf(d.wfile, "#ifdef MRB_FIXED_STATE_ATEXIT_STACK\n");
#ifdef MRB_FIXED_STATE_ATEXIT_STACK
  fprintf(d.wfile, "\t.atexit_stack = { 0 },\n");
#else
  fprintf(d.wfile, "\t.atexit_stack = NULL,\n");
#endif
  fprintf(d.wfile, "#endif\n");
  fprintf(d.wfile, "\t.atexit_stack_len = 0\n");
  fprintf(d.wfile, "};\n\n");

  struct symbol_list *symbols = d.symbols;
  if (symbols != NULL) {
    struct symbol_list *first = symbols->next, *item = first;
    do {
      if (item->data != 0)
        fprintf(d.wfile, "extern uint8_t %s[];\n", &item->name[0]);
      else if (item->func != 0)
        fprintf(d.wfile, "extern mrb_value %s(mrb_state *mrb, mrb_value self);\n", &item->name[0]);
      item = item->next;
    } while (item != first);
  }

  fclose(d.wfile);

  clear_queue((struct queue_t **)&d.symbols);
  clear_queue((struct queue_t **)&d.strings);
  clear_queue((struct queue_t **)&d.objs);
  clear_queue((struct queue_t **)&d.ireps);
  clear_queue((struct queue_t **)&d.debug_infos);
  clear_queue((struct queue_t **)&d.debug_info_files);
  clear_queue((struct queue_t **)&d.arrays);
  clear_queue((struct queue_t **)&d.iv_tbls);
  clear_queue((struct queue_t **)&d.kh_mts);
  clear_queue((struct queue_t **)&d.contexts);
  clear_queue((struct queue_t **)&d.callinfos);

  mrbc_context_free(mrb, c);
  if (mrb->exc) {
    mrb_print_error(mrb);
    ret = -1;
  }

  return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void
print_usage(const char *f)
{
  //printf("Usage: %s [switches] outfile\n", f);
  //printf("switches:\n");
  //printf("  -l, --lvar   remove LVAR section too.\n");
  printf("Usage: %s outfile\n", f);
}

static int
parse_args(int argc, char **argv, struct dump_args *args)
{
  int i;

  args->argc_start = 0;
  args->argc = argc;
  args->argv = argv;
  args->lvar = FALSE;

  for (i = 1; i < argc; ++i) {
    const size_t len = strlen(argv[i]);
    if (len >= 2 && argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'l':
        args->lvar = TRUE;
        break;
      case '-':
        if (strncmp((*argv) + 2, "lvar", len) == 0) {
          args->lvar = TRUE;
          break;
        }
      default:
        return -1;
      }
    }
    else {
      break;
    }
  }

  args->argc_start = i;
  return i;
}

void save_syms_path(char *cmd)
{
  syms_path[0] = '\0';

#ifndef _MSC_VER
  realpath(cmd, syms_path);
#else
  _fullpath(syms_path, cmd, sizeof(syms_path));
#endif

  for (int i = strlen(syms_path) - 1; i >= 0; i--) {
    if (syms_path[i] == '.') {
       syms_path[i] = '\0';
       break;
    }
    if (syms_path[i] == '\\')
      break;
}

#ifndef _MSC_VER
  strcat(syms_path, ".syms");
#else
  strcat_s(syms_path, sizeof(syms_path), ".syms");
#endif
}

int
main(int argc, char **argv)
{
  struct dump_args args;
  int args_result;
  int ret;

  save_syms_path(argv[0]);

  if (argc <= 1) {
    printf("no files to output file\n");
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  args_result = parse_args(argc, argv, &args);
  if (args_result < 0) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  ret = dump(argv[1], &args);

  return ret;
}
