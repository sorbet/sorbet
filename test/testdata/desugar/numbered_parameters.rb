# typed: true

def foo
  yield(42, "foo")
end

foo { _1 }
foo { _9 }
foo { _2 * _1 }

foo do _1 end
foo do _9 end
foo do _2 * _1 end

-> { _1 }
-> { _9 }
-> { _2 * _1 }
