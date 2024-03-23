# Ext2 Filesystem Simulator

This project is an in-depth simulation of the Ext2 filesystem, meticulously crafted in C. It's designed to mimic essential filesystem functionalities such as creating, reading, writing, deleting files and directories, alongside advanced operations like linking and symbolic linking of files. This simulator serves as an excellent resource for understanding the underlying mechanisms of filesystems in a Unix/Linux environment, focusing on inode and block management through a user-space implementation.

## Features

- **File Operations**: Comprehensive support for file operations including creation, deletion, reading, and writing.
- **Directory Management**: Functionalities to create, delete, navigate, and list directories, providing a hands-on experience with directory hierarchy management.
- **Symbolic and Hard Links**: Implements both symbolic and hard linking of files, offering insights into file sharing and inode usage.
- **Path Resolution**: Efficient resolution of both absolute and relative paths, enabling intricate filesystem navigation.
- **Virtual Disk Storage**: Utilizes a virtual disk for storage operations, simulating real-world filesystem behavior with block allocation and deallocation.
- **Inode Management**: Detailed simulation of inode operations, including allocation, deallocation, and manipulation, to closely understand file metadata handling.

## Core Components

- **Basic Commands**: Implements fundamental Unix commands such as `ls`, `cd`, `mkdir`, `rmdir`, `creat`, and `rm` for file and directory operations.
- **Utility Functions**: A rich set of utility functions in `util.c` for block I/O, inode management, directory searches, and path resolution.
- **Linking Functionalities**: Features in `link_unlink.c` and `symlink.c` for creating and managing hard links and symbolic links, enriching the filesystem simulation with advanced file operations.

## Files and Their Contents

### `type.h`
- Defines various data types and structures used across the filesystem code, including `MINODE`, `PROC`, `MTABLE`, `OFT`, along with the standard EXT2 FS structures like `INODE`, `GD` (group descriptor), `SUPER` (superblock), etc.
- `MINODE` represents an in-memory inode, `PROC` represents a process, `MTABLE` represents a mount table, and `OFT` represents an open file table entry.

### `util.h` and `util.c`
- Provide utility functions such as `get_block()`, `put_block()`, `tokenize()`, `iget()`, `iput()`, `search()`, `getino()`, and more. These are fundamental operations for block I/O, inode management, directory search, and path resolution.

### `cd_ls_pwd.h` and `cd_ls_pwd.c`
- Implement the `cd`, `ls`, and `pwd` commands. Functions like `cd()` change the current working directory, `ls()` lists directory contents, and `pwd()` prints the current working directory path.

### `mkdir_creat.h` and `mkdir_creat.c`
- Handle directory creation and file creation with functions like `make_dir()`, `mymkdir()`, `creat_file()`, and `mycreat()`.

### `rmdir_rm.h` and `rmdir_rm.c`
- Implement directory and file removal functionalities with functions such as `rm_dir()` and `rm_child()`.

### `main.c`
- Serves as the entry point for the filesystem application. It initializes the system and processes commands from the user.

## Function Usage Across Files

### Initialization and Mounting (`main.c` → `util.c`)
- At startup, `main.c` calls `mount_root()` to load the root inode into memory using `iget()` from `util.c`.

### Command Processing (`main.c` → Various)
- For filesystem commands entered by the user, `main.c` parses these commands and calls the appropriate functions from `cd_ls_pwd.c`, `mkdir_creat.c`, or `rmdir_rm.c`. For example, if the user enters the `cd` command, `main.c` invokes `cd()` from `cd_ls_pwd.c`.

### Directory Operations (`mkdir_creat.c` → `util.c`)
- When creating a directory, `make_dir()` in `mkdir_creat.c` uses several utility functions from `util.c`, such as `iget()` to load inodes into memory, `iput()` to release inodes, and `getino()` to find the inode number of a given path.

### Path Resolution and Inode Management (`cd_ls_pwd.c`, `mkdir_creat.c`, `rmdir_rm.c` → `util.c`)
- Functions like `cd()`, `ls()`, and `rm_dir()` rely on `getino()` from `util.c` to resolve paths to inode numbers. They also use `iget()` and `iput()` for managing in-memory inodes.

This overview captures the essence of how functions and data structures are defined and utilized across your filesystem implementation. Each `.c` file typically includes its corresponding `.h` file for function prototypes and global variable declarations, ensuring modularity and reusability of code.