/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 *
 * Modified by Pasquale J. Rinaldi, Jr. to be used for aff fuse mounting
 * as a function.
 */

#ifndef EWFFUSE_H
#define EWFFUSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define FUSE_USE_VERSION 35

#include <libewf.h>
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <pthread.h>
#include <sys/fsuid.h>
#include <paths.h>

libewf_handle_t* ewfhandle = NULL;
libewf_error_t* ewferror = NULL;
static char* erawpath = NULL;
static off_t erawsize = 0;
static const char* erawext = ".raw";
/*
#define XCALLOC(type, num) ((type *) xcalloc ((num), sizeof(type)))
#define XMALLOC(type, num) ((type *) xmalloc ((num) * sizeof(type)))
#define XFREE(stale) do { if(stale) { free ((void*) stale); stale = 0; } } while (0)

static AFFILE* afimage = NULL;
static char* rawpath = NULL;
static off_t rawsize = 0;
static const char* rawext = ".raw";

static void* xmalloc(size_t num)
{
    void* alloc = malloc(num);
    if(!alloc)
    {
	perror("Memory Exhausted");
	exit(EXIT_FAILURE);
    }
    return alloc;
};

static void* xcalloc(size_t num, size_t size)
{
    void* alloc = xmalloc(num*size);
    memset(alloc, 0, num*size);
    return alloc;
};

static char* xstrdup(char* string)
{
    return strcpy((char*)xmalloc(strlen(string) + 1), string);
};
*/
static void* ewfuse_init(struct fuse_conn_info* conn, struct fuse_config* cfg)
{
    (void) conn;
    cfg->kernel_cache = 1;
    return NULL;
};

static int ewfuse_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	//} else if (strcmp(path+1, options.filename) == 0) {
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(strcmp(path, erawpath) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = erawsize;
		//stbuf->st_size = strlen(options.mntpt);
	} else
		res = -ENOENT;

	return res;
};

static int ewfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, (fuse_fill_dir_flags)0);
	filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);
	filler(buf, erawpath + 1, NULL, 0, (fuse_fill_dir_flags)0);

	return 0;
};

static int ewfuse_open(const char *path, struct fuse_file_info *fi)
{
	if(strcmp(path, erawpath) != 0)
		return -ENOENT;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;

	return 0;
};

static int ewfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int res = 0;
	(void) fi;
	if(strcmp(path, erawpath) != 0)
		return -ENOENT;
	//af_seek(afimage, (uint64_t)offset, SEEK_SET);
        //off64_t libewf_handle_seek_offset(libewf_handle_t *handle, off64_t offset, int whence, libewf_error_t **error );
        res = libewf_handle_seek_offset(ewfhandle, offset, SEEK_SET, &ewferror);

	errno = 0;
	//res = af_read(afimage, (unsigned char*)buf, (int)size);
        res = libewf_handle_read_buffer(ewfhandle, buf, size, &ewferror);
        //ssize_t libewf_handle_read_buffer(libewf_handle_t *handle, void *buffer, size_t buffer_size, libewf_error_t **error);

	return res;
};

static void ewfuse_destroy(void* param)
{
    libewf_handle_close(ewfhandle, &ewferror);
    libewf_handle_free(&ewfhandle, &ewferror);
    //af_close(afimage);
    XFREE(erawpath);
    return;
};

static const struct fuse_operations ewfuse_oper = {
	.getattr	= ewfuse_getattr,
	.open		= ewfuse_open,
	.read		= ewfuse_read,
	.readdir	= ewfuse_readdir,
	.init           = ewfuse_init,
	.destroy	= ewfuse_destroy,
};

void* ewfuselooper(void *data)
{
    struct fuse* fuse = (struct fuse*) data;
    fuse_loop(fuse);
    //int ret = fuse_loop(fuse);
    //printf("fuse loop return: %d\n", ret);
    //pthread_exit(NULL);
};

struct fuse_args ewargs;
struct fuse* ewfuser;
//struct fuse_session* affusersession;
pthread_t ewfusethread;

void EwfFuser(QString imgpath, QString imgfile)
{
    char** ewffilenames = NULL;
    char* filenames[1] = {NULL};
    //char* filenames[];
    //system_character_t * const *imgfilenames = NULL;
    //system_character_t **libewf_filenames = NULL;
    //system_character_t *filenames[ 1 ]    = { NULL };
    //filenames[ 0 ]      = (system_character_t *) filename;
    int ret;
    char* afpath = NULL;
    char* afbasename = NULL;
    size_t rawpathlen = 0;
    char** fargv = NULL;
    fargv = XCALLOC(char *, 3);
    int fargc = 0;
    char* ipath = new char[imgpath.toStdString().size() + 1];
    strcpy(ipath, imgpath.toStdString().c_str());
    char* iname = new char[imgfile.toStdString().size() + 1];
    strcpy(iname, imgfile.toStdString().c_str());
    filenames[0] = (char*)iname;
    libewf_handle_initialize(&ewfhandle, &ewferror);
    libewf_handle_open(ewfhandle, filenames, 1, LIBEWF_OPEN_READ, &ewferror);
    //libewf_handle_open(ewfhandle, (char* const)iname, 1, LIBEWF_OPEN_READ, &ewferror)
    fargv[0] = "./effuse";
    //fargv[1] = "-o";
    //fargv[2] = "auto_unmount";
    fargc = 1;
    //for(int i=0; i < fargc; i++)
    //    printf("fargv[%d]: %s\n", i, fargv[i]);

    //afimage = af_open(iname, O_RDONLY|O_EXCL,0);
    afpath = xstrdup(ipath);
    printf("efpath: %s\n", afpath);
    afbasename = basename(iname);
    rawpathlen = 1 + strlen(afbasename) + strlen(erawext) + 1;
    erawpath = XCALLOC(char, rawpathlen);
    erawpath[0] = '/';
    strcat(erawpath, afbasename);
    strcat(erawpath, rawext);
    erawpath[rawpathlen - 1] = 0;
    XFREE(afpath);

    //int libewf_handle_get_media_size(libewf_handle_t *handle, size64_t *media_size, libewf_error_t **error );
    libewf_handle_get_media_size(ewfhandle, (size64_t*)&erawsize, &ewferror);

    //rawsize = af_get_imagesize(afimage);
    
    struct fuse_loop_config config;
    config.clone_fd = 0;
    config.max_idle_threads = 5;
    ewargs = FUSE_ARGS_INIT(fargc, fargv);
    ewfuser = fuse_new(&ewargs, &ewfuse_oper, sizeof(fuse_operations), NULL);
    ret = fuse_mount(ewfuser, imgpath.toStdString().c_str());
    fuse_daemonize(1);
    pthread_create(&ewfusethread, NULL, ewfuselooper, (void*) ewfuser);
    //affuser = fuse_new(&args, &affuse_oper, sizeof(fuse_operations), NULL);
    //if(affuser == NULL)
    //    qDebug() << "affuser new error.";
    //ret = fuse_mount(affuser, imgpath.toStdString().c_str());
    //int retd = fuse_daemonize(1);

    //affusersession = fuse_get_session(affuser); 
    //struct fuse_session* se = fuse_get_session(affuser);
    //int retsh = fuse_set_signal_handlers(se);
    //qDebug() << "fuse session signal handlers:" << retsh;
    //pthread_t threadId;
    //int perr = pthread_create(&fusethread, NULL, fuselooper, (void *) affuser);
};

#endif // EWFFUSE_H