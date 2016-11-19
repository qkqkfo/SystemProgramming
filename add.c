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
char * fileName = NULL;		//절단된 파일의 고유 이름(뒤에 숫자 제외)
char * resultName = NULL;	//합쳐진 파일의 이름
int blockSize;
int inItO = 0;				//파일의 이름은 필수로 받아야하는 조건이므로 받았는지 안받았는지 프로그램에게 알려주기 위해 선언
int inItX = 0;				//-x옵션이 포함돼 있는가?
int adjust_Index = 0;		//-x 옵션을 받으면 그 때 한 번 argv의 인덱스값을 하나 빼준다. why? i 값은 2씩 더해지기 진다.-x옵션은 따로 문자열을 받지 않아서 옵션의 위치가 홀수->짝수로 바뀐다.
char input[LEN];			//절단된 파일들의 이름(숫자 포함)
int inputFileNum = 0;		//절단된 파일의 번호

int option(char * argv[], int i) {
	if (strcmp(argv[i], "-n") == 0) {			//합쳐서 생성되는 결과 파일의 이름
		resultName = argv[i + 1];
	}
	else if (strcmp(argv[i], "-x") == 0) {
		adjust_Index = 1;						//argv의 인덱스값을 하나 빼줘서 옵션값의 인덱스 위치를 조정 하기 위함
		inItX = 1;
	}
	else if (strcmp(argv[i], "-o") == 0) {		//절단돼있는 파일의 고유 이름
		fileName = argv[i + 1];
		inItO = 1;								//파일의 이름을 받았다는 표시
	}
	else if (strcmp(argv[i], "-d") == 0) {		//절단된 파일이 있는 위치, 합친 파일을 저장할 디렉토리 경로
		strcpy(pathName, argv[i + 1]);
	}
	else {										//잘못된 옵션을 호출할경우 오류
		perror("Wrong option!\n");
		return -1;
	}
	return 1;
}

int mergeFile(const char * pathName, const char * fileName, char * resultName, int BUFSIZE) {
	int inFile, outFile;
	ssize_t nread;
	char * buffer = (char *)malloc(sizeof(char)*BUFSIZE);

	while ((inFile = open(input, O_RDONLY)) != -1) {			//절단된 파일들을 모두 읽을때까지 읽기
		nread = read(inFile, buffer, BUFSIZE);
		if (inItX) unlink(input);								//-x옵션을 받은 경우 절단된 파일은 삭제한다.
		if ((outFile = open(resultName, O_WRONLY | O_CREAT | O_APPEND, PERM)) == -1) {	//PERM = 0664 ( -rw-rw-r--)
			close(inFile);																//O_APPEND : 파일의 끝에 이어서 쓰기 위함
			return -2;																	//결과 파일 이름으로 생성
		}
		write(outFile, buffer, nread);							//결과 파일에 절단된 파일들을 차례대로 이어서 쓴다.
		sprintf(input, "%s%03d", fileName, inputFileNum++);		//다음 절단된 파일을 읽기 위해 input을 갱신
	}
	close(inFile);
	close(outFile);
	free(buffer);					//동적해제
	if (nread == -1)	return -4;
	else				return 0;
}

int main(int argc, char * argv[]) {
	int i;
	struct stat file_info;											//파일의 정보

	if (getcwd(pathName, LEN) == NULL)	perror("getcwd error\n");	//절단된 파일의 위치는 default 값이 현재 작업중인 디렉토리
	if ((fd = open(pathName, O_RDONLY)) == -1) {					//최초 디렉토리로 이동할 수 있도록 fd를 생성
		perror("directory open error\n");							//fd는 close하지 않고 유지
	}

	for (i = 1; i < argc; i += 2) {
		if (option(argv, i) == -1) exit(-1);		//옵션값을 받는 함수
		if (adjust_Index) {							//-x 옵션을 받으면 argv의 인덱스값을 하나 빼준다. why? i 값은 2씩 더해지기 진다.-x옵션은 따로 문자열을 받지 않아서 옵션의 위치가 홀수->짝수로 바뀐다.
			adjust_Index = 0;
			i--;
		}	
	}

	if (inItO == 0) {														//필수 옵션 -o는 필요
		perror("필수 옵션 '-o'가 없습니다\n");
		exit(-1);
	}
	if (resultName == NULL)	resultName = fileName;							//-n 옵션이 없는 경우 절단해서 생성되는 결과 파일의 이름은 fimeName(default값)으로 설정
	chdir(pathName);														//절단된 파일이 있는 디렉토리로 이동
	sprintf(input, "%s%03d", fileName, inputFileNum++);						//-o옵션으로 받은 파일의 이름에 "000"을 붙여준다.
	if (0 > stat(input, &file_info)) {										//절단된 파일의 정보를 가져온다.
		perror("There is no cut_File.\n");									//절단된 파일이 존재하지 않을 경우 오류, 종료
		exit(-1);
	}
	blockSize = file_info.st_size;

	mergeFile(pathName, fileName, resultName, blockSize);

	printf("받은 파일의 이름 : %s\n", fileName);
	printf("결과 파일의 이름 : %s\n", resultName);
	printf("절단된 파일의 위치, 결과 파일이 생성될 경로 : %s\n", pathName);

	return 0;
}
