--
-- Dzyuba Vlad, IP-42
--
with Ada.Text_IO; use Ada.Text_IO;
procedure main is
  N: constant Integer := 12;
  P: constant Integer := 6;
  H: constant Integer := N / P;
  subtype Index is Integer range 1..N;
  type Vector is array (Index) of Integer;
  type Matrix is array (Index) of Vector;

  generic
    type ItemType is private;
  package PItemPkg is
    protected type PItem is
      entry Read(outItem: in out ItemType);
      procedure Write(inItem: ItemType);
    private
      initialized: Boolean := False;
    end;
  end;

  package body PItemPkg is
    item: ItemType;

    protected body PItem is
      entry Read(outItem: in out ItemType) when initialized is
      begin
        outItem := item;
      end;

      procedure Write(inItem: ItemType) is
      begin
        item := inItem;
        initialized := True;
      end;
    end;
  end;

  protected type PCounter is
    procedure Add(x: Integer);
    function Read return Integer;
    private
      i: Integer := 0;
  end;

  protected body PCounter is
    procedure Add(x: Integer) is
    begin
      i := i + x;
    end;

    function Read return Integer is
    begin
      return i;
    end;
  end;

  protected type PMaximizer is
    procedure Max(x: Integer);
    function Read return Integer;
    private
      i: Integer := 0;
  end;

  protected body PMaximizer is
    procedure Max(x: Integer) is
    begin
      if x > i then
        i := x;
      end if;
    end;

    function Read return Integer is
    begin
      return i;
    end;
  end;

  package PVectorPkg is new PItemPkg(Vector);
  subtype PVector is PVectorPkg.PItem;
  package PMatrixPkg is new PItemPkg(Matrix);
  subtype PMatrix is PMatrixPkg.PItem;

  zInt: PMaximizer;
  d: PCounter;
  B, C, Z: Vector;
  MX: PMatrix;
  ME, MA, MO: Matrix;

 generic
    n: Integer;
  package AsyncSemaphorePkg is
    protected type AsyncSemaphore is
      procedure Add;
      entry Wait;
      private
        i: Integer := 0;
    end;
  end;

  package body AsyncSemaphorePkg is
    protected body AsyncSemaphore is
      procedure Add is
      begin
        i := i + 1;
      end;

      entry Wait when i >= n is
      begin
        null;
      end;
    end;
  end;

  package EventPkg is new AsyncSemaphorePkg(1);
  subtype Event is EventPkg.AsyncSemaphore;

  package WaitPPkg is new AsyncSemaphorePkg(P);
  subtype WaitP is WaitPPkg.AsyncSemaphore;

  generic
    index: Integer;
  package CalculationPkg is
    task type Calculation;
  end;

  ZIsRead, BIsRead, CIsRead, MEIsRead, MXIsRead, MOIsRead: Event;
  zdCalculationEnd, MAIsCalculated: WaitP;

  package body CalculationPkg is
    task body Calculation is
      xe, zIntI, dI: Integer;
      MXi: Matrix;
    begin
      ZIsRead.Wait;
      zIntI := Integer'First;
      for i in index*H+1..(index + 1)*H loop
        if Z(i) > zIntI then
          zIntI := Z(i);
        end if;
      end loop;
      zInt.Max(zIntI);
      BIsRead.Wait;
      CIsRead.Wait;
      dI := 0;
      for i in index*H+1..(index + 1)*H loop
        dI := dI + B(i) * C(i);
      end loop;
      d.Add(dI);
      zdCalculationEnd.Add;
      zdCalculationEnd.Wait;
      zIntI := zInt.Read;
      dI := d.Read;
      MX.Read(MXi);
      MEIsRead.Wait;
      MOIsRead.Wait;
      for i in index*H+1..(index + 1)*H loop
        for j in 1..N loop
          xe := 0;
          for k in 1..N loop
            xe := xe + MXi(j)(k) * ME(k)(i);
          end loop;
          MA(i)(j) := zIntI * MO(j)(i) + dI * xe;
        end loop;
      end loop;
      MAIsCalculated.Add;
    end Calculation;
  end;

  function initVector return Vector is
    res: Vector;
  begin
    for i in Index loop
      res(i) := 1;
    end loop;
    return res;
  end;

  function initMatrix return Matrix is
    mat: Matrix;
  begin
    for i in Index loop
      mat(i) := initVector;
    end loop;
    return mat;
  end;

  task Read1;
  task body Read1 is
    package CalculationIPkg is new CalculationPkg(0);
    calc: CalculationIPkg.Calculation;
  begin
    Put_Line("Thread 1 has started");
    B := initVector;
    BIsRead.Add;
    ME := initMatrix;
    MEIsRead.Add;
  end;

  task Read2;
  task body Read2 is
    package CalculationIPkg is new CalculationPkg(1);
    calc: CalculationIPkg.Calculation;
  begin
    Put_Line("Thread 2 has started");
    MO := initMatrix;
    MOIsRead.Add;
  end;

  task Read3;
  task body Read3 is
    package CalculationIPkg is new CalculationPkg(2);
    calc: CalculationIPkg.Calculation;
  begin
    Put_Line("Thread 3 has started");
    Z := initVector;
    ZIsRead.Add;
    MX.Write(initMatrix);
    MXIsRead.Add;
  end;

  task Read4;
  task body Read4 is
    package CalculationIPkg is new CalculationPkg(3);
    calc: CalculationIPkg.Calculation;
  begin
    Put_Line("Thread 4 has started");
  end;

  task Read5;
  task body Read5 is
    package CalculationIPkg is new CalculationPkg(4);
    calc: CalculationIPkg.Calculation;
  begin
    Put_Line("Thread 5 has started");
  end;

  task Read6;
  task body Read6 is
    package CalculationIPkg is new CalculationPkg(5);
    calc: CalculationIPkg.Calculation;
  begin
    Put_Line("Thread 6 has started");
    C := initVector;
    CIsRead.Add;
    MAisCalculated.Wait;
    if N < 20 then
      for i in 1..N loop
        for j in 1..N loop
          Put(Integer'Image(MA(i)(j)));
          Put(" ");
        end loop;
        Put_Line("");
      end loop;
    end if;
  end;
begin
  null;
end;
