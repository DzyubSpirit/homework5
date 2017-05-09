1 #include <iostream>
2 #include <mpi.h>
3 #include <string.h>
4 #include <stdlib.h>
5 #include <sys/time.h>
6 
7 using namespace std;
8 
9 typedef unsigned long long timestamp_t;
10 
11 timestamp_t get_timestamp () {
12  struct timeval now;
13  gettimeofday (&now, NULL);
14  return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
15 }
16 
17 #define EDGE_COUNT (2 * (P - 1))
18 #define ZH_SENDING 0
19 #define SH_SENDING 1
20 #define MAX_Z 2
21 #define SORTED_S 3
22 #define PART_C 4
23 #define TOTAL_MAX_Z 5
24 #define E_SENDING 6
25 #define MO_SENDING 7
26 #define MTH_SENDING 8
27 #define AH_SENDING 9
28 
29 void mpiRun(MPI_Comm &graph_comm, MPI_Comm &topo_comm, int* index, int* edges);
30 void mergeSend(int rank, MPI_Comm graph_comm, int *Z, int *S, int *maxZ);
31 void maxSort(int &maxZ, int* Z, int* S);
32 int* mergeN(int n, int lengths[], int* arrays[]);
33 
34 int N;
35 int P;
36 int H;
37 int mid;
38 int quarter;
39 
40 int main(int argc, char* argv[]) {
41   MPI_Init(&argc, &argv);
42   MPI_Comm_size(MPI_COMM_WORLD, &P);
43  N = atoi(argv[1]);
44   H = N / P;
45   mid = P / 2;
46   quarter = mid / 2;
47 
48   MPI_Comm graph_comm, topo_comm;
49   int* index = new int[P];
50   int* edges = new int[EDGE_COUNT];
51 
52   mpiRun(graph_comm, topo_comm, index, edges);
53 
54   MPI_Finalize();
55   delete[] index;
56   delete[] edges;
57 }
58 
59 int* initVector(int n) {
60   int *res = new int[n];
61   for (int i = 0; i < n; i++) {
62     res[i] = 1;
63   }
64   return res;
65 }
66 
67 int* initMatrix(int n) {
68   return initVector(n * n);
69 }
70 
71 void mpiRun(MPI_Comm &graph_comm, MPI_Comm &topo_comm, int* index, int* edges) {
72  timestamp_t startTime = get_timestamp();
73   index[0] = 2;
74   for (int i = 1; i < mid - 1; i++) {
75     index[i] = index[i - 1] + 3;
76   }
77   index[mid - 1] = index[mid - 2] + 2;
78 
79   for (int i = mid; i < P; i++) {
80     index[i] = index[i - 1] + 1;
81   }
82 
83   for (int i = 0; i < mid - 1; i++) {
84     edges[i * 2] = i;
85     edges[i * 2 + 1] = i + 1;
86     edges[i * 2 + 2] = i;
87     edges[i * 2 + 3] = mid + i;
88   }
89   edges[EDGE_COUNT - 2] = mid - 1;
90   edges[EDGE_COUNT - 2] = P - 1;
91 
92   MPI_Graph_create(MPI_COMM_WORLD, P, index, edges, 0, &graph_comm);
93 
94   MPI_Comm_dup(graph_comm, &topo_comm);
95 
96   int topo_type;
97   MPI_Topo_test(topo_comm, &topo_type);
98 
99   if (topo_type != MPI_GRAPH) {
100     cout << "Topo type error" << endl;
101     return;
102   }
103  
104   int rank;
105   MPI_Comm_rank(graph_comm, &rank);
106 
107   int H = N / P;
108   MPI_Status status;
109   int maxZ, *Ei, *MOi, *MTh, *Sh, *Ah;
110   Ei = new int[N];
111   MOi = new int[N*N];
112   MTh = new int[N*H];
113   Sh = new int[H];
114   Ah = new int[H];
115   if (rank == 0) {
116     int *Z, *E, *MO, *MT, *S;
117     Z = initVector(N);
118     E = initVector(N);
119     MO = initMatrix(N);
120     MT = initMatrix(N);
121     S = initVector(N);
122 
123     for (int i = 1; i < P; i++) {
124       MPI_Send(Z + i * H, H, MPI_INT, i, ZH_SENDING, graph_comm);
125       MPI_Send(S + i * H, H, MPI_INT, i, SH_SENDING, graph_comm);
126     }
127 
128     maxSort(maxZ, Z, S);   
129     mergeSend(rank, graph_comm, Z, S, &maxZ);
130 
131     for (int i = 1; i < P; i++) {
132       MPI_Send(E, N, MPI_INT, i, E_SENDING, graph_comm);
133       MPI_Send(MO, N * N, MPI_INT, i, MO_SENDING, graph_comm);
134       MPI_Send(MT + i * H * N, H * N, MPI_INT, i, MTH_SENDING, graph_comm);
135     }
136 
137     for (int i = 0; i < H; i++) {
138       Sh[i] = S[i];
139     }
140     for (int i = 0; i < N; i++) {
141       Ei[i] = E[i];
142     }
143     for (int i = 0; i < N * N; i++) {
144       MOi[i] = MO[i];
145     }
146     for (int i = 0; i < N * H; i++) {
147       MTh[i] = MT[i];
148     }
149 
150     delete[] Z;
151     delete[] E;
152     delete[] MO;
153     delete[] MT;
154     delete[] S;
155   } else {
156     int *Z, *S;
157     Z = new int[H];
158     S = new int[H];
159     MPI_Recv(Z, H, MPI_INT, 0, ZH_SENDING, graph_comm, &status);
160     MPI_Recv(S, H, MPI_INT, 0, SH_SENDING, graph_comm, &status);
161 
162     maxSort(maxZ, Z, S);
163 
164     int rank1 = rank < mid ? rank : rank - mid;
165     int sourceRank = (mid & 1) ? quarter : rank1 < quarter ? quarter - 1 : quarter;
166 
167     if (rank < mid) {
168       mergeSend(rank, graph_comm, Z, S, &maxZ);
169       for (int i = 0; i < H; i++) {
170         Sh[i] = S[i];
171       }
172     } else {
173       MPI_Send(&maxZ, 1, MPI_INT, rank - mid, MAX_Z, graph_comm);
174       MPI_Send(S, H, MPI_INT, rank - mid, SORTED_S, graph_comm);
175 
176       MPI_Recv(&maxZ, 1, MPI_INT, sourceRank, TOTAL_MAX_Z, graph_comm, &status);
177       MPI_Recv(Sh, H, MPI_INT, sourceRank, PART_C, graph_comm, &status);
178     }
179     delete[] Z;
180     delete[] S;
181 
182     MPI_Recv(Ei, N, MPI_INT, 0, E_SENDING, graph_comm, &status);
183     MPI_Recv(MOi, N * N, MPI_INT, 0, MO_SENDING, graph_comm, &status);
184     MPI_Recv(MTh, H * N, MPI_INT, 0, MTH_SENDING, graph_comm, &status);
185   }
186 
187   for (int i = 0; i < H; i++) {
188     Ah[i] = Sh[i];
189     for (int j = 0; j < N; j++) {
190       int mot = 0;
191       for (int k = 0; k < N; k++) {
192         mot += MOi[k * N + j] * MTh[i * N + k];
193       }
194       Ah[i] += maxZ * mot * Ei[j];
195     }
196   }
197 
198 
199   if (rank == mid - 1) {
200     int *A = new int[N];
201     for (int i = 0; i < H; i++) {
202       A[(mid - 1) * H + i] = Ah[i];
203     }
204     for (int i = 0; i < P; i++) {
205       if (i != mid - 1) {
206         MPI_Recv(A + i * H, H, MPI_INT, i, AH_SENDING, graph_comm, &status);
207       }
208     }
209     if (N <= 20) {
210       for (int i = 0; i < N; i++) {
211         cout << A[i] << " ";
212       }
213       cout << endl;
214     } else {
215     timestamp_t endTime = get_timestamp();
216     cout << (endTime - startTime) / 1000000.0 << endl;
217 
218     }
219     delete[] A;
220   } else {
221     MPI_Send(Ah, H, MPI_INT, mid - 1, AH_SENDING, graph_comm);
222   }
223   
224   delete[] Ei;
225   delete[] MOi;
226   delete[] MTh;
227   delete[] Sh;
228 }
229 
230 void maxSort(int &maxZ, int* Z, int* S) {
231   maxZ = -2000000000;
232   for (int i = 0; i < H; i++) {
233     if (Z[i] > maxZ) {
234       maxZ = Z[i];
235     }
236   }
237 
238   for (int i = H; i > 1; i--) {
239     for (int j = 1; j < i; j++) {
240       if (S[j - 1] > S[j]) {
241         int c = S[j - 1];
242         S[j - 1] = S[j];
243         S[j] = c;
244       }
245     }
246   }
247 }
248 
249 void mergeSend(int rank, MPI_Comm graph_comm, int *Z, int *S, int *maxZP) {
250   int maxZ = *maxZP;
251   MPI_Status status;
252   int maxAnotherZ;
253   MPI_Recv(&maxAnotherZ, 1, MPI_INT, rank + mid, MAX_Z, graph_comm, &status);
254   if (maxAnotherZ > maxZ) {
255     maxZ = maxAnotherZ;
256   }
257 
258   int* sortedS2 = new int[H];
259   MPI_Recv(sortedS2, H, MPI_INT, rank + mid, SORTED_S, graph_comm, &status);
260 
261   int targetRank = rank < quarter ? rank + 1 : rank - 1;
262   int sourceRank = 2 * rank - targetRank;
263 
264   if (rank < mid - 1 && rank > 0) {
265     MPI_Recv(&maxAnotherZ, 1, MPI_INT, sourceRank, MAX_Z, graph_comm, &status);
266     if (maxAnotherZ > maxZ) {
267       maxZ = maxAnotherZ;
268     }
269     int sortedSSize = (rank < quarter ? 2 * rank : 2 * (mid - 1 - rank)) * H;
270     int* sortedS3 = new int[sortedSSize];
271     MPI_Recv(sortedS3, sortedSSize, MPI_INT, sourceRank,
272         SORTED_S, graph_comm, &status);
273 
274     if ((mid & 1) == 1 && rank == quarter) {
275       int maxAnotherZ;
276       MPI_Recv(&maxAnotherZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm, &status);
277       if (maxAnotherZ > maxZ) {
278         maxZ = maxAnotherZ;
279       }
280       int *sortedS4 = new int[sortedSSize];
281       MPI_Recv(sortedS4, sortedSSize, MPI_INT, targetRank, SORTED_S, graph_comm, &status);
282 
283       int lengths[] = { H, H, sortedSSize, sortedSSize };
284       int* arrays[] = { S, sortedS2, sortedS3, sortedS4 };
285       int *merged = mergeN(4, lengths, arrays);
286 
287       for (int i = 0; i < P; i++) {
288         if (i != rank) {
289           *maxZP = maxZ;
290           MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
291           MPI_Send(merged + i * H, H, MPI_INT, i, PART_C, graph_comm);
292         }
293       }
294 
295       for (int i = 0; i < H; i++) {
296         S[i] = merged[i + rank * H];
297       }
298 
299       delete[] merged;
300       delete[] sortedS4;
301       return;
302     }
303     int mergedSize = sortedSSize + 2 * H;
304     int* merged;
305     int lengths[] = { H, H, sortedSSize };
306     int* arrays[] = { S, sortedS2, sortedS3 };
307     merged = mergeN(3, lengths, arrays);
308     int anotherMaxZ;
309 
310     if ((mid & 1) == 0 && (rank == quarter - 1 || rank == quarter)) {
311       int maxAnotherZ;
312       int *sortedS2 = new int[mid * H];
313 
314       if (rank == quarter) {
315         MPI_Send(&anotherMaxZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
316         MPI_Send(merged, mergedSize, MPI_INT, targetRank, SORTED_S, graph_comm);
317 
318         MPI_Recv(&maxAnotherZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm, &status);
319         if (maxAnotherZ > maxZ) {
320           maxZ = maxAnotherZ;
321         }
322         MPI_Recv(sortedS2, mid * H, MPI_INT, targetRank, SORTED_S, graph_comm, &status);
323 
324       } else {
325         MPI_Recv(&maxAnotherZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm, &status);
326         if (maxAnotherZ > maxZ) {
327           maxZ = maxAnotherZ;
328         }
329         MPI_Recv(sortedS2, mid * H, MPI_INT, targetRank, SORTED_S, graph_comm, &status);
330 
331         MPI_Send(&anotherMaxZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
332         MPI_Send(merged, mergedSize, MPI_INT, targetRank, SORTED_S, graph_comm);
333       }
334 
335       int* totalS;
336       int lengths[] = { mid * H, mid * H };
337       int* arrays[] = { merged, sortedS2 };
338       totalS = mergeN(2, lengths, arrays);
339       *maxZP = maxZ;
340       if (rank == quarter - 1) {
341         for (int i = 0; i < quarter - 1; i++) {
342           MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
343           MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
344         }
345         for (int i = mid; i < mid + quarter; i++) {
346           MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
347           MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
348         }
349       } else {
350         for (int i = quarter + 1; i < mid; i++) {
351           MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
352           MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
353         }
354         for (int i = mid + quarter; i < P; i++) {
355           MPI_Send(maxZP, 1, MPI_INT, i, TOTAL_MAX_Z, graph_comm);
356           MPI_Send(totalS + i * H, H, MPI_INT, i, PART_C, graph_comm);
357         }
358       }
359       for (int i = 0; i < H; i++) {
360         S[i] = totalS[i + rank * H];
361       }
362 
363       delete[] merged;
364       delete[] totalS;
365       delete[] sortedS2;
366       return;
367     }
368     MPI_Send(&anotherMaxZ, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
369     MPI_Send(merged, mergedSize, MPI_INT, targetRank, SORTED_S, graph_comm);
370 
371     delete[] sortedS3;
372   } else {
373     int lengths[] = { H, H };
374     int* arrays[] = { S, sortedS2 };
375     int* merged = mergeN(2, lengths, arrays);
376     *maxZP = maxZ;
377     MPI_Send(maxZP, 1, MPI_INT, targetRank, MAX_Z, graph_comm);
378     MPI_Send(merged, 2 * H, MPI_INT, targetRank, SORTED_S, graph_comm);
379 
380     delete[] merged;
381   }
382   delete[] sortedS2;
383   
384   int sortedSourceRank = (mid & 1) == 0
385     ? rank < quarter ? quarter - 1 : quarter
386     : quarter;
387   MPI_Recv(maxZP, 1, MPI_INT, sortedSourceRank, TOTAL_MAX_Z, graph_comm, &status);
388   MPI_Recv(S, H, MPI_INT, sortedSourceRank, PART_C, graph_comm, &status);
389 }
390 
391 int* mergeN(int n, int lengths[], int* arrays[]) {
392   int* coeffs = new int[n];
393   int totalLength = 0;
394   for (int i = 0; i < n; i++) {
395     coeffs[i] = 0;
396     totalLength += lengths[i];
397   }
398   int* merged = new int[totalLength];
399 
400   for (int i = 0; i < totalLength; i++) {
401     int max_i = -1;
402     int max;
403     for (int j = 0; j < n; j++) {
404       if (coeffs[j] < lengths[j] && (max_i == -1 || arrays[j][coeffs[j]] > max)) {
405         max_i = j;
406         max = arrays[j][coeffs[j]];
407         coeffs[j]++;
408       }
409     }
410     merged[i] = max;
411   }
412 
413   delete[] coeffs;
414   return merged;
415 }
