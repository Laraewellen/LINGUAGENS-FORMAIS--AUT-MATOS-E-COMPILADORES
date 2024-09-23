program VerificaNumero;
var
    numero: integer;
begin
    numero := 5;
    
    if numero > 0 then
        writeln('Numero positivo')
    else
        writeln('Numero negativo');
    
    while numero < 10 do
    begin
        numero := numero + 1;
        writeln('Numero: ', numero);
    end;
end.
