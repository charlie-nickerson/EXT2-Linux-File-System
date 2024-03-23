#include "symlink.h"

int sym_link(char* old_file, char* new_file){
    char* name = basename(old_file);
    char buf[BLKSIZE];
    name[strlen(name)] = 0;
    // Check if the old file exists
    int old_ino = getino(old_file);
    if(old_ino <= 0){
        printf("File %s does not exist.\n", old_file);
        return 0;
    }
    // Check if the new file does not exist
    int new_ino = getino(new_file);
    if(new_ino > 0){
        printf("File %s already exists.\n", new_file);
        return 0;
    }
    creat_file(new_file);
    new_ino = getino(new_file);
    MINODE* mip = iget(dev, new_ino);
    // Set inode to symbolic link
    mip->inode.i_mode = 0120000;
    get_block(mip->dev, mip->inode.i_block[0], buf);
    strcpy(buf, name);
    put_block(mip->dev, mip->inode.i_block[0], buf);
    mip->inode.i_size = strlen(name);
    mip->dirty = 1;
    iput(mip);
    return 1;
}