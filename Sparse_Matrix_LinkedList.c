#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<process.h>
#include<time.h>
#include<math.h>
#define M 500//size limit of array.
#define N 500//second dimension of array.

typedef struct temp {
	float var;
	struct temp *next;
}link;
typedef struct tem {
	link *front;
	link *rear;
}linkends;//one pair of link ends are required per linked list
typedef struct {
	linkends sa;
	linkends ija;
}matrix;

int sprsin(float a[][N + 1], int m, int n, float thresh, matrix *mat);
void sprsax(matrix mat, float x[], float b[], long m, long n);
void sprstx(matrix mat, float x[], float b[], long m, long n);
void get_input(FILE *f, float a[][N + 1], float v[], float vt[], int m, int n);
void nrerror(char string[]);
void initialize(matrix *m);
void list_insert(linkends *m, float value);
void list_destroy(linkends *m);//to free heap memory after the execution of the program.
float list_pop(linkends *m);
int smaller(int m, int n) { return m < n ? m : n; }

void main() {
	matrix A;//matrix
	initialize(&A);
	float big_array[M + 1][N + 1];//i'm using 1 based indexing
	int compact_size;//size of compact form of matrix's array viz. sa, aij.
	float vector[N + 1], vector_t[M + 1], result[M + N];//result's size is bigger of M and N
	FILE *ifptr;
	ifptr = fopen("input2a.dat", "r");//input file
	if (ifptr == NULL) {
		printf("\nFile named \"input2a.dat\" not found\n");
		exit(0);
	}
	FILE *ofptr = fopen("output2.dat", "w");//output file
	int choice = 1, m, n;
	printf("\n\n1.Enter the values of matrix size m and n\n2.Default choice m = n = 500\nEnter your choice:");
	scanf("%d", &choice);
	switch (choice) {
	case 1:
		printf("\nm=");
		scanf("%d", &m);
		printf("\nn=");
		scanf("%d", &n);
		if (m > M || n > N) {
			printf("\nSize is too large. Array size limit macros need to be changed.\n");
			exit(0);
		}
		break;
	case 2:
		m = n = 500;
		break;
	default:
		printf("\nWrong Choice\n");
		exit(0);
	}
	get_input(ifptr, big_array, vector, vector_t, m, n);
	fclose(ifptr);
	compact_size = sprsin(big_array, m, n, 0.000001, &A);//Converting matrix to simple form

	/*
	clock_t start = clock();
	for (unsigned long i = 0; i < 1000; i++)
	sprsax(A, vector, result, m, n);//multiplication with the vector.
	clock_t end = clock();
	printf("\nTime taken for multiplication Ax = %.20lf ms\n\n", ((double)(end - start)) / CLOCKS_PER_SEC);
	*/
	fprintf(ofptr, "Ax:\n");
	sprsax(A, vector, result, m, n);//multiplication with the vector.
	for (int j = 1; j <= m; j++) {
		fprintf(ofptr, "%f\t\t", result[j]);//printing to output file
	}

	fprintf(ofptr, "\n\nxTA:\n");
	sprstx(A, vector_t, result, m, n);//multiplication with the transpose.
	for (int j = 1; j <= n; j++) {
		fprintf(ofptr, "%f\t\t", result[j]);//printing to output file
	}

	link *ptr = A.sa.front;
	fprintf(ofptr, "\n\nSA array:\n");
	for (int j = 1; j <= compact_size; j++) {//printing sa array
		fprintf(ofptr, "%f\t\t", ptr->var);//printing to output file
		ptr = ptr->next;
	}
	ptr = A.ija.front;

	fprintf(ofptr, "\n\nIJA array:\n");
	for (int j = 1; j <= compact_size; j++) {//printing ija array
		fprintf(ofptr, "%d\t\t\t", (int)ptr->var);//printing to output file
		ptr = ptr->next;
	}

	fclose(ofptr);
	list_destroy(&A.ija);//to restore heap memory
	list_destroy(&A.sa);
	printf("\nFinished.\n");
	_getch();
}

void get_input(FILE *f, float a[][N + 1], float v[], float vt[], int m, int n) {
	for (int i = 1; i <= m; i++) {
		for (int j = 1; j <= n; j++) {
			a[i][j] = 0;//default value
		}
	}
	char ch;
	int i, j;
	while (fscanf(f, "%d", &i) == 0) {
		do {//this is done to skip over the sentences in the beginning fo the file
			fscanf(f, "%c", &ch);
		} while (ch != '\n');//go to next line
	}
	fscanf(f, "%d", &j);
	fscanf(f, "%f", &a[i + 1][j + 1]);//converting from zero to one based indexing

	while (fscanf(f, "%d", &i) == 1) {
		fscanf(f, "%d", &j);
		fscanf(f, "%f", &a[i + 1][j + 1]);
	}

	for (int i = 1; i <= n; i++) {
		v[i] = i;
	}
	for (int i = 1; i <= m; i++) {
		vt[i] = i;
	}
}

void nrerror(char string[]) {
	printf("\n%s\n", string);
	exit(0);
}

