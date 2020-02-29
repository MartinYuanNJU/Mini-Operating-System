#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "data.h"
#include "func.h"

/*
int main(int argc, char *argv[]) {
	char driver[NAME_LENGTH];
	char rootDirPath[NAME_LENGTH];
	char srcFilePath[NAME_LENGTH];
	char destDirPath[NAME_LENGTH];
	char destFilePath[NAME_LENGTH];

	stringCpy("fs.bin", driver, NAME_LENGTH - 1);
	stringCpy("/", rootDirPath, NAME_LENGTH - 1);
	stringCpy("./test", srcFilePath, NAME_LENGTH - 1);
	stringCpy("/doc", destDirPath, NAME_LENGTH - 1);
	stringCpy("/doc/test", destFilePath, NAME_LENGTH - 1);
	
	format(driver, SECTOR_NUM, SECTORS_PER_BLOCK);
	mkdir(driver, destDirPath);
	ls(driver, rootDirPath);
	cp(driver, srcFilePath, destFilePath);
	ls(driver, destDirPath);
	ls(driver, destFilePath);
	//cat(driver, destFilePath);
	rm(driver, destFilePath);
	ls(driver, destDirPath);
	ls(driver, rootDirPath);
	rmdir(driver, destDirPath);
	ls(driver, rootDirPath);

	return 0;
}
*/

int main(int argc, char *argv[]) {
	char driver[NAME_LENGTH];
	char srcFilePath[NAME_LENGTH];
	char destFilePath[NAME_LENGTH];

	stringCpy("fs.bin", driver, NAME_LENGTH - 1);
	
    // STEP 1
    // TODO: build file system of os.img, see lab5 4.3.
    // All functions you need have been completed
	int ret=0;
	ret=format(driver,SECTOR_NUM,SECTORS_PER_BLOCK);
	if(ret!=0)
		return -1;

	char destDirPathBoot[NAME_LENGTH];
	stringCpy("/boot/",destDirPathBoot,NAME_LENGTH-1);
	ret=mkdir(driver,destDirPathBoot);

	char destDirPathDev[NAME_LENGTH];
	stringCpy("/dev/",destDirPathDev,NAME_LENGTH-1);
	ret=mkdir(driver,destDirPathDev);

	char destDirPathUsr[NAME_LENGTH];
	stringCpy("/usr/",destDirPathUsr,NAME_LENGTH-1);
	ret=mkdir(driver,destDirPathUsr);

	stringCpy(argv[1],srcFilePath,NAME_LENGTH-1);
	stringCpy("/boot/initrd",destFilePath,NAME_LENGTH-1);
	ret=cp(driver,srcFilePath,destFilePath);
	if(ret!=0)
		return -1;
	
	char stdinFilePath[NAME_LENGTH-1];
	char stdoutFilePath[NAME_LENGTH-1];
	stringCpy("/dev/stdin",stdinFilePath,NAME_LENGTH-1);
	stringCpy("/dev/stdout",stdoutFilePath,NAME_LENGTH-1);
	ret=touch(driver,stdinFilePath);
	if(ret!=0)
		return -1;
	ret=touch(driver,stdoutFilePath);
	if(ret!=0)
		return -1;

    ls(driver, "/");
    ls(driver, "/boot/");
    ls(driver, "/dev/");
    ls(driver, "/usr/");
    
    /** output:
    ls /
    Name: boot, Type: 2, LinkCount: 1, BlockCount: 1, Size: 1024.
    Name: dev, Type: 2, LinkCount: 1, BlockCount: 1, Size: 1024.
    Name: usr, Type: 2, LinkCount: 1, BlockCount: 0, Size: 0.
    LS success.
    8185 inodes and 3052 data blocks available.
    ls /boot/
    Name: initrd, Type: 1, LinkCount: 1, BlockCount: 14, Size: 13400.
    LS success.
    8185 inodes and 3052 data blocks available.
    ls /dev/
    Name: stdin, Type: 1, LinkCount: 1, BlockCount: 0, Size: 0.
    Name: stdout, Type: 1, LinkCount: 1, BlockCount: 0, Size: 0.
    LS success.
    8185 inodes and 3052 data blocks available.
    ls /usr/
    LS success.
    8185 inodes and 3052 data blocks available.
	*/
    
	return 0;
}
