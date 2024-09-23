program MultiplicaValores;
var
    x, y, z: integer;
begin
    x := 2;
    y := 3;
    if x < y then
    begin
        z := x * y;
        writeln('Resultado: ', z);
    end;
end.
