with Ada.Synchronous_Task_Control; use Ada.Synchronous_Task_Control;
with Ada.Text_IO; use Ada.Text_IO;
package body lib is
  function sliceVec(partI: Integer; vec: vector_i.Vector) return vector_o.Vector is
    res: vector_o.Vector;
  begin
    for i in res'Range loop
      res(i) := vec(convert(partI, i));
    end loop;
    return res;
  end;

  function sliceMat(partI: Integer; mat: matrix_i.Matrix) return matrix_o.Matrix is
    res: matrix_o.Matrix;
  begin
    for i in res'Range loop
      for j in res(i)'Range loop
        res(i)(j) := mat(i)(convert(partI, j));
      end loop;
    end loop;
    return res;
  end;

  function mult(mat1: matrix1.Matrix; mat2: matrix2.Matrix) return matrix_o.Matrix is
    res: matrix_o.Matrix;
  begin
    for i in res'Range loop
      for j in res(i)'Range loop
        res(i)(j) := 0;
        for k in mat2'Range loop
          res(i)(j) := res(i)(j) + mat1(i)(k) * mat2(k)(j);
        end loop;
      end loop;
    end loop;
    return res;
  end;

  package body merge_p is
    function mergeVectors(vec: matrix_i.Matrix) return vector_o.Vector is
      vec_i: array (Index) of vector_i.Index;
      vec_b: array (Index) of Boolean;
      res: vector_o.Vector;
      res_i: Integer := 0;
      min_val: Integer;
      min_j: Index := Index'First;
    begin
      for i in vec_i'Range loop
        vec_i(i) := vec(i)'First;
        vec_b(i) := true;
      end loop;
      for i in res'Range loop
        min_j := vec'First;
        min_val := 32000;
        for j in vec'Range loop
          if vec_b(j) and vec(j)(vec_i(j)) < min_val then
            min_j := j;
            min_val := vec(j)(vec_i(j));
          end if;
        end loop;
        res(res_i) := vec(min_j)(vec_i(min_j));
        if vec_i(min_j) = vector_i.Index'Last then
          vec_b(min_j) := false;
        else
          vec_i(min_j) := vec_i(min_j) + 1;
        end if;
        res_i := res_i + 1;
      end loop;
      return res;
    end;
  end merge_p;
end lib;
