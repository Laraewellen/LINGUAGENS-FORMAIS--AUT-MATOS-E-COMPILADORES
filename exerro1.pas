program Erro1;
var
    a, b, resultado: integer;
begin
    a := 5;
    b := 3;
    resultado := a %% b;  { erro: %% não é um operador válido }
    writeln('Resultado: ', resultado);
end.
