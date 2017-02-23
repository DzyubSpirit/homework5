with Ada.Text_IO; use Ada.Text_IO;
with Ada.Integer_Text_IO; use Ada.Integer_Text_IO;
package body matrix is
  function oneMatrix return Matrix is
    res: Matrix;
  begin
    for i in Index loop
      res(i) := oneVector;
    end loop;
    return res;
  end;

  function fromVector(vec: vector_i.Vector) return Matrix is
    res: Matrix;
  begin
    for i in res'Range loop
      for j in res(i)'Range loop
        res(i)(j) := vec(j);
      end loop;
    end loop;
    return res;
  end;

  function sum(mat1: Matrix; mat2: Matrix) return Matrix is
    res: Matrix;
  begin
    for i in res'Range loop
      for j in res(i)'Range loop
        res(i)(j) := mat1(i)(j) + mat2(i)(j);
      end loop;
    end loop;
    return res;
  end;

  function toVector(mat: Matrix) return vector_i.Vector is
    res: vector_i.Vector;
  begin
    for i in res'Range loop
      res(i) := mat(mat'First)(i);
    end loop;
    return res;
  end;

  procedure print(mat: Matrix) is
  begin
    for i in mat'Range loop
      vector_i.print(mat(i));
    end loop;
  end;
end matrix;
