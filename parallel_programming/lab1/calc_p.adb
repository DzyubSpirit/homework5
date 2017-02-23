with Ada.Text_IO; use Ada.Text_IO;
with Ada.Integer_Text_IO; use Ada.Integer_Text_IO;
with lib; use lib;
with matrix;
package body calc_p is
  procedure calc(e: Integer; Z: vectorZ.Vector; T: vectorT.Vector; K: matrixK.Matrix; R: matrixR.Matrix; Ah: out vectorZ.Vector) is
    subtype VecIndex is Integer range 0..0;
    package matrixT is new matrix(VecIndex, vectorT);
    package matrixZ is new matrix(VecIndex, vectorZ);
    package matrixKR is new matrix(indexT, vectorZ);
    function mult_KR is new mult(IndexT, IndexKR, IndexZ, vectorK, vectorZ, matrixK, matrixR, matrixKR); 
    function mult_TKR is new mult(VecIndex, IndexT, IndexZ, vectorT, vectorZ, matrixT, matrixKR, matrixZ);
    tkr: matrixZ.Matrix;
  begin
    tkr := mult_TKR(matrixT.fromVector(T), mult_KR(K, R));
    Ah := matrixZ.toVector(matrixZ.sum(matrixZ.fromVector(vectorZ.mult(e, Z)), tkr));
    vectorZ.sort(Ah);
  end;
end calc_p;
