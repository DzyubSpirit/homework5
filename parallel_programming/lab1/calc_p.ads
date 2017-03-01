--------------------------------------------------------------------------------
-- calc_p
-- Purpose:
--   Calculate formula of lab task.
--------------------------------------------------------------------------------
with vector;
with matrix;
generic
  type IndexZ is range <>;
  with package vectorZ is new vector(IndexZ);
  type IndexT is range <>;
  with package vectorT is new vector(IndexT);
  type IndexKR is range <>;
  with package vectorK is new vector(IndexKR);
  with package matrixK is new matrix(IndexT, vectorK);
  with package matrixR is new matrix(IndexKR, vectorZ);
  package calc_p is
  procedure calc(e: Integer; Z: vectorZ.Vector; T: vectorT.Vector; K: matrixK.Matrix; R: matrixR.Matrix; Ah: out vectorZ.Vector);
end calc_p;
