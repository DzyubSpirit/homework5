#include <iostream>
#include <mpi.h>

using namespace std;

#define N 100
#define EDGE_COUNT (2 * (P - 1))
#define Z_SENDING 0
#define S_SENDING 1
#define MAX_Z 2
#define SORTED_S 3
#define PART_S 4
#define TOTAL_MAX_Z 5

void mpiRun(MPI_Comm &graph_comm, MPI_Comm &topo_comm, int* index, int* edges);
void mergeSend(int rank, MPI_Comm graph_comm, int *Z, int *S, int maxZ);
void maxSort(int &maxZ, int* Z, int* S);
int* mergeN(int n, int lengths[], int* arrays[]);

int P;
int H;
int mid;
int quarter;

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
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
  edges[EDGE_COUNT - 2] = mid -1;
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
  if (rank == 0) {
    int *Z, *E, *MO, *MT, *S;
    Z = initVector(N);
    E = initVector(N);
    MO = initMatrix(N);
    MT = initMatrix(N);
    S = initVector(N);

    for (int i = 1; i < P; i++) {
      MPI_Send(Z + i * H, H, MPI_INT, i, Z_SENDING, graph_comm);
      MPI_Send(S + i * H, H, MPI_INT, i, S_SENDING, graph_comm);
    }

    int maxZ;
    maxSort(maxZ, Z, S);   
    mergeSend(rank, graph_comm, Z, S, maxZ);
    cout << rank << " end" << endl;

    delete[] Z;
    delete[] E;
    delete[] MO;
    delete[] MT;
    delete[] S;
  } else {
    int *Z, *S;
    Z = new int[H];
    S = new int[H];
    MPI_Recv(Z, H, MPI_INT, 0, Z_SENDING, graph_comm, &status);
    MPI_Recv(S, H, MPI_INT, 0, S_SENDING, graph_comm, &status);

    int maxZ;
    maxSort(maxZ, Z, S);

    int rank1 = rank < mid ? rank : rank - mid;
    int sourceRank = mid & 1 ? quarter : rank1 < quarter ? quarter - 1 : quarter;

    if (rank < mid) {
      mergeSend(rank, graph_comm, Z, S, maxZ);
      cout << rank << " end" << endl;
    } else {
      MPI_Send(&maxZ, 1, MPI_INT, rank - mid, MAX_Z, graph_comm);
      MPI_Send(S, H, MPI_INT, rank - mid, SORTED_S, graph_comm);

      int *partS = new int[H];
      cout << rank << " waits for " << sourceRank << endl;
      MPI_Recv(&maxZ, 1, MPI_INT, sourceRank, TOTAL_MAX_Z, graph_comm, &status);
      MPI_Recv(partS, H, MPI_INT, sourceRank, PART_S, graph_comm, &status);
      cout << rank << " end" << endl;
      delete[] partS;
    }
  }
}

void maxSort(int &maxZ, int* Z, int* S) {
  maxZ = 0;
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

void mergeSend(int rank, MPI_Comm graph_comm, int *Z, int *S, int maxZ) {
  MPI_Status status;
  int maxAnotherZ;
  MPI_Recv(&maxAnotherZ, 1, MPI_INT, rank + mid, MAX_Z, graph_comm, &status);
  if (maxAnotherZ > maxZ) {
    maxZ = maxAnotherZ;
  }

  int* sortedS2 = new int[H];
  MPI_Recv(sortedS2, H, MPI_INT, rank + mid, SORTED_S, graph_comm, &status);
  cout << rank << " first recv" << endl;

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
      cout << rank << " waits for " << sourceRank << endl;
      MPI_Recv(&maxAnotherZ, 1, MPI_INT, sourceRank, MAX_Z, graph_comm, &status);
      if (maxAnotherZ > maxZ) {
        maxZ = maxAnotherZ;
      }
      int *sortedS4 = new int[mid * H];
      MPI_Recv(&sortedS4, (mid - 1) * H, MPI_INT, sourceRank, SORTED_S, graph_comm, &status);
      cout << rank << " received for " << sourceRank << endl;

      int lengths[] = { H, H, (mid - 1) * H, (mid - 1) * H };
      int* arrays[] = { S, sortedS2, sortedS3, sortedS4 };
      int *merged = mergeN(4, lengths, arrays);

      for (int i = 0; i < P; i++) {
        if (i != rank) {
          MPI_Send(&maxZ, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(merged + i * H, H, MPI_INT, i, PART_S, graph_comm);
          cout << rank << " sends to " << i << endl;
        }
      }
        
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
    cout << rank << " sends to " << targetRank << endl;


    if (mid & 1 == 0 && (rank == quarter - 1 || rank == quarter)) {
      int maxAnotherZ;
      MPI_Recv(&maxAnotherZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm, &status);
      if (maxAnotherZ > maxZ) {
        maxZ = maxAnotherZ;
      }
      int *sortedS2 = new int[mid * H];
      MPI_Recv(&sortedS2, mid * H, MPI_INT, targetRank, SORTED_S, graph_comm, &status);

      int* totalS;
      int lengths[] = { mergedSize, mergedSize };
      int* arrays[] = { merged, sortedS2 };
      totalS = mergeN(2, lengths, arrays);
      if (rank == quarter - 1) {
        for (int i = 0; i < quarter - 1; i++) {
          MPI_Send(&maxZ, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + i * H, H, MPI_INT, i, PART_S, graph_comm);
          MPI_Send(&maxZ, 1, MPI_INT, i + mid, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + (i + mid) * H, H, MPI_INT, i + mid, PART_S, graph_comm);
          cout << rank << " sends to " << i << " " << i + mid << endl;
        }
      } else {
        for (int i = quarter + 1; i < mid; i++) {
          MPI_Send(&maxZ, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + i * H, H, MPI_INT, i, PART_S, graph_comm);
          MPI_Send(&maxZ, 1, MPI_INT, i + mid, TOTAL_MAX_Z, graph_comm);
          MPI_Send(totalS + (i + mid) * H, H, MPI_INT, i + mid, PART_S, graph_comm);
          cout << rank << " sends to " << i << " " << i + mid << endl;
        }
      }

      delete[] merged;
      delete[] totalS;
      delete[] sortedS2;
    }
    delete[] sortedS3;
  } else {
    int lengths[] = { H, H };
    int* arrays[] = { S, sortedS2 };
    int* merged = mergeN(2, lengths, arrays);
    MPI_Send(&maxZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
    MPI_Send(merged, 2 * H, MPI_INT, targetRank, SORTED_S, graph_comm);
    cout << rank << " sends to " << targetRank << endl;

    delete[] merged;
  }
  delete[] sortedS2;

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
    int min_i = -1;
    int min;
    for (int j = 0; j < n; j++) {
      if (coeffs[j] < lengths[j] && (min_i == -1 || arrays[j][coeffs[j]] < min)) {
        min_i = j;
        min = arrays[j][coeffs[j]];
      }
    }
    merged[i] = min;
  }

  delete[] coeffs;
  return merged;
}
