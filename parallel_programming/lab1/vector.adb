with Ada.Text_IO; use Ada.Text_IO;
package body vector is
  function oneVector return Vector is
    res: Vector;
  begin
    for i in Index loop
      res(i) := 1;
    end loop;
    return res;
  end;

  function mult(k: Integer; vec: Vector) return Vector is
    res: Vector;
  begin
    for i in res'Range loop
      res(i) := k * vec(i);
    end loop;
    return res;
  end;

  procedure sort(vec: in out Vector) is
    c: Integer;
  begin
    for i in vec'First+1..vec'Last loop
      for j in vec'First..vec'Last - (i - vec'First) loop
        if vec(j) > vec(j + 1) then
          c := vec(j);
          vec(j) := vec(j + 1);
          vec(j + 1) := c;
        end if;
      end loop;
    end loop;
  end;

  procedure print(vec: Vector) is
  begin
    for i in vec'Range loop
      Put(Integer'Image(vec(i)));
      Put(" ");
    end loop;
    New_Line;
  end;
end vector;
