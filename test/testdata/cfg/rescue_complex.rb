# @typed
def meth; 0; end
def foo; 1; end
def bar; 2; end
def baz; 3; end
def take_arg(x); x; end

def multiple_rescue()
  begin
      meth
  rescue
      baz
  rescue
      bar
  end
end

def multiple_rescue_classes()
  begin
      meth
  rescue Foo, Bar => baz # error: Stubbing out unknown constant
      baz
  end
end

def parse_rescue_ensure()
  begin; meth; rescue; baz; ensure; bar; end
end

def parse_bug_rescue_empty_else()
  begin; rescue LoadError; else; end
end

def parse_ruby_bug_12686()
  take_arg (bar rescue nil)
end

def parse_rescue_mod()
  meth rescue bar
end

def parse_resbody_list_var()
  begin; meth; rescue foo => ex; bar; end
end

def parse_rescue_else_ensure()
  begin; meth; rescue; baz; else foo; ensure; bar end
end

def parse_rescue()
  begin; meth; rescue; foo; end
end

def parse_resbody_var()
  begin; meth; rescue => ex; bar; end
end

def parse_resbody_var_1()
  begin; meth; rescue => @ex; bar; end # error: Use of undeclared variable `@ex'
end

def parse_rescue_mod_op_assign()
  foo += meth rescue bar # error: Method + does not exist on NilClass
end

def parse_ruby_bug_12402()
  foo = raise(bar) rescue nil
end

def parse_ruby_bug_12402_1()
  foo += raise(bar) rescue nil # error: Method + does not exist on NilClass
end

def parse_ruby_bug_12402_2()
  foo[0] += raise(bar) rescue nil
end
