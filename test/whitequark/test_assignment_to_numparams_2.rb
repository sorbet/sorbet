# typed: true

proc {_1; _1, foo = [nil, nil]} # error: _1 is reserved for numbered parameter
                            # ^ error: unexpected token tRCURLY
