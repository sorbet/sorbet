# typed: true
def f(a, b); end;
f 1, {1 => 2} {1}
            # ^ error: unexpected token tLCURLY
