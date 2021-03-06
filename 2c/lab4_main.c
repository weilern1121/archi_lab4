
#include <sys/types.h>
#include "lab4_util.h"


#define SYS_WRITE 1
#define STDOUT 1
#define SYS_OPEN 2
#define SYS_READ 0
#define SYS_LSEAK 8
#define SYS_CLOSE 3
#define SYS_GETDENTS 78
#define SYS_EXIT 60
#define O_RDONLY 0


#define BUF_SIZE 1024


extern int system_call();
void connect_strings(char *a, char *b, char* output);
int str_size(char *str);
void file_name_search(char *path, char *filename);
void dir_search(char *path);
void connect_strings_with_space(char *a, char *b, char* output);
void connect_strings_regular(char *a, char *b,char *c, char* output);
int exac_command(char *path, char *filename, char *command, int flag_found);


void connect_strings_regular(char *a, char *b,char *c, char* output) {
    int sa = str_size(a);
    int sb = str_size(b);
    int sc = str_size(c);
    int i = 0, j = 0,k=0;
    for (; i < sa; i++)
        output[i] = a[i];
    for (; j < sb; j++)
        output[i + j] = b[j];
    for (; k < sc; k++)
        output[i + j + k] = c[k];
    output[i+j+k]='\0';
}

void connect_strings(char *a, char *b, char* output) {
    int sa = str_size(a);
    int sb = str_size(b);
    /*char *output = malloc((size_t) (sa + sb + 2));*/
    int i = 0, j = 0;
    for (; i < sa; i++)
        output[i] = a[i];
    output[i] = '/';
    i++;
    for (; j < sb; j++)
        output[i + j] = b[j];

    output[i+j]='\0';
}

void connect_strings_with_space(char *a, char *b, char* output) {
    int sa = str_size(a);
    int sb = str_size(b);
    /*char *output = malloc((size_t) (sa + sb + 2));*/
    int i = 0, j = 0;
    for (; i < sa; i++)
        output[i] = a[i];
    output[i] = ' ';
    i++;
    for (; j < sb; j++)
        output[i + j] = b[j];
    output[i+j]='\0';
}


int str_size(char *str) {

    int i;
    for (i = 0; str[i] != '\0'; ++i);
    return i;
}
/*
getdents()
       The system call getdents() reads several linux_dirent structures from
       the directory referred to by the open file descriptor fd into the
       buffer pointed to by dirp.  The argument count specifies the size of
       that buffer.
       */
struct linux_dirent {
    long d_ino; 		/* Inode number */
    off_t d_off;		/* Offset to next linux_dirent */
    unsigned short d_reclen;	/* Length of this linux_dirent */
    char d_name[];		/* Filename (null-terminated) */
};

int main(int argc, char *argv[]) {
    char *filename;
    char *command;
    int i, flagN = 0,flagE=0,flag_found=0;
    for (i = 0; i < argc; i++) {

        if (simple_strcmp(argv[i], "-n") == 0){
            i++;
            filename = argv[i];
            flagN = 1;
        }
	if (simple_strcmp(argv[i], "-e") == 0){
	    if(i+2>argc)
		system_call(SYS_EXIT,0x55);
            i++;
            filename = argv[i];
	    i++;
	    command=argv[i];
            flagE = 1;
        }
    }
    if (flagN == 1)
        file_name_search(".", filename);
    
	if (flagE == 1){
        	flag_found=exac_command(".", filename,command,flag_found);
		if(flag_found==0){
			char* s1="The file '";
			char* s2="' Does not exist.";
			char newPath[str_size(s1)+str_size(s2)+str_size(filename)+1];
			connect_strings_regular(s1,filename,s2,newPath);
			system_call(SYS_WRITE, STDOUT, newPath, str_size(newPath));
   			system_call(SYS_WRITE, STDOUT, "\n", 1);
		}
	}

	if(flagN==0 && flagE==0)
        	dir_search(".");
	
    return 0;
}

