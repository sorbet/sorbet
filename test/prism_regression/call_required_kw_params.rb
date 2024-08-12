# typed: true

 foo(a: 1)
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 foo a: 1
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 bar(a: 2, b: 3)
#^^^ error: Method `bar` does not exist on `T.class_of(<root>)`

 bar a: 2, b: 3
#^^^ error: Method `bar` does not exist on `T.class_of(<root>)`
