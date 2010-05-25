/*
 * kernel-mode quine
 * Copyright (C) 2010 Yusuke Endoh
 *
 * usage: (NOTE THAT THERE IS NO WARRANTY FOR THE PROGRAM)
 *   1) build and load quine.ko
 *     # configure KDIR in Makefile for your linux environment
 *     $ vi Makefile
 *     $ make
 *     $ insmod ./quine.ko
 *     $ less /proc/quine/quine.c
 *
 *   2) check quine
 *     $ mkdir /tmp/quine-build && cd /tmp/quine-build
 *     $ cp /proc/quine/quine.c /proc/quine/Makefile .
 *     $ sudo rmmod quine
 *     $ make
 *     $ sudo insmod ./quine.ko
 *     $ diff quine.c /proc/quine/quine.c
 *     $ diff Makefile /proc/quine/Makefile
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

MODULE_DESCRIPTION("quine");
MODULE_AUTHOR("Yusuke Endoh <mame@tsg.ne.jp>");
MODULE_LICENSE("GPL");

#define S(s) #s
#define XS(s) S(s)

static struct proc_dir_entry *dir;
static char quine_txt[8000];
static char makefile_txt[] = "KDIR = " XS(KDIR) "\nEXTRA_CFLAGS = -D"
  "KDIR=$(KDIR)\nobj-m += quine.o\nall:\n\tmake -C $(KDIR) M=$(PWD) "
  "modules\nclean:\n\tmake -C $(KDIR) M=$(PWD) clean\n";
static char self[] = "/*^ * kernel-mode quine^ * Copyright (C) 2010 "
  "Yusuke Endoh^ *^ * usage: (NOTE THAT THERE IS NO WARRANTY FOR THE"
  " PROGRAM)^ *   1) build and load quine.ko^ *     # configure KDIR"
  " in Makefile for your linux environment^ *     $ vi Makefile^ *  "
  "   $ make^ *     $ insmod ./quine.ko^ *     $ less /proc/quine/qu"
  "ine.c^ *^ *   2) check quine^ *     $ mkdir /tmp/quine-build && c"
  "d /tmp/quine-build^ *     $ cp /proc/quine/quine.c /proc/quine/Ma"
  "kefile .^ *     $ sudo rmmod quine^ *     $ make^ *     $ sudo in"
  "smod ./quine.ko^ *     $ diff quine.c /proc/quine/quine.c^ *     "
  "$ diff Makefile /proc/quine/Makefile^ */^^#include <linux/module."
  "h>^#include <linux/kernel.h>^#include <linux/proc_fs.h>^#include "
  "<linux/init.h>^^MODULE_DESCRIPTION(~quine~);^MODULE_AUTHOR(~Yusuk"
  "e Endoh <mame@tsg.ne.jp>~);^MODULE_LICENSE(~GPL~);^^#define S(s) "
  "#s^#define XS(s) S(s)^^static struct proc_dir_entry *dir;^static "
  "char quine_txt[8000];^static char makefile_txt[] = ~KDIR = ~ XS(K"
  "DIR) ~|nEXTRA_CFLAGS = -D~^  ~KDIR=$(KDIR)|nobj-m += quine.o|nall"
  ":|n|tmake -C $(KDIR) M=$(PWD) ~^  ~modules|nclean:|n|tmake -C $(K"
  "DIR) M=$(PWD) clean|n~;^static char self[] = ~!~;^^static void qu"
  "ine(void)^{^  char ch, *dst = quine_txt, *src = self, *src2 = src"
  ";^  while ((ch = *src++)) {^    switch (ch) {^      case 33:^    "
  "    while ((ch = *src2++)) {^          *dst++ = ch;^          if "
  "((dst - quine_txt + 12) % 70 == 0) {^            int i;^         "
  "   for (i = 0; i < 5; i++) *dst++ = ~|~|n  |~~[i];^          }^  "
  "      }^        break;^      case 126: ch -=  8;^      case  94: "
  "ch -= 52;^      case 124: ch -= 32;^      default:^        *dst++"
  " = ch;^    }^  }^  *dst = 0;^}^^static int quine_read(char *page,"
  " char **start, off_t off,^               int count, int *eof, voi"
  "d *data)^{^  char *txt = data;^  int n = strlen(txt) - off;^  if "
  "(n <= 0) {^    *eof = 1;^    return 0;^  }^  if (n > count) n = c"
  "ount;^  memcpy(page, txt + off, n);^  *start = page;^  return n;^"
  "}^^static int quine_init_module(void)^{^  struct proc_dir_entry *"
  "path;^^  quine();^^  dir = proc_mkdir(~quine~, 0);^  if (dir == N"
  "ULL) {^    printk(~quine: failed to create /proc/quine|n~);^    g"
  "oto err0;^  }^^  path = create_proc_read_entry(^    ~quine.c~, 04"
  "44, dir, quine_read, quine_txt);^  if (path == NULL) {^    printk"
  "(~quine: failed to create /proc/quine/quine.c|n~);^    goto err1;"
  "^  }^^  path = create_proc_read_entry(^    ~Makefile~, 0444, dir,"
  " quine_read, makefile_txt);^  if (path == NULL) {^    printk(~qui"
  "ne: failed to create /proc/quine/Makefile|n~);^    goto err2;^  }"
  "^^  printk(~quine loaded; copy /proc/quine/ and make|n~);^  retur"
  "n 0;^^  err2: remove_proc_entry(~quine.c~, dir);^  err1: remove_p"
  "roc_entry(~quine~, 0); dir = 0;^  err0: return -ENOMEM;^}^^static"
  " void quine_cleanup_module(void)^{^  if (dir) {^    remove_proc_e"
  "ntry(~quine.c~, dir);^    remove_proc_entry(~Makefile~, dir);^  }"
  "^  remove_proc_entry(~quine~, 0);^  printk(~quine unloaded.|n~);^"
  "}^^module_init(quine_init_module);^module_exit(quine_cleanup_modu"
  "le);^";