void initialize(matrix *m) {
	m->ija.front = NULL;
	m->ija.rear = NULL;
	m->sa.front = NULL;
	m->sa.rear = NULL;
}

void list_insert(linkends *m, float value) {
	link *ptr = (link*)malloc(sizeof(link));
	ptr->var = value;
	ptr->next = NULL;
	if (m->front == NULL) {
		m->front = m->rear = ptr;
	}
	else {
		m->rear->next = ptr;
		m->rear = ptr;
	}
}

void list_destroy(linkends *m) {
	while (m->front != m->rear) {
		link *ptr = m->front;
		m->front = m->front->next;
		free(ptr);
	}
	free(m->front);
	m->front = m->rear = NULL;
}

float list_pop(linkends *m) {
	if (m->front == NULL) nrerror("Queue is empty");
	float temp = m->front->var;
	if (m->front == m->rear) {
		free(m->front);
		m->front = m->rear = NULL;
	}
	else {
		link *ptr = m->front;
		m->front = m->front->next;
		free(ptr);
	}
	return temp;
}

int sprsin(float a[][N + 1], int m, int n, float thresh, matrix *mat) {
	/*
	Converts a square matrix a[1..n][1..n] into row - indexed sparse storage mode.Only elements
	of a with magnitude thresh are retained.Output is in two linear arrays with dimension
	nmax(an input parameter) : sa[1..] contains array values, indexed by ija[1..].The
	number of elements filled of sa and ija on output are both ija[ija[1] - 1] - 1 (see text).
	*/
	int i, j;
	unsigned long k;
	int x = smaller(m, n);
	for (j = 1; j <= x; j++) {
		list_insert(&mat->sa, a[j][j]);//sa[j] = a[j][j];// Store diagonal elements.
	}
	list_insert(&mat->ija, m + 2);//ija[1] = n + 2;// Index to 1st rowoff - diagonal element, if any.
	k = m + 1;
	for (int i = x + 1; i <= m + 1; i++) {
		list_insert(&mat->sa, 0);//sa[k] = 0;//any value can be put here.
	}
	linkends queue;//the queue which holds values to be inserted later
	queue.front = queue.rear = NULL;

	for (i = 1; i <= m; i++) {
		//Loop over rows.
		for (j = 1; j <= n; j++) {
			//Loop over columns.
			if (fabs(a[i][j]) >= thresh && i != j) {
				k++;
				list_insert(&mat->sa, a[i][j]);// Store off - diagonal elements and their columns.
				list_insert(&queue, j); //ija[k] = j;
			}
		}
		list_insert(&mat->ija, k + 1);//ija[i + 1] = k + 1;// As each row is completed, store index to
	}// next.
	while (queue.front != NULL) {
		list_insert(&mat->ija, list_pop(&queue));
	}
	return k;
}

void sprsax(matrix mat, float x[], float b[], long m, long n) {
	/*
	Multiply a matrix in row - index sparse storage arrays sa and ija by a vector x[1..n], giving
	a vector b[1..n].
	*/
	long i, k;
	int y = smaller(m, n);
	link *sa = mat.sa.front;
	link *ija = mat.ija.front;
	link *ptr = ija;
	for (int i = n + 1; i <= m; i++) {
		b[i] = 0;
	}
	for (i = 1; i <= n; i++) {
		b[i] = sa->var * x[i];// Start with diagonal term.
		sa = sa->next;
		ptr = ptr->next;
	}
	for (int i = y + 1; i <= m + 1; i++) {//to skip over useless terms
		sa = sa->next;
		ptr = ptr->next;
	}
	for (i = 1; i <= m; i++) {
		for (k = (int)ija->var; k <= (int)ija->next->var - 1; k++) {// Loop over off - diagonal terms.
			b[i] += sa->var * x[(int)ptr->var];//b[i] += sa[k] * x[ija[k]];
			sa = sa->next;
			ptr = ptr->next;
		}
		ija = ija->next;
	}
}

void sprstx(matrix mat, float x[], float b[], long m, long n) {
	/*
	Multiply the transpose of a matrix in row - index sparse storage arrays sa and ija by a vector
	x[1..n], giving a vector b[1..n].
	*/
	long i, k;
	int y = smaller(m, n);
	link *sa = mat.sa.front;
	link *ija = mat.ija.front;
	link *ptr = ija;
	for (int i = 0; i <= m; i++) {
		b[i] = 0;
	}
	for (i = 1; i <= n; i++) {
		b[i] = sa->var * x[i];// Start with diagonal term.
		sa = sa->next;
		ptr = ptr->next;
	}
	for (int i = y + 1; i <= m + 1; i++) {
		sa = sa->next;//because we want to skip the n+1th term
		ptr = ptr->next;
	}
	for (i = 1; i <= m; i++) {
		//Loop over off - diagonal terms.
		for (k = (int)ija->var; k <= (int)ija->next->var - 1; k++) {
			b[(int)ptr->var] += sa->var * x[i];//b[ija[k]] += sa[k] * x[i];
			sa = sa->next;
			ptr = ptr->next;
		}
		ija = ija->next;
	}
}