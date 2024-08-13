# typed: true

 foo(1)
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 foo 1
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 bar(2, 3)
#^^^ error: Method `bar` does not exist on `T.class_of(<root>)`

 bar 2, 3
#^^^ error: Method `bar` does not exist on `T.class_of(<root>)`