void dir_search(char *path) {
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;
    int bpos;
    char d_type;
    char *name;
    /*printf("%s\n", path);
    fd = open(path, 0);*/
    fd=system_call(SYS_OPEN,path,O_RDONLY,0777);
    system_call(SYS_WRITE, STDOUT, path, str_size(path));
    system_call(SYS_WRITE, STDOUT, "\n", 1);
    if (fd == -1)
        system_call(SYS_EXIT,0x55);
    for (;;) {

        /*nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);*/
        nread = system_call(SYS_GETDENTS, fd, buf, BUF_SIZE);
        if (nread == -1)
            system_call(SYS_EXIT,0x55);

        if (nread == 0)
            break;

        /*system_call(SYS_WRITE,STDOUT, nread, bpos);*/

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            d_type = *(buf + bpos + d->d_reclen - 1);
            if (d->d_ino != 0) {
                name = d->d_name;
                if (d_type == 8 || d_type == 4) {
                    if (simple_strcmp(name, ".") != 0 && simple_strcmp(name, "..") != 0) {
			char newPath[str_size(path)+str_size(name)+2];
                        /*printf("%s\n", connect_strings(path, name,newPath));*/
			connect_strings(path, name,newPath);
			system_call(SYS_WRITE, STDOUT,newPath, str_size(newPath));
   			system_call(SYS_WRITE, STDOUT, "\n", 1);
                        if (d_type == 4)
                            dir_search(newPath);
                    }
                }
                /*system_call(SYS_WRITE,STDOUT, d->d_name,simple_strlen(d->d_name));
                system_call(SYS_WRITE,STDOUT, (char *) "\n",1)*/
                bpos += d->d_reclen;
            }
        }
    }
}

void file_name_search(char *path, char *filename) {
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;
    int bpos;
    char d_type;
    char *name;
    fd=system_call(SYS_OPEN,path,O_RDONLY,0777);
    /*system_call(SYS_WRITE, STDOUT, path, str_size(path));
    system_call(SYS_WRITE, STDOUT, "\n", 1);*/
 
    if(simple_strcmp(filename,".")==0){
        /*printf("%s\n", path);*/
	system_call(SYS_WRITE, STDOUT, path, str_size(path));
	system_call(SYS_WRITE, STDOUT, "\n", 1);
	}

    for (;;) {
        /*nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);*/
        nread = system_call(SYS_GETDENTS, fd, buf, BUF_SIZE);
    	if (fd == -1)
	        system_call(SYS_EXIT,0x55);
        if (nread == 0)
            break;

        /*system_call(SYS_WRITE,STDOUT, nread, bpos);*/

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            d_type = *(buf + bpos + d->d_reclen - 1);
            if (d->d_ino != 0) {
                name = d->d_name;

                if (d_type == 8 || d_type == 4) {
                    if (simple_strcmp(name, ".") != 0 && simple_strcmp(name, "..") != 0) {
			char newPath[str_size(path)+str_size(name)+2];
                        if (simple_strcmp(name, filename) == 0){
                            /*printf("%s \n", connect_strings(path, name));*/
			connect_strings(path, name,newPath);
			system_call(SYS_WRITE, STDOUT, newPath, str_size(newPath));
   			system_call(SYS_WRITE, STDOUT, "\n", 1);
			}
                        if (d_type == 4){
			    connect_strings(path, name,newPath);
                            file_name_search(newPath, filename);}
                    }
                }
                bpos += d->d_reclen;
            }
        }
    }
}


int exac_command(char *path, char *filename, char *command, int flag_found) {
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;
    int bpos;
    char d_type;
    char *name;
    /*if (strcmp(filename, ".") == 0)
        printf("%s\n", path);*/
    fd=system_call(SYS_OPEN,path,O_RDONLY,0777);
    /*fd=system_call(SYS_OPEN,argc > 1 ? argv[1] : ".", 2,0777);*/
    if (fd == -1)
	        system_call(SYS_EXIT,0x55);
    for (;;) {

        nread = system_call(SYS_GETDENTS, fd, buf, BUF_SIZE);
        if (nread == -1)
	        system_call(SYS_EXIT,0x55);
        if (nread == 0)
            break;
        /*system_call(SYS_WRITE,STDOUT, nread, bpos);*/
        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            d_type = *(buf + bpos + d->d_reclen - 1);
            if (d->d_ino != 0) {
                name = d->d_name;
                if (d_type == 8 || d_type == 4) {
                    if (simple_strcmp(name, ".") != 0 && simple_strcmp(name, "..") != 0) {
                  	char newPath[str_size(path)+str_size(name)+2];
			if (simple_strcmp(name, filename) == 0) {
			    connect_strings(path, name,newPath);
                  	    char newCommand[str_size(newPath)+str_size(command)+2];
			    connect_strings_with_space(command, newPath,newCommand);
			    flag_found=1;
                            simple_system(newCommand);
                        }
                        if (d_type == 4) {
			    connect_strings(path, name,newPath);
                            exac_command(newPath, filename, command,flag_found);
                        }
                    }
                }
                bpos += d->d_reclen;
            }
        }
    }
	return flag_found;
}
