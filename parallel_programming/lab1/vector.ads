generic
  type Index is range <>;
package vector is
  type Vector is array (Index) of Integer;

  function mult(k: Integer; vec: Vector) return Vector;
  function oneVector return Vector;
  procedure sort(vec: in out Vector);
  procedure print(vec: Vector);
end vector;
