# typed: true

def foo
  (class << self; self; end).class_eval # error: Method `class_eval` does not exist on `NilClass`
end
