#include "link_unlink.h"

int link_files(char *old_file, char *new_file){
    int oino;
    MINODE *omip;

    // 1. Verify path exists and is not a DIR
    oino = getino(old_file);
    omip = iget(dev, oino);
    // 1. Check if file is regular
    if(!S_ISREG(omip->inode.i_mode)){
        printf("File is not a regular file.\n");
        return 0;
    }
    printf("File is type REG.\n");
    // Ne file must not exist yet
    int lino = getino(link);
    if(lino != 0){
        printf("Link file already exists.\n");
        return 0;
    }
    // Create new_file with the same inode number of as the old_file
    char *parent = dirname(new_file);
    char *child  = basename(new_file);
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    // Creat entry in new parent DIR with same inode number of old_file
    enter_name(pmip, oino, child);
    // Increment inode's links_count by 1
    omip->inode.i_links_count++;
    // For write back bt iput(omip)
    omip->dirty = 1;
    iput(omip);
    iput(pmip);

    return 1;
}

int unlink_file(char* filename){
    // Get filename's minode
    int ino = getino(filename);
    // Check if linked file is a REG or symbolic LNK file
    if(ino <=0){
        printf("File does not exist.\n");
        return 0;
    }
    MINODE *mip = iget(dev, ino);
    if(!S_ISREG(mip->inode.i_mode) && !S_ISLNK(mip->inode.i_mode)){
        printf("File is not a REG or symbolic LNK file.\n");
        return 0;
    }
    // Remove name entry from parents DIR's data block
    char *parent = dirname(filename);
    char *child = basename(filename);
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    rm_child(pmip, child);
    pmip->dirty = 1;
    iput(pmip);
    // Decrement inode's link_count by 1
    mip->inode.i_links_count--;
    if(mip->inode.i_links_count > 0){
        mip->dirty = 1; // For write inode back to disk
    }
    else{
        // Deallocate all data blocks in inode
        // deallocate inode
        for(int i = 0; i < 12 && mip->inode.i_block[i] != 0; i++){
            bdalloc(dev, mip->inode.i_block[i]);
        }
        idalloc(dev, ino);
    }
    iput(mip);
    return 1;
}