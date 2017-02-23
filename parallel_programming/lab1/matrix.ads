with vector;
generic
  type Index is range <>;
  with package vector_i is new vector(<>);
package matrix is
  use vector_i;
  type Matrix is array (Index) of vector_i.Vector;

  function sum(mat1: Matrix; mat2: Matrix) return Matrix;
  function oneMatrix return Matrix;
  function fromVector(vec: vector_i.Vector) return Matrix;
  function toVector(mat: Matrix) return vector_i.Vector;
  procedure print(mat: Matrix);
end matrix;
