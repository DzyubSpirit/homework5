/*
// main
// Author:
//    Dzyuba Vlad, IP-42
*/
#include <iostream>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

typedef unsigned long long timestamp_t;

timestamp_t get_timestamp () {
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

using namespace std;

int N, P, H;

// functions for generating structures
int* readVec();
int* readMat();
// functions for copying structures
int* veccpy(int *src);
int* matcpy(int *src);

int main(int argc, char* argv[]) {
	N = atoi(argv[1]);
	P = atoi(argv[2]);
	H = N / P;
	// input data
	int *Z, *MO, *E, *S, *MT;
	// output data
	int *A = new int[N];

	// intermidiate data
	int z = -65536, *B = new int[N], *C = new int[N];
	timestamp_t startTime = get_timestamp();
	#pragma omp parallel num_threads(P)
	{
		int tid = omp_get_thread_num();
		// generate data if first or last thread
		if (tid == 0) {
			Z = readVec();
			MO = readMat();
		}
		if (tid == P - 1) {
			E = readVec();
			S = readVec();
			MT = readMat();
		}
		// finding maximum of Z
		#pragma omp barrier
		for (int i = 0; i < P; i++) {
			int mval = Z[i*H];
			for (int j = i*H+1; j < (i+1)*H; j++) {
				mval = Z[j] > mval ? Z[j] : mval;
			}
			#pragma omp critical(zUpdate)
			if (mval > z) {
				z = mval;
			}
		}
		#pragma omp barrier
		// calculating B
		int *MO1, *E1, z1;
		#pragma omp critical(dataCopy)
		{
			MO1 = matcpy(MO);
		 	E1 = veccpy(E);
		 	z1 = z;
		}
		#pragma omp for
		for (int i = 0; i < P; i++) {
			for (int j = i * H; j < (i + 1) * H; j++) {
				B[j] = 0;
				for (int k = 0; k < N; k++) {
					int x = 0;
					for (int l = 0; l < N; l++) {
						x += MO1[k*N+l] * MT[l*N+j];
					}
					B[j] += x * E1[k];
				}
				B[j] *= z1;
			}
		}
		#pragma omp barrier
		// sorting parts of S
		#pragma omp for
		for (int i = 0; i < P; i++) {
			for (int j = 1; j < H; j++) {
				for (int k = i * H; k < (i + 1) * H - j; k++) {
					if (S[k] > S[k + 1]) {
						int x = S[k];
						S[k] = S[k + 1];
						S[k + 1] = x;
					}
				}
			}
		}
		#pragma omp barrier
		// sorting whole S and stores itto C
		#pragma omp single
		{
			int* ind = new int[N];
			for (int i = 0; i < N; i++) {
				ind[i] = 0;
			}
			for (int i = 0; i < N; i++) {
				int mini = -1;
				for (int j = 0; j < N; j++) {
					if (ind[j] < N) {
						if (mini == -1 || S[ind[j]] < S[ind[mini]]) {
							mini = j;
						}
					}
				}
				C[i] = S[ind[mini]];
				ind[mini]++;
			}
		}
		#pragma omp barrier
		// calculating A
		#pragma omp for
		for (int i = 0; i < N; i++) {
			A[i] = B[i] + C[i];
		}
	}
	// print A
	if (N <= 20) {
		for (int i = 0; i < N; i++) {
			cout << A[i] << " ";
		}
		cout << endl;
	} else {
		timestamp_t endTime = get_timestamp();
		cout << (endTime - startTime) / 1000000.0 << endl;
	}
	delete[] A;
}

int* readArr(int n) {
	int* res = new int[n];
	for (int i = 0; i < n; i++) {
		res[i] = 1;
	}
	return res;
}

int* readVec() { return readArr(N); }
int* readMat() { return readArr(N*N); }

int* arrcpy(int *src, int n) {
	int *res = new int[n];		
	for (int i = 0; i < n; i++) {
		res[i] = src[i];
	}
	return res;
}

int* veccpy(int *src) { return arrcpy(src, N); }
int* matcpy(int *src) { return arrcpy(src, N * N); }
