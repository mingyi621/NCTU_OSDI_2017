/* This file use for NCTU OSDI course */
/* It's contants fat file system operators */

#include <inc/stdio.h>
#include <fs.h>
#include <fat/ff.h>
#include <diskio.h>

extern struct fs_dev fat_fs;

/*TODO: Lab7, fat level file operator.
 *       Implement below functions to support basic file system operators by using the elmfat's API(f_xxx).
 *       Reference: http://elm-chan.org/fsw/ff/00index_e.html (or under doc directory (doc/00index_e.html))
 *
 *  Call flow example:
 *        ┌──────────────┐
 *        │     open     │
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │   sys_open   │  file I/O system call interface
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │  file_open   │  VFS level file API
 *        └──────────────┘
 *               ↓
 *        ╔══════════════╗
 *   ==>  ║   fat_open   ║  fat level file operator
 *        ╚══════════════╝
 *               ↓
 *        ┌──────────────┐
 *        │    f_open    │  FAT File System Module
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │    diskio    │  low level file operator
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │     disk     │  simple ATA disk dirver
 *        └──────────────┘
 */

/* Note: 1. Get FATFS object from fs->data
*        2. Check fs->path parameter then call f_mount.
*/
int fat_mount(struct fs_dev *fs, const void* data)
{

}

/* Note: Just call f_mkfs at root path '/' */
int fat_mkfs(const char* device_name)
{
	int res;
//    	printk("[MKFS] Device : %s\n", device_name);
    	res = f_mkfs("/", 0, 0);

    	if (res != 0)
        	return -1;
    	return 0;
}

/* Note: Convert the POSIX's open flag to elmfat's flag.
*        Example: if file->flags == O_RDONLY then open_mode = FA_READ
*                 if file->flags & O_APPEND then f_seek the file to end after f_open
*/
int fat_open(struct fs_fd* file)
{
	FIL* data = file->data;
    	uint32_t flags = 0;
    
    	flags |= O_WRONLY & file->flags ?  FA_WRITE : 0;
    	flags |= (O_CREAT & file->flags) && (O_TRUNC & file->flags) ? FA_CREATE_ALWAYS : 0;
    	flags |= (O_CREAT & file->flags) && !(O_TRUNC & file->flags) ? FA_CREATE_NEW : 0;
    	flags |= file->flags == 0 ? FA_READ : 0;
    	flags |= O_RDWR & file->flags ? FA_READ|FA_WRITE : 0; 
    
//    	printk("[FAT OPEN] flags = %d\n", flags);
     
    	int ret_val = f_open(data, file->path, flags);

    	if (ret_val != 0)
        	return -ret_val;

    	if ((file->flags & O_TRUNC) || (file->flags & O_CREAT))
        	file->size = 0;
    	file->pos = 0;
    	return ret_val;
}

int fat_close(struct fs_fd* file)
{
	int ret_val;
   	FIL* data = file->data;
   	ret_val = f_close(data);
   	if (ret_val != 0)
       		ret_val = -ret_val;
   	return ret_val; 

}
int fat_read(struct fs_fd* file, void* buf, size_t count)
{
	FIL* data = file->data;
    	UINT bw;
    	int ret_val = -1;
    	ret_val = f_read(data, buf, count, &bw);
    	if (ret_val != 0)
        	return -ret_val;
    
    	file->pos += bw;
    	return bw;

}
int fat_write(struct fs_fd* file, const void* buf, size_t count)
{
    	FIL* data = file->data;
    	UINT bw;
    	int ret_val = -1;
    	ret_val = f_write(data, buf, count, &bw);
    	if (ret_val != 0)
        	return -ret_val;
    
    	int next_pos = file->pos + bw;
    	int size_offset = 0;
    	if (next_pos > file->size) 
        	size_offset = next_pos - file->size;
    	file->size += size_offset;   // update size of file
    	file->pos = next_pos;
    	
    	return bw;
}
int fat_lseek(struct fs_fd* file, off_t offset)
{
	int retVal = f_lseek(file->data, offset);
	printk("[%s] retVal: %d\n", __func__, retVal);
	if(retVal != 0)
		return -retVal;
	return 0;
}
int fat_unlink(struct fs_fd* file, const char *pathname)
{
	int retVal = f_unlink(pathname);
	if(retVal != 0)
		return -retVal;
	return 0;
}

struct fs_ops elmfat_ops = {
    .dev_name = "elmfat",
    .mount = fat_mount,
    .mkfs = fat_mkfs,
    .open = fat_open,
    .close = fat_close,
    .read = fat_read,
    .write = fat_write,
    .lseek = fat_lseek,
    .unlink = fat_unlink
};



