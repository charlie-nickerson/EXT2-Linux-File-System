#include "rmdir_rm.h"
#include "mkdir_creat.h"
#include "link_unlink.h"

int rm_dir(char *path){
    //Check dirname
    if(path)
    {
        int ino, pino;
        char name[256], buf[BLKSIZE], *cp;
        MINODE *mip, *pmip;
        
        // Path is absolute if true and relative if false
        if (abs_path(path)==0)
        { 
          dev = root->dev; 
          mip = root;
        } 
        else 
        {                   
          mip = running->cwd;
          dev = running->cwd->dev;
          printf("dev: %d running->cwd->dev: %d\n", dev, running->cwd->dev);
        }
        // 1. Get in-memory inode of pathname
        ino = getino(path);
        mip = iget(dev, ino);

        findmyname(mip, ino, name);
        printf("dir name = %s pino = %d parent name = %s\n", path, mip->ino, name);
        // Verify inode is a DIR
        if(!S_ISDIR(mip->inode.i_mode)){
            printf("File is not dir type. Try using rm.\n");
            iput(mip);
            return -1;
        }
        // Check if DIR is busy
        if(mip->refCount > 1 || mip->mounted || mip == running->cwd){
          printf("Error: %s is busy", path);
          iput(mip);
        }
        // If S_ISDIR() & refCount <= 1
        if ((mip->inode.i_mode & 0xF000) == 0x4000 && mip->refCount <= 1){
          printf("mip is in a direcctory.\n");
          printf("mip link count = %d.\n", mip->inode.i_links_count);

          if(mip->inode.i_links_count < 3){
            int actual_links_count = 0;

            get_block(dev, mip->inode.i_block[0], buf);
            dp = (DIR*)buf;
            cp = buf;

            while(cp < (buf + BLKSIZE)){
              actual_links_count++;
              cp += dp->rec_len;
              dp = (DIR*)cp;
            }

            if(actual_links_count < 3){
              for(int i = 0; i < 12; i++){
                if(mip->inode.i_block[i] == 0){
                  continue;
                }
                else{
                  bdalloc(mip->dev, mip->inode.i_block[i]);
                }
                idalloc(mip->dev, mip->ino);
                iput(mip);
                // Get parent's ino and inode
                u32 *inum = malloc(8);
                pino = findino(mip, inum);
                pmip = iget(mip->dev, pino);
                // Get the name from parent DIR's data block
                findmyname(pmip, ino, name);
                printf("pino = %d ino = %d name = %s\n", pino, ino, name);
                // If name is new:
                // 1. Remove the name from the parent directory
                // 2. Decrement the parent link_count
                // 3. Mark the parent mip as dirty and then write the pmip into the block
                // 4. Deallocate parent mip's data blocks and inode
                if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0 && strcmp(name, "/") != 0){
                  // Remove name from parent directory
                  rm_child(pmip, name); 
                  // Decrement parent links_count
                  pmip->inode.i_links_count--; 
                  pmip->inode.i_mtime = pmip->inode.i_atime = time(0L); // touch a/mtime
                  // mark dirty
                  pmip->dirty = 1; 
                  bdalloc(pmip->dev, mip->inode.i_block[0]);
                  idalloc(pmip->dev, mip->ino);
                  iput(pmip);
                }
                else{
                  printf("mip has the link count %d which is greater than 3\n", actual_links_count);
                }
              }
            }
            else{
              printf("mips children has the link count %d which is greater than 3", actual_links_count);
            }
          }
          else if(mip->refCount > 1){
            printf("mip=%d was busy.\n", mip->ino);
            iput(mip);
          }
          else{
            printf("File is not a directory. Try using rm.\n");
            iput(mip);
          }
        }
      }
      return 0;
}

int rm_child(MINODE *pmip, char *name){
    char buf[BLKSIZE], *cp, *rm_cp, temp[256];
    DIR *dp;
    int block_i, i, j, size, last_len, rm_len;
    // Search parent inode's data blocks for the name entered
    for (i=0; i<pmip->inode.i_blocks; i++){
        if (pmip->inode.i_block[i]==0) break;
        get_block(pmip->dev, pmip->inode.i_block[i], buf); // Get next block
        printf("get_block i=%d\n", i);
        printf("name=%s\n", name);
        dp = (DIR*)buf;
        cp = buf;
        // Keep track of block index
        block_i = i;
        i=0;
        j=0;

        while (cp + dp->rec_len < buf + BLKSIZE){
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;

            if (!strcmp(name, temp)){
                i=j;
                rm_cp = cp;
                rm_len = dp->rec_len;
                printf("rm=[%d %s] ", dp->rec_len, temp);
            } else
                printf("[%d %s] ", dp->rec_len, temp);


            last_len = dp->rec_len;
            cp += dp->rec_len;
            dp = (DIR*)cp;
            j++; // get count of entries into j 
        }
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;

        printf("[%d %s]\n", dp->rec_len, temp);
        printf("block_i=%d\n", block_i);

        if (j==0){ // first entry
            printf("First entry!\n");

            printf("deallocating data block=%d\n", block_i);
            bdalloc(pmip->dev, pmip->inode.i_block[block_i]); // dealloc this block

            for (i=block_i; i < pmip->inode.i_blocks; i++); // move other blocks up
        } else if (i==0) { // last entry
            cp -= last_len; // remove last entry
            last_len = dp->rec_len;
            dp = (DIR*)cp;
            dp->rec_len += last_len; // Absorb the rec_len with the previous entry
            printf("dp->rec_len=%d\n", dp->rec_len);
        } else { // middle entry
            size = buf+BLKSIZE - (rm_cp+rm_len);
            printf("copying n=%d bytes\n", size);
            // Move all trailing entries to he left to overlay the deleted entry
            memmove(rm_cp, rm_cp + rm_len, size);
            cp -= rm_len; 
            dp = (DIR*)cp;
            // Add the previous rec_len to the last entry
            dp->rec_len += rm_len;

            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
        }

        put_block(pmip->dev, pmip->inode.i_block[block_i], buf);
    }

    return 0;
}