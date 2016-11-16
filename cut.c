#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PERM	0664			//-rw-rw-r--
#define LEN		1024
#define MAXNUM	1000
int fd;
char pathName[LEN];			//절단된 파일의 위치
char * fileName = NULL;
char * resultName = NULL;
int fileSize;
int blockSize = 512 << 10;	//default blockSize = 512KB
int inItO = 0;				//파일의 이름은 필수로 받아야하는 조건이므로 받았는지 안받았는지 프로그램에게 알려주기 위해 선언
int resultFileNum = 0;		//결과 파일의 번호

int option(char * argv[], int i) {
	if (strcmp(argv[i], "-n") == 0) {			//절단해서 생성되는 결과 파일의 이름
		resultName = argv[i + 1];
	}
	else if (strcmp(argv[i], "-s") == 0) {		//절단하려는 파일의 크기(단위 KB)
		blockSize = atoi(argv[i + 1]) << 10;
	}
	else if (strcmp(argv[i], "-o") == 0) {		//절단할 파일의 이름 주기
		fileName = argv[i + 1];
		inItO = 1;								//파일의 이름을 받았다는 표시
	}
	else if (strcmp(argv[i], "-d") == 0) {		//절단할 파일을 저장할 디렉토리 경로
		strcpy(pathName, argv[i + 1]);
	}
	else {										//잘못된 옵션을 호출할경우 오류
		perror("Wrong option!\n");
		return -1;
	}
	return 1;
}

int cutFile(const char * pathName, const char * fileName, char * resultName, int BUFSIZE) {
	int inFile, outFile;
	ssize_t nread;
	char * buffer = (char *)malloc(sizeof(char)*BUFSIZE);

	if ((inFile = open(fileName, O_RDONLY)) == -1)	return -1;

	while ((nread = read(inFile, buffer, BUFSIZE)) > 0) {
		chdir(pathName);		//결과 파일을 생성할 디렉토리로 이동
		char output[LEN];
		sprintf(output, "%s%03d", resultName, resultFileNum++);
		if ((outFile = open(output, O_WRONLY | O_CREAT | O_TRUNC, PERM)) == -1) {		//PERM = 0664 ( -rw-rw-r--)
			close(inFile);
			return -2;
		}
		if (write(outFile, buffer, nread) < nread) {
			close(inFile);
			close(outFile);
			return -3;
		}
		fchdir(fd);				//초기 디렉토리로 다시 이동
	}
	close(inFile);
	close(outFile);
	if (nread == -1)	return -4;
	else				return 0;
}

int main(int argc, char * argv[]) {
	int i;
	struct stat file_info;

	if (getcwd(pathName, LEN) == NULL)	perror("getcwd error\n");	//절단된 파일의 위치는 default 값이 현재 작업중인 디렉토리
	if ((fd = open(pathName, O_RDONLY)) == -1) {					//최초 디렉토리로 이동할 수 있도록 fd를 생성
		perror("directory open error\n");							//fd는 close하지 않고 유지
	}

	for (i = 1; i < argc; i += 2) if (option(argv, i) == -1) exit(-1);		//옵션값을 받는 함수

	if (inItO == 0) {														//필수 옵션 -o는 필요
		perror("필수 옵션 '-o'가 없습니다\n");
		exit(-1);
	}

	if (resultName == NULL)	resultName = fileName;							//-n 옵션이 없는 경우 절단해서 생성되는 결과 파일의 이름은 fimeName(default값)으로 설정

	if (0 > stat(fileName, &file_info)) {
		perror("There is no file.\n");
		exit(-1);
	}
	else {
		fileSize = file_info.st_size;
		if ((fileSize / blockSize) > MAXNUM) {								//분할 파일의 최대 갯수(1000개)를 넘기면
			perror("Block size is too small to cut\n");
			exit(-1);
		}
	}


	mkdir(pathName, 0755);													//drwxrwxr-x  = 0775로 디렉토리 생성
	cutFile(pathName, fileName, resultName, blockSize);

	printf("받은 파일의 이름 : %s\n", fileName);
	printf("나눌 파일의 단위 : %dKB\n", blockSize >> 10);
	printf("결과 파일의 이름 : %s\n", resultName);
	printf("결과 파일이 저장될 경로 : %s\n", pathName);

	return 0;
}
