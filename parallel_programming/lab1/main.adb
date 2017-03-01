--------------------------------------------------------------------------------
-- main
-- Author:
--   Dzyuba Vlad, IP-42
-- Purpose:
--   Parallel calculating of operations with numbers, vectors and matricies.
--------------------------------------------------------------------------------
with Ada.Text_IO; use Ada.Text_IO;
with Ada.Synchronous_Task_Control; use Ada.Synchronous_Task_Control;
with lib; use lib;
with tasks;
with calc_p;
with matrix;
with vector;
procedure main is
  -- Output vector size
  N: constant Integer := 6;
  -- Number of processors
  P: constant Integer := 3;
  -- Size of one vector part
  pSize: constant Integer := N / P;
  -- MK row width and MR column height
  N2: Integer := 4;
  -- Vector T width
  N3: Integer := 5;
  subtype IndexZ is Integer range 0..N-1;
  subtype IndexT is Integer range 0..N3-1;
  subtype IndexKR is Integer range 0..N2-1;
  package vectorZ is new vector(IndexZ);
  package vectorT is new vector(IndexT);
  package vectorK is new vector(IndexKR);
  package matrixK is new matrix(IndexT, VectorK);
  package matrixR is new matrix(IndexKR, vectorZ);
  -- Vector and matrix types for part formula
  subtype IndexPart is IndexZ range 0..pSize-1;
  package vectorZh is new vector(IndexPart);
  subtype VecIndex is Integer range 0..0;
  package matrixZh is new matrix(VecIndex, VectorZh);
  package matrixRh is new matrix(IndexKR, VectorZh);
  -- Input number, vectors and matricies
  e: Integer := 1;
  Z: vectorZ.Vector := vectorZ.oneVector;
  T: vectorT.Vector := vectorT.oneVector;
  MK: matrixK.Matrix := matrixK.oneMatrix;
  MR: matrixR.Matrix := matrixR.oneMatrix;
  -- Parted vectors and matricies
  subtype PIndex is Integer range 0..P-1;
  Zhs: array (PIndex) of vectorZh.Vector;
  MRhs: array (PIndex) of matrixRh.Matrix;
  package matrixAhs is new matrix(PIndex, VectorZh);
  Ahs: matrixAhs.Matrix;
  function convertFromPart(partI: Integer; ind: IndexPart) return IndexZ is
  begin
    return partI * (IndexPart'Last - IndexPart'First) + ind;
  end;
  function sliceVecN is new sliceVec(IndexZ, IndexPart, convertFromPart, vectorZ, vectorZh);
  function sliceMatN is new sliceMat(IndexKR, IndexZ, IndexPart, convertFromPart, VectorZ, VectorZh, matrixR, matrixRh);
  function newSize(s1: PIndex; s2: IndexPart) return Integer is
  begin
    return s1 * s2;
  end;
  package merge_p_i is new merge_p(matrixAhs, newSize);
  A: merge_p_i.vector_o.Vector;
  package calc_p_i is new calc_p(IndexPart, vectorZh, IndexT, VectorT, IndexKR, VectorK, matrixK, MatrixRh);
  package tasks_i is new tasks(calc_p_i);
  use tasks_i;
  tasks: array (0..P-1) of calc_part;
  sems: array (0..P-1) of Suspension_Object;
begin
  for i in PIndex loop
    Zhs(i) := sliceVecN(i, Z);
    MRhs(i) := sliceMatN(i, MR);
    tasks(i).start(sems(i), e, Zhs(i), T, MK, MRhs(i), Ahs(i));
  end loop;
  for i in PIndex loop
    Suspend_Until_True(sems(i));
  end loop;
  A := merge_p_i.mergeVectors(Ahs);
  merge_p_i.vector_o.print(A);
end main;
