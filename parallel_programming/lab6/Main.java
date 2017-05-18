/*
 * Dziuba Vlad, IP-42
 * */
import java.util.concurrent.Semaphore;
import java.util.concurrent.RecursiveTask;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.ForkJoinTask;
import java.util.function.IntBinaryOperator;

public class Main {
  private static final int N = 800;
  private static final int P = 8;
  private static final int H = N / P;


  private static int[] Z;
  private static Semaphore ZIsRead = new Semaphore(0);
  private static Integer z;
  private static int[][] MO;
  private static Semaphore MOIsRead = new Semaphore(0);
  private static int[] B;
  private static Semaphore BIsRead = new Semaphore(0);
  private static int[] C;
  private static Semaphore CIsRead = new Semaphore(0);
  private static Integer d;
  private static int[][] MX;
  private static Semaphore MXIsRead = new Semaphore(0);
  private static int[][] ME;
  private static Semaphore MEIsRead = new Semaphore(0);

  private static int[][] MA;

  private static Semaphore zdIsCalculated = new Semaphore(0);
  private static Semaphore MAIsCalculated = new Semaphore(0);

  private static Runnable[] calculations = new Runnable[P];

  private static final IntBinaryOperator minZ = (begin, end) -> {
    int min = Integer.MAX_VALUE;
    for (int i = begin; i < end; i++) {
      min = Math.min(min, Z[i]);
    }
    return min;
  };

  private static final IntBinaryOperator sumBC = (begin, end) -> {
    int sum = 0;
    for (int i = begin; i < end; i++) {
      sum += B[i] * C[i];
    }
    return sum;
  };


  private static final ForkJoinTask<Integer> minimizer =
    new ArraySplitTask(Math::min, minZ, 0, N).task;
  private static final ForkJoinTask<Integer> summator =
    new ArraySplitTask((x, y) -> (x + y), sumBC, 0, N).task;

  static {
    MA = new int[N][];
    for (int i = 0 ; i < N; i++) {
      MA[i] = new int[N];
    }
  }

  public static void main(String... args) {
    for (int i = 0; i < P; i++) {
      calculations[i] = new Calculation(i);
    }
    calculations[0] = new ReadCalculation(calculations[0], new Read1());
    calculations[1] = new ReadCalculation(calculations[1], new Read2());
    calculations[2] = new ReadCalculation(calculations[2], new Read3());
    calculations[5] = new ReadWriteCalculation(calculations[5], new Read6(), new Write6());

    for (int i = 0; i < P-1; i++) {
      new Thread(calculations[i]).start();
    }
    Thread lastThread = new Thread(calculations[P-1]);
    lastThread.start();

    ForkJoinPool.commonPool().execute(ForkJoinTask.adapt(() -> {
      ForkJoinTask<?> t1 = ForkJoinTask.adapt(() -> {
        try {
          ZIsRead.acquire();
        } catch (Exception e) {
          e.printStackTrace();
        }
        z = minimizer.invoke();
      });
      ForkJoinTask<?> t2 = ForkJoinTask.adapt(() -> {
        try {
          BIsRead.acquire();
          CIsRead.acquire();
        } catch (Exception e) {
          e.printStackTrace();
        }
        d = summator.invoke();
      });
      t1.fork();
      t2.fork();
      t1.join();
      t2.join();
      zdIsCalculated.release(P);
    }));

    try {
      lastThread.join();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  static class Calculation implements Runnable {
    private int index;
    
    public Calculation(int index) {
      this.index = index;
    }

    @Override
    public void run() {
      try {
        zdIsCalculated.acquire();
      } catch (Exception e) {
        e.printStackTrace();
      }
      int zi, di;
      int[][] MXi;
      synchronized (z) {
        zi = z;
      }
      synchronized (d) {
        di = d;
      }
      synchronized (MX) {
        MXi = MX;
      }
      for (int i = index * H; i < (index + 1) * H; i++) {
        for (int j = 0; j < N; j++) {
          MA[j][i] = zi * MO[j][i];
          for (int k = 0; k < N; k++) {
            MA[j][i] += di * MX[j][k] * ME[k][i];
          }
        }
      }
      MAIsCalculated.release();
    }
  }

  static int[] initVector() {
    int[] res = new int[N];
    for (int i = 0; i < N; i++) {
      res[i] = 1;
    }
    return res;
  }

  static int[][] initMatrix() {
    int[][] res = new int[N][];
    for (int i = 0; i < N; i++) {
      res[i] = initVector();
    }
    return res;
  }

  static class ArraySplitTask {
    private IntBinaryOperator op;
    private IntBinaryOperator opChunk;
    public Recurser task;

    public ArraySplitTask(IntBinaryOperator op,
        IntBinaryOperator opChunk, int begin, int end) {
      this.op = op;
      this.opChunk = opChunk;
      this.task = new Recurser(begin, end);
    }

    @SuppressWarnings("serial")
    public class Recurser extends RecursiveTask<Integer> {
      private int begin;
      private int end;

      public Recurser(int begin, int end) {
        this.begin = begin;
        this.end = end;
      }

      protected Integer compute() {
        if (end - begin <= H) {
          return opChunk.applyAsInt(begin, end);
        }
        int mid = (begin + end) / 2;
        Recurser t1 = new Recurser(begin, mid);
        t1.fork();
        Recurser t2 = new Recurser(mid, end);
        return op.applyAsInt(t2.compute(), t1.join());
      }
    }
  }

  static class Read1 implements Runnable {
    @Override
    public void run() {
      B = initVector();
      BIsRead.release();
      ME = initMatrix();
      MEIsRead.release();
    }
  }

  static class Read2 implements Runnable {
    @Override
    public void run() {
      MO = initMatrix();
      MOIsRead.release();
    }
  }

  static class Read3 implements Runnable {
    @Override
    public void run() {
      Z = initVector();
      ZIsRead.release();
      MX = initMatrix();
      MXIsRead.release();
    }
  }

  static class Read6 implements Runnable {
    @Override
    public void run() {
      C = initVector();
      CIsRead.release();
    }
  }

  static class Write6 implements Runnable {
    @Override
    public void run() {
      try {
        MAIsCalculated.acquire(P);
      } catch (Exception e) {
        e.printStackTrace();
      }
      if (N < 20) {
        for (int i = 0; i < N; i++) {
          for (int j = 0; j < N; j++) {
            System.out.print(MA[i][j]);
            System.out.print(" ");
          }
          System.out.println();
        }
      }
    } 
  }

  static class ReadCalculation implements Runnable {
    private Runnable reader;
    private Runnable calculation;

    public ReadCalculation(Runnable calculation, Runnable reader) {
      this.reader = reader;
      this.calculation = calculation;
    }

    @Override
    public void run() {
      reader.run();
      calculation.run();
    }
  }

  static class ReadWriteCalculation implements Runnable {
    private Runnable writer;
    private ReadCalculation readCalculation;

    public ReadWriteCalculation(Runnable calculation, Runnable reader, Runnable writer) {
      this.readCalculation = new ReadCalculation(calculation, reader);
      this.writer = writer;
    }

    @Override
    public void run() {
      readCalculation.run();
      writer.run();
    }
  }
}


