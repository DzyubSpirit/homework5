1 /*
2 // main
3 // Author:
4 //    Dzyuba Vlad, IP-42
5 */
6 #include <iostream>
7 #include <omp.h>
8 #include <string.h>
9 #include <stdlib.h>
10 #include <sys/time.h>
11 
12 typedef unsigned long long timestamp_t;
13 
14 timestamp_t get_timestamp () {
15  struct timeval now;
16  gettimeofday (&now, NULL);
17  return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
18 }
19 
20 using namespace std;
21 
22 int N, P, H;
23 
24 // functions for generating structures
25 int* readVec();
26 int* readMat();
27 // functions for copying structures
28 int* veccpy(int *src);
29 int* matcpy(int *src);
30 
31 int main(int argc, char* argv[]) {
32  N = atoi(argv[1]);
33  P = atoi(argv[2]);
34  H = N / P;
35  // input data
36  int *Z, *MO, *E, *S, *MT;
37  // output data
38  int *A = new int[N];
39 
40  // intermidiate data
41  int z = -65536, *B = new int[N], *C = new int[N];
42  timestamp_t startTime = get_timestamp();
43  #pragma omp parallel num_threads(P)
44  {
45   int tid = omp_get_thread_num();
46   // generate data if first or last thread
47   if (tid == 0) {
48    Z = readVec();
49    MO = readMat();
50   }
51   if (tid == P - 1) {
52    E = readVec();
53    S = readVec();
54    MT = readMat();
55   }
56   // finding maximum of Z
57   #pragma omp barrier
58   for (int i = 0; i < P; i++) {
59    int mval = Z[i*H];
60    for (int j = i*H+1; j < (i+1)*H; j++) {
61     mval = Z[j] > mval ? Z[j] : mval;
62    }
63    #pragma omp critical(zUpdate)
64    if (mval > z) {
65     z = mval;
66    }
67   }
68   #pragma omp barrier
69   // calculating B
70   int *MO1, *E1, z1;
71   #pragma omp critical(dataCopy)
72   {
73    MO1 = matcpy(MO);
74     E1 = veccpy(E);
75     z1 = z;
76   }
77   #pragma omp for
78   for (int i = 0; i < P; i++) {
79    for (int j = i * H; j < (i + 1) * H; j++) {
80     B[j] = 0;
81     for (int k = 0; k < N; k++) {
82      int x = 0;
83      for (int l = 0; l < N; l++) {
84       x += MO1[k*N+l] * MT[l*N+j];
85      }
86      B[j] += x * E1[k];
87     }
88     B[j] *= z1;
89    }
90   }
91   #pragma omp barrier
92   // sorting parts of S
93   #pragma omp for
94   for (int i = 0; i < P; i++) {
95    for (int j = 1; j < H; j++) {
96     for (int k = i * H; k < (i + 1) * H - j; k++) {
97      if (S[k] > S[k + 1]) {
98       int x = S[k];
99       S[k] = S[k + 1];
100       S[k + 1] = x;
101      }
102     }
103    }
104   }
105   #pragma omp barrier
106   // sorting whole S and stores itto C
107   #pragma omp single
108   {
109    int* ind = new int[N];
110    for (int i = 0; i < N; i++) {
111     ind[i] = 0;
112    }
113    for (int i = 0; i < N; i++) {
114     int mini = -1;
115     for (int j = 0; j < N; j++) {
116      if (ind[j] < N) {
117       if (mini == -1 || S[ind[j]] < S[ind[mini]]) {
118        mini = j;
119       }
120      }
121     }
122     C[i] = S[ind[mini]];
123     ind[mini]++;
124    }
125   }
126   #pragma omp barrier
127   // calculating A
128   #pragma omp for
129   for (int i = 0; i < N; i++) {
130    A[i] = B[i] + C[i];
131   }
132  }
133  // print A
134  if (N <= 20) {
135   for (int i = 0; i < N; i++) {
136    cout << A[i] << " ";
137   }
138   cout << endl;
139  } else {
140   timestamp_t endTime = get_timestamp();
141   cout << (endTime - startTime) / 1000000.0 << endl;
142  }
143  delete[] A;
144 }
145 
146 int* readArr(int n) {
147  int* res = new int[n];
148  for (int i = 0; i < n; i++) {
149   res[i] = 1;
150  }
151  return res;
152 }
153 
154 int* readVec() { return readArr(N); }
155 int* readMat() { return readArr(N*N); }
156 
157 int* arrcpy(int *src, int n) {
158  int *res = new int[n];  
159  for (int i = 0; i < n; i++) {
160   res[i] = src[i];
161  }
162  return res;
163 }
164 
165 int* veccpy(int *src) { return arrcpy(src, N); }
166 int* matcpy(int *src) { return arrcpy(src, N * N); }
