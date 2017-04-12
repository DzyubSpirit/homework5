/*
// main
// Author:
//    Dzyuba Vlad, IP-42
*/
using System;
using System.Threading;

namespace Main {
	public class MainClass {
		// parameters
		const int N = 8;
		const int AllP = 8;
		const int ReadP = 4;
		const int H = N / AllP;
		
		// calc data
		static int[,] mt, mu, mo, mz;
		static int[] b, a, d;
		static int e;

		// control objects
		static Semaphore[] semaphores1 = new Semaphore[ReadP];
		static EventWaitHandle readingEnd =
			new EventWaitHandle(false, EventResetMode.ManualReset);
		static Mutex resMutex = new Mutex();
		static Semaphore[] semaphores2 = new Semaphore[AllP];
		static EventWaitHandle dCalcEnd =
		 	new EventWaitHandle(false, EventResetMode.ManualReset);
		static Object criticalSection = new Object();
		static Semaphore[] semaphores3 = new Semaphore[AllP];

		public static void Main(string[] args) {
			Thread[] threads = new Thread[AllP];
			ReadFunc[] readFuncs = new ReadFunc[ReadP] {
			 	new T1Read(), new T2Read(), new T3Read(), new T4Read()
			};
  		for (int i = 0; i < ReadP; i++) {
				semaphores1[i] = new Semaphore(0, 1);	
				threads[i] = new Thread(new ThreadCalcWithRead(readFuncs[i], i).Run);
			}
			for (int i = ReadP; i < AllP; i++) {
				threads[i] = new Thread(new ThreadCalc(i).Run);
			}
			for (int i = 0; i < AllP; i++) {
				semaphores2[i] = new Semaphore(0, 1);
				semaphores3[i] = new Semaphore(0, 1);
				threads[i].Start();
			}
			for (int i = 0; i < ReadP; i++) {
				semaphores1[i].WaitOne();
				semaphores1[i] = null;
			}
			d = new int[N];
			readingEnd.Set();
			for (int i = 0; i < AllP; i++) {
				semaphores2[i].WaitOne();
				semaphores2[i] = null;
			}
			a = new int[N];
			dCalcEnd.Set();
			for (int i = 0; i < AllP; i++) {
				semaphores3[i].WaitOne();
				semaphores3[i] = null;
			}
			if (N < 20) {
				for (int i = 0; i < N; i++) {
					Console.Write(a[i]);
					Console.Write(" ");
				}
				Console.WriteLine();
			}
		}

		class ThreadCalc {
			protected int partIndex;

			public ThreadCalc(int partIndex) {
				this.partIndex = partIndex;
			}

			public virtual void Run() {
				readingEnd.WaitOne();

				int[] bi;
				resMutex.WaitOne();
				bi = b;
				resMutex.ReleaseMutex();

				for (int i = H * partIndex; i < H * (partIndex + 1); i++) {
					d[i] = 0;
					for (int j = 0; j < N; j++) {
						d[i] += bi[j] * mo[j, i];
					}
				}

				semaphores2[partIndex].Release();
				dCalcEnd.WaitOne();

				int[,] mzi;
				int[] di;
				int ei;
				lock (criticalSection) {
					ei = e;
					di = d;
					mzi = mz;
				}

				for (int i = H * partIndex; i < H * (partIndex + 1); i++) {
					a[i] = 0;
					for (int j = 0; j < N; j++) {
						int zt = ei * mu[j, i];
						for (int k = 0; k < N; k++) {
							zt += mzi[j, k] * mt[k, i];
						}
						a[i] += di[i] * zt;		
					}
				}

				semaphores3[partIndex].Release();
			}
		}

		class ThreadCalcWithRead : ThreadCalc {
			ReadFunc readFunc;
			public ThreadCalcWithRead( ReadFunc readFunc
															 , int partIndex) : base(partIndex) {
				this.readFunc = readFunc;
			}

			public override void Run() {
				readFunc.Read();
				semaphores1[partIndex].Release();
				base.Run();
			}
		}

		interface ReadFunc {
			void Read();
		}

		class T1Read : ReadFunc {
			public void Read() {
				mt = new int[N, N];
				for (int i = 0; i < N; i++) {
					for (int j = 0; j < N; j++) {
						mt[i, j] = 1;
					}
				}
				e = 1;
			}
		}

		class T2Read : ReadFunc {
			public void Read() {
				mu = new int[N, N];
				for (int i = 0; i < N; i++) {
					for (int j = 0; j < N; j++) {
						mu[i, j] = 1;
					}
				}
			}
		}

		class T3Read : ReadFunc {
			public void Read() {
				mo = new int[N, N];
				mz = new int[N, N];
				for (int i = 0; i < N; i++) {
					for (int j = 0; j < N; j++) {
						mo[i, j] = 1;
						mz[i, j] = 1;
					}
				}
			}
		}

		class T4Read : ReadFunc {
			public void Read() {
				b = new int[N];
				for (int i = 0; i < N; i++) {
					b[i] = 1;
				}
			}
		}
	}

}
