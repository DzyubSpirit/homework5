#include <iostream>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

typedef unsigned long long timestamp_t;

timestamp_t get_timestamp () {
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

#define EDGE_COUNT (2 * (P - 1))
#define ZH_SENDING 0
#define SH_SENDING 1
#define MAX_Z 2
#define SORTED_S 3
#define PART_C 4
#define TOTAL_MAX_Z 5
#define E_SENDING 6
#define MO_SENDING 7
#define MTH_SENDING 8
#define AH_SENDING 9

void mpiRun(MPI_Comm &graph_comm, MPI_Comm &topo_comm, int* index, int* edges);
void mergeSend(int rank, MPI_Comm graph_comm, int *Z, int *S, int *maxZ);
void maxSort(int &maxZ, int* Z, int* S);
int* mergeN(int n, int lengths[], int* arrays[]);

int N;
int P;
int H;
int mid;
int quarter;

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
	N = atoi(argv[1]);
  H = N / P;
  mid = P / 2;
  quarter = mid / 2;

  MPI_Comm graph_comm, topo_comm;
  int* index = new int[P];
  int* edges = new int[EDGE_COUNT];

  mpiRun(graph_comm, topo_comm, index, edges);

  MPI_Finalize();
  delete[] index;
  delete[] edges;
}

int* initVector(int n) {
  int *res = new int[n];
  for (int i = 0; i < n; i++) {
    res[i] = 1;
  }
  return res;
}

int* initMatrix(int n) {
  return initVector(n * n);
}

