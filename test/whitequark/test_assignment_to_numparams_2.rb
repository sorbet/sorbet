# typed: true

proc {_1; _1, foo = [nil, nil]} # error: cannot assign to numbered parameter _1
                            # ^ error: unexpected token tRCURLY
