--------------------------------------------------------------------------------
-- tasks
-- Purpose:
--   Using task type for calculating vector part
--------------------------------------------------------------------------------
with Ada.Synchronous_Task_Control; use Ada.Synchronous_Task_Control;
with vector;
with matrix;
with calc_p;
generic
  with package calc_p_i is new calc_p(<>);
  use calc_p_i;
package tasks is
  task type calc_part is
    entry start(sem: in out Suspension_Object; e: Integer; Z: vectorZ.Vector; T: vectorT.Vector; K: matrixK.Matrix; R: matrixR.Matrix; Ah: out vectorZ.Vector);
  end calc_part;
end tasks;
