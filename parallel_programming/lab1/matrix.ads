--------------------------------------------------------------------------------
-- matrix
-- Purpose:
--   Using matrixes as array with dynamic index of vectors.
--------------------------------------------------------------------------------
with vector;
generic
  type Index is range <>;
  with package vector_i is new vector(<>);
package matrix is
  use vector_i;
  type Matrix is array (Index) of vector_i.Vector;

  function sum(mat1: Matrix; mat2: Matrix) return Matrix;
  ------------------------------------------------------------------------------ 
  -- oneMatrix
  -- Purpose:
  --   Creating matrix with each element equals one. 
  ------------------------------------------------------------------------------ 
  function oneMatrix return Matrix;
  ------------------------------------------------------------------------------ 
  -- fromVector
  -- Purpose:
  --   Creating one-height matrix from Vector
  ------------------------------------------------------------------------------ 
  function fromVector(vec: vector_i.Vector) return Matrix;
  ------------------------------------------------------------------------------ 
  -- toVector
  -- Purpose:
  --   Creating vector from matrix. Returns first row of matrix.
  ------------------------------------------------------------------------------ 
  function toVector(mat: Matrix) return vector_i.Vector;
  procedure print(mat: Matrix);
end matrix;
