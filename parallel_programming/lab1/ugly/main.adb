--------------------------------------------------------------------------------
-- main
-- Author:
--   Dzyuba Vlad, IP-42
-- Purpose:
--   Parallel calculating of operations with numbers, vectors and matricies.
--------------------------------------------------------------------------------
with Ada.Text_IO; use Ada.Text_IO;
with Ada.Synchronous_Task_Control; use Ada.Synchronous_Task_Control;
procedure main is
  N: constant Integer := 10;
  subtype Index is Integer range 0..N-1;
  type Vector is array (Index) of Integer;
  type Matrix is array (Index) of Vector;

  sem1, sem2: Suspension_Object;
  e: Integer;
  Z, T: Vector;
  MK, MR: Matrix;
  A: Vector;
  
  task T1;
  task T2;

  task body T1 is
    e1: Integer;
    T1: Vector;
    MK1: Matrix;
    subtype PartedIndex is Index range 0..N/2-1;
    type VectorH is array (PartedIndex) of Integer;
    type MatrixH is array (Index) of VectorH;
    -- Result of multiplication of MK and MR
    MKR: MatrixH;
    -- Result of multiplication of T and MKR
    TKR: VectorH;
    i, j: Integer;
  begin
    -- Reading
    e := 1;
    for i in Index loop
      Z(i) := 1;
      T(1) := 1;
      for j in Index loop
        MK(i)(j) := 1;
        MR(i)(j) := 1;
      end loop;
    end loop;
    -- Copying
    e1 := e;
    T1 := T;
    MK1 := MK;
    -- Signal about copying end to T2
    Set_True(sem1);
    -- Calculate MKR
    for i in Index loop
      for j in PartedIndex loop
        MKR(i)(j) := 0;
        for k in Index loop
          MKR(i)(j) := MKR(i)(j) + MK1(i)(k) * MR(k)(j);
        end loop;
      end loop;
    end loop;
    -- Calculate TKR
    for i in PartedIndex loop
      TKR(i) := 0;
      for j in Index loop
        TKR(i) := TKR(i) + T1(j) * MKR(j)(i);
      end loop;
    end loop;
    -- Calculate Ah
    for i in PartedIndex loop
      A(i) := e * Z(i) + TKR(i);
    end loop;
    -- Sort Ah
    for i in 0..PartedIndex'Last-PartedIndex'First loop
      for j in PartedIndex'First..PartedIndex'Last-i loop
        if A(j) > A(j + 1) then
          A(j) := A(j) + A(j + 1);
          A(j + 1) := A(j) - A(j + 1);
          A(j) := A(j) - A(j + 1);
        end if;
      end loop;
    end loop;
    -- Wait signal about calculation finishing from T2
    Suspend_Until_True(sem2);
    -- sortM A
    i := 0;
    j := N / 2;
    while (i /= j) loop
      if A(j) < A(i) then
        A(i) := A(i) + A(j);
        A(j) := A(i) - A(j);
        A(i) := A(i) - A(j);
        if j < N - 1 then
          j := j + 1;
        else
          j := i + 1;
        end if;
      end if;
      i := i + 1;
    end loop;
    -- Print A
    for i in Index loop
      Put(Integer'Image(A(i)));
      Put(" ");
    end loop;
    New_Line;
  end;

  task body T2 is
    e2: Integer;
    T2: Vector;
    MK2: Matrix;
    subtype PartedIndex is Index range N/2..N-1;
    type VectorH is array (PartedIndex) of Integer;
    type MatrixH is array (Index) of VectorH;
    -- Result of multiplication of MK and MR
    MKR: MatrixH;
    -- Result of multiplication of T and MKR
    TKR: VectorH;
  begin
    -- Wait signal about copying finish from T1
    Suspend_Until_True(sem1);
    -- Copying
    e2 := e;
    T2 := T;
    MK2 := MK;
    -- Calculate MKR
    for i in Index loop
      for j in PartedIndex loop
        MKR(i)(j) := 0;
        for k in Index loop
          MKR(i)(j) := MKR(i)(j) + MK2(i)(k) * MR(k)(j);
        end loop;
      end loop;
    end loop;
    -- Calculate TKR
    for i in PartedIndex loop
      TKR(i) := 0;
      for j in Index loop
        TKR(i) := TKR(i) + T2(j) * MKR(j)(i);
      end loop;
    end loop;
    -- Calculate Ah
    for i in PartedIndex loop
      A(i) := e * Z(i) + TKR(i);
    end loop;
    -- Sort Ah
    for i in 1..PartedIndex'Last-PartedIndex'First loop
      for j in PartedIndex'First..PartedIndex'Last-i loop
        if A(j) > A(j + 1) then
          A(j) := A(j) + A(j + 1);
          A(j + 1) := A(j) - A(j + 1);
          A(j) := A(j) - A(j + 1);
        end if;
      end loop;
    end loop;
    -- Signal about calculating finish to T1
    Set_True(sem2);
  end;
begin
  null;
end;
