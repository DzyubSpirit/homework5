--------------------------------------------------------------------------------
-- main
-- Author:
--   Dzyuba Vlad, IP-42
-- Purpose:
--   Parallel calculating of operations with numbers, vectors and matricies.
--------------------------------------------------------------------------------
with Ada.Text_IO; use Ada.Text_IO;

procedure main is
  N: constant Integer := 128;
  P: constant Integer := 8;
  H: constant Integer := N / P;
  subtype Index is Integer range 0..N-1;
  subtype PartIndex is Integer range 0..H-1;
  type Vector is array (Index) of Integer;
  type VectorPart is array (PartIndex) of Integer;
  type Matrix is array (Index) of Vector;
  type MatrixPart is array (PartIndex) of Vector;

  function initVector return Vector is
    res: Vector;
  begin
    for i in Vector'Range loop
      res(i) := 1;
    end loop;
    return res;
  end;

  function initMatrix return Matrix is
    res: Matrix;
  begin
    for i in Matrix'Range loop
      res(i) := initVector;
    end loop;
    return res;
  end;

  type IntDataNames is (ZMaxData, FinalZMaxData, DData);
  type IntPacket is record
    data: Integer;
    dest: Integer;
    dataName: IntDataNames;
  end record;

  type VecDataNames is (TData);
  type VecPacket is record
    data: Vector;
    dest: Integer;
    dataName: VecDataNames;
  end record;

  type VecPartDataNames is (ZhData, EhData, AhData);
  type VecPartPacket is record
    data: VectorPart;
    src: Integer;
    dest: Integer;
    dataName: VecPartDataNames;
  end record;

  type MatDataNames is (MOData);
  type MatPacket is record
    data: Matrix;
    dest: Integer;
    dataName: MatDataNames;
  end record;
  
  type MatPartDataNames is (MKhData);
  type MatPartPacket is record
    data: MatrixPart;
    dest: Integer;
    dataName: MatPartDataNames;
  end record;

  type DynArray is array (Integer range <>) of Integer;

  subtype ServerIds is Integer range 0..P-1;
  type NextsArray is Array (ServerIds) of ServerIds;

  task type Calculation is
    entry Init(i, parent, neighboursCount: Integer; ns: NextsArray);
    entry PIntPacket(x: IntPacket);
    entry PVecPacket(vec: VecPacket);
    entry PVecPartPacket(vech: VecPartPacket);
    entry PMatPacket(mat: MatPacket);
    entry PMatPartPacket(math: MatPartPacket);
    entry Finish;
  end;

  calc1, calc2, calc3, calc4, calc5, calc6, calc7, calc8: Calculation;

  procedure callPMatPartPacket(i: ServerIds; packet: MatPartPacket) is
  begin
    case i is
      when 0 => calc1.PMatPartPacket(packet);
      when 1 => calc2.PMatPartPacket(packet);
      when 2 => calc3.PMatPartPacket(packet);
      when 3 => calc4.PMatPartPacket(packet);
      when 4 => calc5.PMatPartPacket(packet);
      when 5 => calc6.PMatPartPacket(packet);
      when 6 => calc7.PMatPartPacket(packet);
      when 7 => calc8.PMatPartPacket(packet);
    end case;
  end;

  procedure callPMatPacket(i: ServerIds; packet: MatPacket) is
  begin
    case i is
      when 0 => calc1.PMatPacket(packet);
      when 1 => calc2.PMatPacket(packet);
      when 2 => calc3.PMatPacket(packet);
      when 3 => calc4.PMatPacket(packet);
      when 4 => calc5.PMatPacket(packet);
      when 5 => calc6.PMatPacket(packet);
      when 6 => calc7.PMatPacket(packet);
      when 7 => calc8.PMatPacket(packet);
    end case;
  end;

  procedure callPVecPartPacket(i: ServerIds; packet: VecPartPacket) is
  begin
    case i is
      when 0 => calc1.PVecPartPacket(packet);
      when 1 => calc2.PVecPartPacket(packet);
      when 2 => calc3.PVecPartPacket(packet);
      when 3 => calc4.PVecPartPacket(packet);
      when 4 => calc5.PVecPartPacket(packet);
      when 5 => calc6.PVecPartPacket(packet);
      when 6 => calc7.PVecPartPacket(packet);
      when 7 => calc8.PVecPartPacket(packet);
    end case;
  end;

  procedure callPVecPacket(i: ServerIds; packet: VecPacket) is
  begin
    case i is
      when 0 => calc1.PVecPacket(packet);
      when 1 => calc2.PVecPacket(packet);
      when 2 => calc3.PVecPacket(packet);
      when 3 => calc4.PVecPacket(packet);
      when 4 => calc5.PVecPacket(packet);
      when 5 => calc6.PVecPacket(packet);
      when 6 => calc7.PVecPacket(packet);
      when 7 => calc8.PVecPacket(packet);
    end case;
  end;

  procedure callPIntPacket(i: ServerIds; packet: IntPacket) is
  begin
    case i is
      when 0 => calc1.PIntPacket(packet);
      when 1 => calc2.PIntPacket(packet);
      when 2 => calc3.PIntPacket(packet);
      when 3 => calc4.PIntPacket(packet);
      when 4 => calc5.PIntPacket(packet);
      when 5 => calc6.PIntPacket(packet);
      when 6 => calc7.PIntPacket(packet);
      when 7 => calc8.PIntPacket(packet);
    end case;
  end;

  task body Calculation is
    index, parentIndex, nearCount: Integer;
    nexts: NextsArray;
    maxZs: NextsArray;
    zh: VectorPart;
    zhRecv: Boolean := False;
    maxZCalculated: Boolean := False;
    maxZ: Integer := Integer'First;
    maxZsCount: Integer := 0;
    AhsCount: Integer := 1;

    T, Z, E, A: Vector;
    Eh: VectorPart;
    d: Integer;
    MO, MK: Matrix;
    MKh: MatrixPart;

    TRecv: Boolean := False;
    dRecv: Boolean := False;
    MORecv: Boolean := False;
    EhRecv: Boolean := False;
    MKhRecv: Boolean := False;

    intPackets: Array (0..2*P-1) of IntPacket;
    intPacketsCount: Integer := 0;
    vecPackets: Array (0..2*P-1) of VecPacket;
    vecPacketsCount: Integer := 0;
    vecPartPackets: Array (0..2*P-1) of VecPartPacket;
    vecPartPacketsCount: Integer := 0;
    matPackets: Array (0..2*P-1) of MatPacket;
    matPacketsCount: Integer := 0;
    matPartPackets: Array (0..2*P-1) of MatPartPacket;
    matPartPacketsCount: Integer := 0;

    needFinish: Boolean := False;
  begin
    accept Init(i, parent, neighboursCount: Integer; ns: NextsArray) do
      index := i;
      Put_LINE("Begin thread " & Integer'Image(index));
      parentIndex := parent;
      nearCount := neighboursCount;
      nexts := ns;
      case index is
        when 0 =>
          T := initVector;
          d := 1;
          MO := initMatrix;  
        when 4 =>
          Z := initVector;

          for i in 0..P-1 loop
            if i /= index then
              declare
                ZhPacket: VecPartPacket;
              begin
                for j in Zh'Range loop
                  Zh(j) := Z(i*H+j);
                end loop;
                ZhPacket := (Zh, index, i, ZhData);
                vecPartPackets(vecPartPacketsCount) := ZhPacket;
                vecPartPacketsCount := vecPartPacketsCount + 1;
              end;
            end if;
          end loop;
          zhRecv := True;
        when 7 =>
          E := initVector;
          MK := initMatrix;

          for i in 0..H-1 loop
            Eh(i) := E(7*H+i);
            MKh(i) := MK(7*H+i);
          end loop;
        when others => null;
      end case;
    end;

    loop
      select
        accept PIntPacket(x: IntPacket) do
          if x.dest = index then
            case x.dataName is
              when ZMaxData =>
                maxZs(maxZsCount) := x.data;     
                maxZsCount := maxZsCount + 1;
              when finalZMaxData =>
                maxZ := x.data;
              when dData =>
                d := x.data;
                dRecv := True;
            end case;
          else
            intPackets(intPacketsCount) := x;
            intPacketsCount := intPacketsCount + 1;
          end if;
        end;
      or
        accept PVecPacket(vec: VecPacket) do
          if vec.dest = index then
            case vec.dataName is
              when TData =>
                T := vec.data;
                TRecv := True; 
            end case;
          else
            vecPackets(vecPacketsCount) := vec;
            vecPacketsCount := vecPacketsCount + 1;
          end if;
        end;
      or
        accept PMatPacket(mat: MatPacket) do
          if mat.dest = index then
            case mat.dataName is
              when MOData =>
                MO := mat.data;
                MORecv := True;
            end case;
          else
            matPackets(matPacketsCount) := mat;
            matPacketsCount := matPacketsCount + 1;
          end if;
        end;
      or
        accept PVecPartPacket(vech: VecPartPacket) do
          if vech.dest = index then
            case vech.dataName is
              when ZhData => 
                zh := vech.data;
                zhRecv := True;
              when EhData =>
                Eh := vech.data;
                EhRecv := True;
              when AhData =>
                for i in 0..H-1 loop
                  A(vech.src*H+i) := vech.data(i);
                end loop;
                AhsCount := AhsCount + 1;
            end case;
          else
            vecPartPackets(vecPacketsCount) := vech;
            vecPartPacketsCount := vecPartPacketsCount + 1;
          end if;
        end;
      or
        accept PMatPartPacket(math: MatPartPacket) do
          if math.dest = index then
            case math.dataName is
              when MKhData => 
                MKh := math.data;
                MKhRecv := True;
            end case;
          else
            matPartPackets(matPacketsCount) := math;
            matPartPacketsCount := matPartPacketsCount + 1;
          end if;
        end;
      or
        accept Finish do
          needFinish := True;
        end;
      else
        if needFinish then
          goto EndCalculation;
        end if;

        for i in 0..intPacketsCount-1 loop
          callPIntPacket(nexts(intPackets(i).dest), intPackets(i));
        end loop;
        intPacketsCount := 0;

        for i in 0..vecPartPacketsCount-1 loop
          callPVecPartPacket(nexts(vecPartPackets(i).dest), vecPartPackets(i));
        end loop;
        vecPartPacketsCount := 0;

        for i in 0..vecPacketsCount-1 loop
          callPVecPacket(nexts(vecPackets(i).dest), vecPackets(i));
        end loop;
        vecPacketsCount := 0;

        for i in 0..matPacketsCount-1 loop
          callPMatPacket(nexts(matPackets(i).dest), matPackets(i));
        end loop;
        matPacketsCount := 0;

        for i in 0..matPartPacketsCount-1 loop
          callPMatPartPacket(nexts(matPartPackets(i).dest), matPartPackets(i));
        end loop;
        matPartPacketsCount := 0;

        if zhRecv then
          if not maxZCalculated then
            for i in zh'Range loop
              maxZ := Integer'Max(maxZ, zh(i));
            end loop;
            maxZCalculated := True;
          end if;

          if maxZsCount = nearCount then
            for i in 0..maxZsCount-1 loop
              maxZ := Integer'Max(maxZ, maxZs(i));
            end loop;
            if parentIndex /= -1 then
              declare
                newPacket: IntPacket := (maxZ, parentIndex, ZMaxData);
              begin
                intPackets(intPacketsCount) := newPacket;
                intPacketsCount := intPacketsCount + 1;
              end;
            else
              for i in 1..P-1 loop
                declare
                  newPacket: IntPacket := (maxZ, i, FinalZMaxData);
                begin
                  intPackets(intPacketsCount) := newPacket;
                  intPacketsCount := intPacketsCount + 1;
                end;
                declare
                  TPacket: VecPacket := (T, i, TData);
                  dPacket: IntPacket := (d, i, dData);
                  MOPacket: MatPacket := (MO, i, MOData);
                begin
                  vecPackets(vecPacketsCount) := TPacket;
                  vecPacketsCount := vecPacketsCount + 1;
                  intPackets(intPacketsCount) := dPacket;
                  intPacketsCount := intPacketsCount + 1;
                  matPackets(matPacketsCount) := MOPacket;
                  matPacketsCount := matPacketsCount + 1;

                  TRecv := True;
                  dRecv := True;
                  MORecv := True;
                end;
              end loop;
            end if;
            zhRecv := False;
          end if;
        end if;

        if TRecv and dRecv and MORecv then
          if index = 7 then
            for i in 0..P-1 loop
              if i /= index then
                declare
                  Eh: VectorPart;
                  MKh: MatrixPart;
                begin
                  for i in 0..H-1 loop
                    Eh(i) := E(i*H+i);
                  end loop;
                  for i in 0..H-1 loop
                    MKh(i) := MK(i*H+i);
                  end loop;
                declare
                  EhPacket: VecPartPacket := (Eh, index, i, EhData);
                  MKhPacket: MatPartPacket := (MKh, i, MKhData);
                begin
                  vecPartPackets(vecPartPacketsCount) := EhPacket;
                  vecPartPacketsCount := vecPartPacketsCount + 1;
                  matPartPackets(matPartPacketsCount) := MKhPacket;
                  matPartPacketsCount := matPartPacketsCount + 1;

                  EhRecv := True;
                  MKhRecv := True;
                end;
                end;
              end if;
            end loop;
          end if;
          TRecv := False;
          dRecv := False;
          MORecv := False;
        end if;

        if EhRecv and MKhRecv then
          declare
            Ah: VectorPart;
            mok: Integer;
          begin
            for i in 0..H-1 loop
              Ah(i) := maxZ * Eh(i);
              for j in 0..N-1 loop
                mok := 0;
                for k in 0..N-1 loop
                  mok := mok + MO(j)(k) * MKh(i)(k);
                end loop;
                Ah(i) := Ah(i) + d * mok * T(j);
              end loop;
            end loop;
            if index /= 4 then
              declare
                newPacket: VecPartPacket := (Ah, index, 4, AhData);
              begin
                vecPartPackets(vecPartPacketsCount) := newPacket;
                vecPartPacketsCount := vecPartPacketsCount + 1;
              end;
            else
              for i in 0..H-1 loop
                A(4*H+i) := Ah(i);
              end loop;
            end if;
          end;
          EhRecv := False;
          MKhRecv := False;
        end if;

        if AhsCount = P then
          for i in 0..N-1 loop
            put(Integer'Image(A(i)));
          end loop;
          put_line("");
          calc1.Finish;
          calc2.Finish;
          calc3.Finish;
          calc4.Finish;
          calc6.Finish;
          calc7.Finish;
          calc8.Finish;
          goto EndCalculation;
        end if;

        delay 0.1;
      end select;
    end loop;
    <<EndCalculation>>
    put_line("Trhead finished " & Integer'Image(index));
  end Calculation;

  nextHop1: NextsArray := (0, 1, 1, 1, 1, 1, 1, 1);
  nextHop2: NextsArray := (0, 1, 2, 2, 2, 5, 5, 5);
  nextHop3: NextsArray := (1, 1, 2, 3, 4, 1, 1, 1);
  nextHop4: NextsArray := (2, 2, 2, 3, 2, 2, 2, 2);
  nextHop5: NextsArray := (2, 2, 2, 2, 4, 2, 2, 2);
  nextHop6: NextsArray := (1, 1, 1, 1, 1, 5, 6, 6);
  nextHop7: NextsArray := (5, 5, 5, 5, 5, 5, 6, 7);
  nextHop8: NextsArray := (6, 6, 6, 6, 6, 6, 6, 7);
begin
  calc1.Init(0, -1, 1, nextHop1);
  calc2.Init(1,  0, 2, nextHop2);
  calc3.Init(2,  1, 2, nextHop3);
  calc4.Init(3,  2, 0, nextHop4);
  calc5.Init(4,  2, 0, nextHop5);
  calc6.Init(5,  1, 1, nextHop6);
  calc7.Init(6,  5, 1, nextHop7);
  calc8.Init(7,  6, 0, nextHop8);
end;
