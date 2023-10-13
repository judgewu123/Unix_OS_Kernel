#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef struct pair
{
  int d_c;
  int f_c;
}pair;

int d[2], r[2];
int ar[6];
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

pair
tree(char *path, int level){
    
    char buf[DIRSIZ], *p;
    int fd, bytes;
    struct dirent de;
    struct stat st;
    struct pair a;
    struct pair b;
    
    a.d_c = 0;
    a.f_c = 0;
    if((fd = open(path, 0))<0){
        printf("%s [error opening dir]\n", path);
        close(fd);
        return a;
    }
    
    if(fstat(fd, &st) < 0){
      close(fd);
      return a;
    }
    if(st.type==T_DIR){
      strcpy(buf, path);
      p = buf + strlen(buf);
      if(*(p-1)!='/') *p++='/';
      bytes = read(fd, &de, sizeof(de));
      while(bytes>0){
        if(strcmp(de.name, ".")==0||strcmp(de.name, "..")==0){
          bytes = read(fd, &de, sizeof(de));
          continue;
        }
        if(de.inum==0){
          bytes = read(fd, &de, sizeof(de));
          continue;
        }
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
          bytes = read(fd, &de, sizeof(de));
          continue;
        }
        for(int j = 0 ; j < level-1; j++){
          if(ar[j+1]){
            printf(" ");
          }
          else{
            printf("|");
          }
          printf("   ");
        }
        printf("|\n");
        for(int j = 0 ; j < level-1; j++){
          if(ar[j+1]){
            printf(" ");
          }
          else{
            printf("|");
          }
          printf("   ");
        }
        char tmp[DIRSIZ];
        memmove(tmp, de.name, sizeof(de.name));
        bytes = read(fd, &de, sizeof(de));
        if(bytes!=sizeof(de)){
          ar[level]=1;
        }
        if(st.type == 1){
          a.d_c+=1;
          printf("+-- %s\n", tmp);
          b = tree(buf, level+1);
          a.d_c += b.d_c;
          a.f_c += b.f_c;
        }else{
          a.f_c +=1;
          printf("+-- %s\n", tmp);
        }
      }
    }
  close(fd);
  return a;
}

int main(int argc, char* argv[]) {
    int fd, pid, nbytes, temp, dir_count=0, file_count=0;;
    struct stat st;
    struct pair k;
    if((fd = open(argv[1], 0))<0){
      printf("%s [error opening dir]\n", argv[1]);
      close(fd);
      printf("\n%d directories, %d files\n", dir_count, file_count);
      exit(0);
    }

    if(fstat(fd, &st) < 0){
      close(fd);
      exit(0);
    }

    if(st.type==T_FILE){
      printf("%s [error opening dir]\n", argv[1]);
      close(fd);
      printf("\n%d directories, %d files\n", dir_count, file_count);
      exit(0);
    }

    if(pipe(d)<0) exit(0); //pipe fail;
    if(pipe(r)<0) exit(0); //pipe fail;

    printf("%s\n", argv[1]);
    pid = fork();
    if(pid == 0){
      k = tree(argv[1],1);
      close(d[0]);
      close(r[0]);
      write(d[1], &k.d_c, sizeof(k.d_c));
      write(r[1], &k.f_c, sizeof(k.f_c));
      exit(0);
    }else{
      wait(0);
      close(r[1]);
      close(d[1]);
      while(1){
        nbytes = read(r[0], &temp, sizeof(temp));
        if(nbytes<=0) break;
        file_count+=temp;
      }
      while(read(d[0], &temp, sizeof(temp))>0){
        dir_count+=temp;
      }
      printf("\n%d directories, %d files\n", dir_count, file_count);
      
    }
    
    exit(0);
}

