/*
 * ИП-42, Дзюба Влад
 */
#include <omp.h>
#include <iostream>

using namespace std;

#define N 12
#define P 6

int* initVector(int n);
int* initMatrix(int n);
int max(int *arr, int b, int e);

int main() {
  // Initialize data
  int *Z, *B, e, *C, *MO, *MK, *A = new int[N];
  int z = 0;

#pragma omp parallel num_threads(P)
  {
    int thread_num = omp_get_thread_num();
    cout << "Thread " << thread_num + 1 << " started" << endl;
  // Initializes first thread input
    switch (thread_num) {
      case 0: {
        Z = initVector(N);
        B = initVector(N);
      } break;
  // Initializes third thread input
      case 2: {
        C = initVector(N);
        e = 1;
      } break;
  // Initializes sixth thread input
      case 5: {
        MO = initMatrix(N);
        MK = initMatrix(N);
      } break;
    }
  // Finds maximum of Z
#pragma omp barrier
    for (int i = 0; i < N; i++) {
#pragma omp task shared(z)
      {
#pragma omp critical(zCalculation)
        z = Z[i] > z ? Z[i] : z;
      }
    }
#pragma omp taskwait
    // Calculates A
#pragma omp for
    for (int i = 0; i < N; i++) {
      int zi, ei, *Ci, *MOi;
      // Copies shared data
#pragma omp read atomic
      zi = z;
#pragma omp read atomic
      ei = e;
#pragma omp read atomic
      Ci = C;
#pragma omp read atomic
      MOi = MO;
      // Calculates A
      A[i] = zi * B[i]; 
      for (int j = 0; j < N; j++) {
        int ok = 0;
        for (int k = 0; k < N; k++) {
          ok += MOi[j * N + k] * MK[k * N + i];
        }
        A[i] += ei * ok * Ci[j];
      }
    }
#pragma omp master
    if (N < 20) {
      for (int i = 0; i < N; i++) {
        cout << A[i] << " ";
      }
      cout << endl;
    }
    cout << "Thread " << thread_num + 1 << " ended" << endl;
  }
}

// Initializes vectors
int* initVector(int n) {
  int* res = new int[n];
  for (int i = 0; i < n; i++) {
    res[i] = 1;
  }
  return res;
}

// Initializes matricies
int* initMatrix(int n) {
  int* res = new int[n * n];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      res[i * n + j] = 1;
    }
  }
  return res;
}
