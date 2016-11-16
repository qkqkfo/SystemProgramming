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
char pathName[LEN];			//���ܵ� ������ ��ġ
char * fileName = NULL;
char * resultName = NULL;
int fileSize;
int blockSize = 512 << 10;	//default blockSize = 512KB
int inItO = 0;				//������ �̸��� �ʼ��� �޾ƾ��ϴ� �����̹Ƿ� �޾Ҵ��� �ȹ޾Ҵ��� ���α׷����� �˷��ֱ� ���� ����
int resultFileNum = 0;		//��� ������ ��ȣ

int option(char * argv[], int i) {
	if (strcmp(argv[i], "-n") == 0) {			//�����ؼ� �����Ǵ� ��� ������ �̸�
		resultName = argv[i + 1];
	}
	else if (strcmp(argv[i], "-s") == 0) {		//�����Ϸ��� ������ ũ��(���� KB)
		blockSize = atoi(argv[i + 1]) << 10;
	}
	else if (strcmp(argv[i], "-o") == 0) {		//������ ������ �̸� �ֱ�
		fileName = argv[i + 1];
		inItO = 1;								//������ �̸��� �޾Ҵٴ� ǥ��
	}
	else if (strcmp(argv[i], "-d") == 0) {		//������ ������ ������ ���丮 ���
		strcpy(pathName, argv[i + 1]);
	}
	else {										//�߸��� �ɼ��� ȣ���Ұ�� ����
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
		chdir(pathName);		//��� ������ ������ ���丮�� �̵�
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
		fchdir(fd);				//�ʱ� ���丮�� �ٽ� �̵�
	}
	close(inFile);
	close(outFile);
	if (nread == -1)	return -4;
	else				return 0;
}

int main(int argc, char * argv[]) {
	int i;
	struct stat file_info;

	if (getcwd(pathName, LEN) == NULL)	perror("getcwd error\n");	//���ܵ� ������ ��ġ�� default ���� ���� �۾����� ���丮
	if ((fd = open(pathName, O_RDONLY)) == -1) {					//���� ���丮�� �̵��� �� �ֵ��� fd�� ����
		perror("directory open error\n");							//fd�� close���� �ʰ� ����
	}

	for (i = 1; i < argc; i += 2) if (option(argv, i) == -1) exit(-1);		//�ɼǰ��� �޴� �Լ�

	if (inItO == 0) {														//�ʼ� �ɼ� -o�� �ʿ�
		perror("�ʼ� �ɼ� '-o'�� �����ϴ�\n");
		exit(-1);
	}

	if (resultName == NULL)	resultName = fileName;							//-n �ɼ��� ���� ��� �����ؼ� �����Ǵ� ��� ������ �̸��� fimeName(default��)���� ����

	if (0 > stat(fileName, &file_info)) {
		perror("There is no file.\n");
		exit(-1);
	}
	else {
		fileSize = file_info.st_size;
		if ((fileSize / blockSize) > MAXNUM) {								//���� ������ �ִ� ����(1000��)�� �ѱ��
			perror("Block size is too small to cut\n");
			exit(-1);
		}
	}


	mkdir(pathName, 0755);													//drwxrwxr-x  = 0775�� ���丮 ����
	cutFile(pathName, fileName, resultName, blockSize);

	printf("���� ������ �̸� : %s\n", fileName);
	printf("���� ������ ���� : %dKB\n", blockSize >> 10);
	printf("��� ������ �̸� : %s\n", resultName);
	printf("��� ������ ����� ��� : %s\n", pathName);

	return 0;
}
