with Ada.Text_IO; use Ada.Text_IO;
package body tasks is  
  task body calc_part is
  begin
    accept start(sem: in out Suspension_Object; e: Integer; Z: vectorZ.Vector; T: vectorT.Vector; K: matrixK.Matrix; R: matrixR.Matrix; Ah: out vectorZ.Vector) do
      calc_p_i.calc(e, Z, T, K, R, Ah);
      Set_True(sem);
    end start;
  end calc_part;
end tasks;
