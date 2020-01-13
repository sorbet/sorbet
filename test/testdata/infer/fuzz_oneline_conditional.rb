# typed: true
if foo # error: Method `foo` does not exist on `T.class_of(<root>)`
  bar = 3
end
quz = foo if quz || bar # error: Method `foo` does not exist on `T.class_of(<root>)`
           # ^^^ error: This code is unreachable
quz
