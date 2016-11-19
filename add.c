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
char * fileName = NULL;		//���ܵ� ������ ���� �̸�(�ڿ� ���� ����)
char * resultName = NULL;	//������ ������ �̸�
int blockSize;
int inItO = 0;				//������ �̸��� �ʼ��� �޾ƾ��ϴ� �����̹Ƿ� �޾Ҵ��� �ȹ޾Ҵ��� ���α׷����� �˷��ֱ� ���� ����
int inItX = 0;				//-x�ɼ��� ���Ե� �ִ°�?
int adjust_Index = 0;		//-x �ɼ��� ������ �� �� �� �� argv�� �ε������� �ϳ� ���ش�. why? i ���� 2�� �������� ����.-x�ɼ��� ���� ���ڿ��� ���� �ʾƼ� �ɼ��� ��ġ�� Ȧ��->¦���� �ٲ��.
char input[LEN];			//���ܵ� ���ϵ��� �̸�(���� ����)
int inputFileNum = 0;		//���ܵ� ������ ��ȣ

int option(char * argv[], int i) {
	if (strcmp(argv[i], "-n") == 0) {			//���ļ� �����Ǵ� ��� ������ �̸�
		resultName = argv[i + 1];
	}
	else if (strcmp(argv[i], "-x") == 0) {
		adjust_Index = 1;						//argv�� �ε������� �ϳ� ���༭ �ɼǰ��� �ε��� ��ġ�� ���� �ϱ� ����
		inItX = 1;
	}
	else if (strcmp(argv[i], "-o") == 0) {		//���ܵ��ִ� ������ ���� �̸�
		fileName = argv[i + 1];
		inItO = 1;								//������ �̸��� �޾Ҵٴ� ǥ��
	}
	else if (strcmp(argv[i], "-d") == 0) {		//���ܵ� ������ �ִ� ��ġ, ��ģ ������ ������ ���丮 ���
		strcpy(pathName, argv[i + 1]);
	}
	else {										//�߸��� �ɼ��� ȣ���Ұ�� ����
		perror("Wrong option!\n");
		return -1;
	}
	return 1;
}

int mergeFile(const char * pathName, const char * fileName, char * resultName, int BUFSIZE) {
	int inFile, outFile;
	ssize_t nread;
	char * buffer = (char *)malloc(sizeof(char)*BUFSIZE);

	while ((inFile = open(input, O_RDONLY)) != -1) {			//���ܵ� ���ϵ��� ��� ���������� �б�
		nread = read(inFile, buffer, BUFSIZE);
		if (inItX) unlink(input);								//-x�ɼ��� ���� ��� ���ܵ� ������ �����Ѵ�.
		if ((outFile = open(resultName, O_WRONLY | O_CREAT | O_APPEND, PERM)) == -1) {	//PERM = 0664 ( -rw-rw-r--)
			close(inFile);																//O_APPEND : ������ ���� �̾ ���� ����
			return -2;																	//��� ���� �̸����� ����
		}
		write(outFile, buffer, nread);							//��� ���Ͽ� ���ܵ� ���ϵ��� ���ʴ�� �̾ ����.
		sprintf(input, "%s%03d", fileName, inputFileNum++);		//���� ���ܵ� ������ �б� ���� input�� ����
	}
	close(inFile);
	close(outFile);
	free(buffer);					//��������
	if (nread == -1)	return -4;
	else				return 0;
}

int main(int argc, char * argv[]) {
	int i;
	struct stat file_info;											//������ ����

	if (getcwd(pathName, LEN) == NULL)	perror("getcwd error\n");	//���ܵ� ������ ��ġ�� default ���� ���� �۾����� ���丮
	if ((fd = open(pathName, O_RDONLY)) == -1) {					//���� ���丮�� �̵��� �� �ֵ��� fd�� ����
		perror("directory open error\n");							//fd�� close���� �ʰ� ����
	}

	for (i = 1; i < argc; i += 2) {
		if (option(argv, i) == -1) exit(-1);		//�ɼǰ��� �޴� �Լ�
		if (adjust_Index) {							//-x �ɼ��� ������ argv�� �ε������� �ϳ� ���ش�. why? i ���� 2�� �������� ����.-x�ɼ��� ���� ���ڿ��� ���� �ʾƼ� �ɼ��� ��ġ�� Ȧ��->¦���� �ٲ��.
			adjust_Index = 0;
			i--;
		}	
	}

	if (inItO == 0) {														//�ʼ� �ɼ� -o�� �ʿ�
		perror("�ʼ� �ɼ� '-o'�� �����ϴ�\n");
		exit(-1);
	}
	if (resultName == NULL)	resultName = fileName;							//-n �ɼ��� ���� ��� �����ؼ� �����Ǵ� ��� ������ �̸��� fimeName(default��)���� ����
	chdir(pathName);														//���ܵ� ������ �ִ� ���丮�� �̵�
	sprintf(input, "%s%03d", fileName, inputFileNum++);						//-o�ɼ����� ���� ������ �̸��� "000"�� �ٿ��ش�.
	if (0 > stat(input, &file_info)) {										//���ܵ� ������ ������ �����´�.
		perror("There is no cut_File.\n");									//���ܵ� ������ �������� ���� ��� ����, ����
		exit(-1);
	}
	blockSize = file_info.st_size;

	mergeFile(pathName, fileName, resultName, blockSize);

	printf("���� ������ �̸� : %s\n", fileName);
	printf("��� ������ �̸� : %s\n", resultName);
	printf("���ܵ� ������ ��ġ, ��� ������ ������ ��� : %s\n", pathName);

	return 0;
}