static void quine(void)
{
  char ch, *dst = quine_txt, *src = self, *src2 = src;
  while ((ch = *src++)) {
    switch (ch) {
      case 33:
        while ((ch = *src2++)) {
          *dst++ = ch;
          if ((dst - quine_txt + 12) % 70 == 0) {
            int i;
            for (i = 0; i < 5; i++) *dst++ = "\"\n  \""[i];
          }
        }
        break;
      case 126: ch -=  8;
      case  94: ch -= 52;
      case 124: ch -= 32;
      default:
        *dst++ = ch;
    }
  }
  *dst = 0;
}

static int quine_read(char *page, char **start, off_t off,
               int count, int *eof, void *data)
{
  char *txt = data;
  int n = strlen(txt) - off;
  if (n <= 0) {
    *eof = 1;
    return 0;
  }
  if (n > count) n = count;
  memcpy(page, txt + off, n);
  *start = page;
  return n;
}

static int quine_init_module(void)
{
  struct proc_dir_entry *path;

  quine();

  dir = proc_mkdir("quine", 0);
  if (dir == NULL) {
    printk("quine: failed to create /proc/quine\n");
    goto err0;
  }

  path = create_proc_read_entry(
    "quine.c", 0444, dir, quine_read, quine_txt);
  if (path == NULL) {
    printk("quine: failed to create /proc/quine/quine.c\n");
    goto err1;
  }

  path = create_proc_read_entry(
    "Makefile", 0444, dir, quine_read, makefile_txt);
  if (path == NULL) {
    printk("quine: failed to create /proc/quine/Makefile\n");
    goto err2;
  }

  printk("quine loaded; copy /proc/quine/ and make\n");
  return 0;

  err2: remove_proc_entry("quine.c", dir);
  err1: remove_proc_entry("quine", 0); dir = 0;
  err0: return -ENOMEM;
}

static void quine_cleanup_module(void)
{
  if (dir) {
    remove_proc_entry("quine.c", dir);
    remove_proc_entry("Makefile", dir);
  }
  remove_proc_entry("quine", 0);
  printk("quine unloaded.\n");
}

module_init(quine_init_module);
module_exit(quine_cleanup_module);
