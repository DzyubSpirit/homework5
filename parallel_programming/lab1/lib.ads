with Ada.Synchronous_Task_Control; use Ada.Synchronous_Task_Control;
with vector;
with matrix;
package lib is
  generic
    type IndexI is range <>;
    type IndexO is range <>;
    with function convert(partI: Integer; ind: IndexO) return IndexI;
    with package vector_i is new vector(IndexI);
    with package vector_o is new vector(IndexO);
  function sliceVec(partI: Integer; vec: vector_i.Vector) return vector_o.Vector;

  generic
    type Index is range <>;
    type IndexI is range <>;
    type IndexO is range <>;
    with function convert(partI: Integer; ind: IndexO) return IndexI;
    with package vector_i_i is new vector(IndexI);
    with package vector_i_o is new vector(IndexO);
    with package matrix_i is new matrix(Index, vector_i_i);
    with package matrix_o is new matrix(Index, vector_i_o);
  function sliceMat(partI: Integer; mat: matrix_i.Matrix) return matrix_o.Matrix;

   generic
    type RowIndex1 is range <>;
    type GenIndex is range <>;
    type ColIndex2 is range <>;
    with package vector_i1 is new vector(GenIndex);
    with package vector_i2 is new vector(ColIndex2);
    with package matrix1 is new matrix(RowIndex1, vector_i1);
    with package matrix2 is new matrix(GenIndex, vector_i2);
    with package matrix_o is new matrix(RowIndex1, vector_i2);
   function mult(mat1: matrix1.Matrix; mat2: matrix2.Matrix) return matrix_o.Matrix;

  generic
    with package matrix_i is new matrix(<>);
    with function newSize(s1: matrix_i.Index; s2: matrix_i.vector_i.Index) return Integer;
  package merge_p is
    use matrix_i;
    use type Index;
    use type vector_i.Index;
    subtype IndexO is Integer range 0..newSize(Index'Last-Index'First, vector_i.Index'Last - vector_i.Index'First)-1;
    package vector_o is new vector(IndexO);
    function mergeVectors(vec: matrix_i.Matrix) return vector_o.Vector;
  end merge_p;
end lib;

