program analisador;

var
    num1, num2, result: integer;
    operation: char;

begin
    num1 := 10;
    num2 := 20;

    write('Enter operation (+, -, *, /): ');
    read(operation);

    if operation = '+' then
        result := num1 + num2
    else if operation = '-' then
        result := num1 - num2
    else if operation = '*' then
        result := num1 * num2
    else if operation = '/' then
        result := num1 / num2
    else
        write('Error: Invalid operation.');

    write('The result is: ', result);
end.