void mpiRun(MPI_Comm &graph_comm, MPI_Comm &topo_comm, int* index, int* edges) {
	timestamp_t startTime = get_timestamp();
  index[0] = 2;
  for (int i = 1; i < mid - 1; i++) {
    index[i] = index[i - 1] + 3;
  }
  index[mid - 1] = index[mid - 2] + 2;

  for (int i = mid; i < P; i++) {
    index[i] = index[i - 1] + 1;
  }

  for (int i = 0; i < mid - 1; i++) {
    edges[i * 2] = i;
    edges[i * 2 + 1] = i + 1;
    edges[i * 2 + 2] = i;
    edges[i * 2 + 3] = mid + i;
  }
  edges[EDGE_COUNT - 2] = mid - 1;
  edges[EDGE_COUNT - 2] = P - 1;

  MPI_Graph_create(MPI_COMM_WORLD, P, index, edges, 0, &graph_comm);

  MPI_Comm_dup(graph_comm, &topo_comm);

  int topo_type;
  MPI_Topo_test(topo_comm, &topo_type);

  if (topo_type != MPI_GRAPH) {
    cout << "Topo type error" << endl;
    return;
  }
 
  int rank;
  MPI_Comm_rank(graph_comm, &rank);

  int H = N / P;
  MPI_Status status;
  int maxZ, *Ei, *MOi, *MTh, *Sh, *Ah;
  Ei = new int[N];
  MOi = new int[N*N];
  MTh = new int[N*H];
  Sh = new int[H];
  Ah = new int[H];
  if (rank == 0) {
    int *Z, *E, *MO, *MT, *S;
    Z = initVector(N);
    E = initVector(N);
    MO = initMatrix(N);
    MT = initMatrix(N);
    S = initVector(N);

    for (int i = 1; i < P; i++) {
      MPI_Send(Z + i * H, H, MPI_INT, i, ZH_SENDING, graph_comm);
      MPI_Send(S + i * H, H, MPI_INT, i, SH_SENDING, graph_comm);
    }

    maxSort(maxZ, Z, S);   
    mergeSend(rank, graph_comm, Z, S, &maxZ);

    for (int i = 1; i < P; i++) {
      MPI_Send(E, N, MPI_INT, i, E_SENDING, graph_comm);
      MPI_Send(MO, N * N, MPI_INT, i, MO_SENDING, graph_comm);
      MPI_Send(MT + i * H * N, H * N, MPI_INT, i, MTH_SENDING, graph_comm);
    }

    for (int i = 0; i < H; i++) {
      Sh[i] = S[i];
    }
    for (int i = 0; i < N; i++) {
      Ei[i] = E[i];
    }
    for (int i = 0; i < N * N; i++) {
      MOi[i] = MO[i];
    }
    for (int i = 0; i < N * H; i++) {
      MTh[i] = MT[i];
    }

    delete[] Z;
    delete[] E;
    delete[] MO;
    delete[] MT;
    delete[] S;
  } else {
    int *Z, *S;
    Z = new int[H];
    S = new int[H];
    MPI_Recv(Z, H, MPI_INT, 0, ZH_SENDING, graph_comm, &status);
    MPI_Recv(S, H, MPI_INT, 0, SH_SENDING, graph_comm, &status);

    maxSort(maxZ, Z, S);

    int rank1 = rank < mid ? rank : rank - mid;
    int sourceRank = mid & 1 ? quarter : rank1 < quarter ? quarter - 1 : quarter;

    if (rank < mid) {
      mergeSend(rank, graph_comm, Z, S, &maxZ);
      for (int i = 0; i < H; i++) {
        Sh[i] = S[i];
      }
    } else {
      MPI_Send(&maxZ, 1, MPI_INT, rank - mid, MAX_Z, graph_comm);
      MPI_Send(S, H, MPI_INT, rank - mid, SORTED_S, graph_comm);

      MPI_Recv(&maxZ, 1, MPI_INT, sourceRank, TOTAL_MAX_Z, graph_comm, &status);
      MPI_Recv(Sh, H, MPI_INT, sourceRank, PART_C, graph_comm, &status);
    }
    delete[] Z;
    delete[] S;

    MPI_Recv(Ei, N, MPI_INT, 0, E_SENDING, graph_comm, &status);
    MPI_Recv(MOi, N * N, MPI_INT, 0, MO_SENDING, graph_comm, &status);
    MPI_Recv(MTh, H * N, MPI_INT, 0, MTH_SENDING, graph_comm, &status);
  }

  for (int i = 0; i < H; i++) {
    Ah[i] = Sh[i];
    for (int j = 0; j < N; j++) {
      int mot = 0;
      for (int k = 0; k < N; k++) {
        mot += MOi[k * N + j] * MTh[i * N + k];
      }
      Ah[i] += maxZ * mot * Ei[j];
    }
  }

  MPI_Send(Ah, H, MPI_INT, mid - 1, AH_SENDING, graph_comm);

  if (rank == mid - 1) {
    int *A = new int[N];
    for (int i = 0; i < H; i++) {
      A[(mid - 1) * H + i] = Ah[i];
    }
    for (int i = 0; i < P; i++) {
      if (i != mid - 1) {
        MPI_Recv(A + i * H, H, MPI_INT, i, AH_SENDING, graph_comm, &status);
      }
    }
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
  
  delete[] Ei;
  delete[] MOi;
  delete[] MTh;
  delete[] Sh;
}

void maxSort(int &maxZ, int* Z, int* S) {
  maxZ = -2000000000;
  for (int i = 0; i < H; i++) {
    if (Z[i] > maxZ) {
      maxZ = Z[i];
    }
  }

  for (int i = H; i > 1; i--) {
    for (int j = 1; j < i; j++) {
      if (S[j - 1] > S[j]) {
        int c = S[j - 1];
        S[j - 1] = S[j];
        S[j] = c;
      }
    }
  }
}

void mergeSend(int rank, MPI_Comm graph_comm, int *Z, int *S, int *maxZP) {
  int maxZ = *maxZP;
  MPI_Status status;
  int maxAnotherZ;
  MPI_Recv(&maxAnotherZ, 1, MPI_INT, rank + mid, MAX_Z, graph_comm, &status);
  if (maxAnotherZ > maxZ) {
    maxZ = maxAnotherZ;
  }

  int* sortedS2 = new int[H];
  MPI_Recv(sortedS2, H, MPI_INT, rank + mid, SORTED_S, graph_comm, &status);

  int targetRank = rank < quarter ? rank + 1 : rank - 1;
  int sourceRank = 2 * rank - targetRank;

  if (rank < mid - 1 && rank > 0) {
    MPI_Recv(&maxAnotherZ, 1, MPI_INT, sourceRank, MAX_Z, graph_comm, &status);
    if (maxAnotherZ > maxZ) {
      maxZ = maxAnotherZ;
    }
    int sortedSSize = (rank < quarter ? 2 * rank : 2 * (mid - 1 - rank)) * H;
    int* sortedS3 = new int[sortedSSize];
    MPI_Recv(sortedS3, sortedSSize, MPI_INT, sourceRank,
        SORTED_S, graph_comm, &status);

    if (mid & 1 == 1 && rank == quarter) {
      int maxAnotherZ;
      MPI_Recv(&maxAnotherZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm, &status);
      if (maxAnotherZ > maxZ) {
        maxZ = maxAnotherZ;
      }
      int *sortedS4 = new int[sortedSSize];
      MPI_Recv(sortedS4, sortedSSize, MPI_INT, targetRank, SORTED_S, graph_comm, &status);

      int lengths[] = { H, H, sortedSSize, sortedSSize };
      int* arrays[] = { S, sortedS2, sortedS3, sortedS4 };
      int *merged = mergeN(4, lengths, arrays);

      for (int i = 0; i < P; i++) {
        if (i != rank) {
          *maxZP = maxZ;
          MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(merged + i * H, H, MPI_INT, i, PART_C, graph_comm);
        }
      }

      for (int i = 0; i < H; i++) {
        S[i] = merged[i + rank * H];
      }

      delete[] merged;
      delete[] sortedS4;
      return;
    }
    int mergedSize = sortedSSize + 2 * H;
    int* merged;
    int lengths[] = { H, H, sortedSSize };
    int* arrays[] = { S, sortedS2, sortedS3 };
    merged = mergeN(3, lengths, arrays);
    int anotherMaxZ;
    MPI_Send(&anotherMaxZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
    if (anotherMaxZ > maxZ) {
      maxZ = anotherMaxZ;
    }
    MPI_Send(merged, mergedSize, MPI_INT, targetRank, SORTED_S, graph_comm);


    if ((mid & 1) == 0 && (rank == quarter - 1 || rank == quarter)) {
      int maxAnotherZ;
      MPI_Recv(&maxAnotherZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm, &status);
      if (maxAnotherZ > maxZ) {
        maxZ = maxAnotherZ;
      }
      int *sortedS2 = new int[mid * H];
      MPI_Recv(sortedS2, mid * H, MPI_INT, targetRank, SORTED_S, graph_comm, &status);

      int* totalS;
      int lengths[] = { mid * H, mid * H };
      int* arrays[] = { merged, sortedS2 };
      totalS = mergeN(2, lengths, arrays);
      *maxZP = maxZ;
      if (rank == quarter - 1) {
        for (int i = 0; i < quarter - 1; i++) {
          MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
        }
        for (int i = mid; i < mid + quarter; i++) {
          MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
        }
      } else {
        for (int i = quarter + 1; i < mid; i++) {
          MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
        }
        for (int i = mid + quarter; i < P; i++) {
          MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
        }
      }
      for (int i = 0; i < H; i++) {
        S[i] = totalS[i + rank * H];
      }

      delete[] merged;
      delete[] totalS;
      delete[] sortedS2;
      return;
    }
    delete[] sortedS3;
  } else {
    int lengths[] = { H, H };
    int* arrays[] = { S, sortedS2 };
    int* merged = mergeN(2, lengths, arrays);
    *maxZP = maxZ;
    MPI_Send(maxZP, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
    MPI_Send(merged, 2 * H, MPI_INT, targetRank, SORTED_S, graph_comm);

    delete[] merged;
  }
  delete[] sortedS2;
  
  int sortedSourceRank = (mid & 1) == 0
    ? rank < quarter ? quarter - 1 : quarter
    : quarter;
  MPI_Recv(maxZP, 1, MPI_INT, sortedSourceRank, TOTAL_MAX_Z, graph_comm, &status);
  MPI_Recv(S, H, MPI_INT, sortedSourceRank, PART_C, graph_comm, &status);
}

int* mergeN(int n, int lengths[], int* arrays[]) {
  int* coeffs = new int[n];
  int totalLength = 0;
  for (int i = 0; i < n; i++) {
    coeffs[i] = 0;
    totalLength += lengths[i];
  }
  int* merged = new int[totalLength];

  for (int i = 0; i < totalLength; i++) {
    int max_i = -1;
    int max;
    for (int j = 0; j < n; j++) {
      if (coeffs[j] < lengths[j] && (max_i == -1 || arrays[j][coeffs[j]] > max)) {
        max_i = j;
        max = arrays[j][coeffs[j]];
        coeffs[j]++;
      }
    }
    merged[i] = max;
  }

  delete[] coeffs;
  return merged;
}
