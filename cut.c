#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#define PERM 0664 //-rw-rw-r--
#define LEN	1024

int fd;
char pathName[LEN];			//���ܵ� ������ ��ġ
char * fileName = NULL;
char * resultName = NULL;
int size = 512;				//default size = 512KB
int inItO = 0;				//������ �̸��� �ʼ��� �޾ƾ��ϴ� �����̹Ƿ� �޾Ҵ��� �ȹ޾Ҵ��� ���α׷����� �˷��ֱ� ���� ����
char fileNum[4] = "000";
char outputName[LEN];

int option(char * argv[], int i) {
	if (strcmp(argv[i], "-n") == 0) {			//�����ؼ� �����Ǵ� ��� ������ �̸�
		resultName = argv[i + 1];
	}
	else if (strcmp(argv[i], "-s") == 0) {		//�����Ϸ��� ������ ũ��(���� KB)
		size = atoi(argv[i + 1]);
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

//a�� x���� ���ϴ� �Լ�
int power(int a, int x) {
	int i;
	int result = 1;
	for (i = 0; i < x; i++) {
		result *= a;
	}
	return result;
}
//num�� ���ڷ� �޾Ƽ� ���� ���� �ڿ� �ٿ��� ���ڹ��ڿ��� ��ȯ
char * getFileNum(int num) {
	char * fn = fileNum;
	int number;
	int i;
	int p;	//POWER��
	for (i = 0; i < 3; i++) {
		p = power(10, 2 - i);
		number = num / p;
		num -= (number * p);
		fn[i] = (char)number + '0';
	}
	return fn;
}

char * strCar(char * des, char * src) {
	int i,j;
	char * opN = outputName;
	for (i = 0; des[i] != '\0'; ++i)		{ opN[i] = des[i]; }

	for (j = 0; src[j] != '\0'; ++j, ++i)	{ opN[i] = src[j]; }

	opN[i]= '\0';						//������ ���ڿ��� ���� ��Ÿ���� ����

	return opN;
}

int cutFile(const char * pathName, const char * fileName, char * resultName, int BUFSIZE) {
	int inFile, outFile;
	ssize_t nread;
	char * buffer = malloc((char)BUFSIZE);
	int num = 0;

	if ((inFile = open(fileName, O_RDONLY)) == -1)	return -1;

	while ((nread = read(inFile, buffer, BUFSIZE)) > 0) {
		chdir(pathName);		//��� ������ ������ ���丮�� �̵�
		if ((outFile = open(strCar(resultName, getFileNum(num++)), O_WRONLY | O_CREAT | O_TRUNC, PERM)) == -1) {	//PERM = 0664 ( -rw-rw-r--)
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
	//open�Լ��� ���Ͽ� ������ directory�� ���� file descriptor
	int i;
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

	printf("���� ������ �̸� : %s\n", fileName);
	printf("���� ������ ���� : %d\n", size);
	printf("��� ������ �̸� : %s\n", resultName);
	printf("��� ������ ����� ��� : %s\n", pathName);
	mkdir(pathName,0755);														//drwxrwxr-x  = 0775�� ���丮 ����

	cutFile(pathName, fileName, resultName, size);
	//getcwd(testPath, LEN);
	//printf("�̵� �� ���� ���丮 : %s\n", testPath);

	//getcwd(testPath, LEN);
	//printf("fchdir �� ���� ���丮 : %s\n", testPath);

	return 0;
}