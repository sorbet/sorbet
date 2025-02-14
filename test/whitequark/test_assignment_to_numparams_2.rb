# typed: true

proc {_1; _1, foo = [nil, nil]} # parser-error: cannot assign to numbered parameter `_1`
