Makefile_SRC = <<'END'.gsub(/^\s{8}/, "\t")
KDIR = XS-KDIR
EXTRA_CFLAGS = -DKDIR=$(KDIR)
obj-m += quine.o
all:
        make -C $(KDIR) M=$(PWD) modules
clean:
        make -C $(KDIR) M=$(PWD) clean
END

QuineC_SRC = <<'END'
/*
 * kernel-mode quine
 * Copyright (C) 2010,2015 Yusuke Endoh
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
#include <linux/seq_file.h>
#include <linux/init.h>

MODULE_DESCRIPTION("quine");
MODULE_AUTHOR("Yusuke Endoh <mame@ruby-lang.org>");
MODULE_LICENSE("GPL");

#define S(s) #s
#define XS(s) S(s)

static struct proc_dir_entry *dir;
static char quine_txt[8000];
static char makefile_txt[] = MAKEFILE-QUINE;
static char self[] = QUINE-C-QUINE;

static void quine(void)
{
  char ch, *dst = quine_txt, *src = self, *src2 = src;
  while ((ch = *src++)) {
    switch (ch) {
      case 33:
        while ((ch = *src2++)) {
          *dst++ = ch;
          if ((dst - quine_txt + XX) % 70 == 0) {
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

static int proc_quine_show(struct seq_file *m, void *not_used)
{
    seq_puts(m, (const char*) m->private);
    return 0;
}

static int proc_quine_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_quine_show, PDE_DATA(inode));
}

static const struct file_operations fops = {
    .open    = proc_quine_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release,
};

static int quine_init_module(void)
{
  struct proc_dir_entry *path;

  quine();

  dir = proc_mkdir("quine", 0);
  if (dir == NULL) {
    printk("quine: failed to create /proc/quine\n");
    goto err0;
  }

  path = proc_create_data("quine.c", 0, dir, &fops, quine_txt);
  if (path == NULL) {
    printk("quine: failed to create /proc/quine/quine.c\n");
    goto err1;
  }

  path = proc_create_data("Makefile", 0, dir, &fops, makefile_txt);
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
END

def replace(src, pattern, str)
  idx = src[/.*(?=#{ pattern })/].size - 3
  str = (?# * idx + str).gsub(/.{64}[^\\"]/) { $& + "\"\n  \"" }
  src.sub!(pattern) { str[idx .. -1] }
end

replace(QuineC_SRC, "MAKEFILE-QUINE",
  Makefile_SRC.dump.sub("XS-KDIR", '" XS(KDIR) "'))

QuineC_SRC.sub!("XX", "%2d" % (72 - (QuineC_SRC =~ /^.*QUINE-C-QUINE/) % 70))

replace(QuineC_SRC, "QUINE-C-QUINE",
  QuineC_SRC.gsub("QUINE-C-QUINE", '"!"').
    gsub(/[\n"\\]/, ?\n => ?^, ?" => ?~, ?\\ => ?|).dump)

Makefile_SRC.sub!("XS-KDIR", "/lib/modules/$(shell uname -r)/build")
 
File.write("Makefile", Makefile_SRC)
File.write("quine.c", QuineC_SRC)
