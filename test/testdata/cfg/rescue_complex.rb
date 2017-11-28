def multiple_rescue()
  begin
      meth # error: Method meth does not exist on Object
  rescue
      baz # error: Method baz does not exist on Object
  rescue
      bar # error: Method bar does not exist on Object
  end
end

def multiple_rescue_classes()
  begin
      meth # error: Method meth does not exist on Object
  rescue Foo, Bar => baz # error: Stubbing out unknown constant
      baz
  end
end

def parse_rescue_ensure()
  begin; meth; rescue; baz; ensure; bar; end # error: Unsupported node type Ensure
end

def parse_bug_rescue_empty_else()
  begin; rescue LoadError; else; end
end

def parse_ruby_bug_12686()
  f (g rescue nil) # error: does not exist on Object
end

def parse_rescue_mod()
  meth rescue bar # error: does not exist on Object
end

def parse_resbody_list_var()
  begin; meth; rescue foo => ex; bar; end # error: does not exist on Object
end

def parse_rescue_else_ensure()
  begin; meth; rescue; baz; else foo; ensure; bar end # error: Unsupported node type Ensure
end

def parse_rescue()
  begin; meth; rescue; foo; end # error: does not exist on Object
end

def parse_resbody_var()
  begin; meth; rescue => ex; bar; end # error: does not exist on Object
end

def parse_resbody_var_1()
  begin; meth; rescue => @ex; bar; end # error: MULTI
end

def parse_rescue_mod_op_assign()
  foo += meth rescue bar # error: Unsupported node type OpAsgn
end

def parse_ruby_bug_12402()
  foo = raise(bar) rescue nil # error: does not exist on Object
end

def parse_ruby_bug_12402_1()
  foo += raise(bar) rescue nil # error: Unsupported node type OpAsgn
end

def parse_ruby_bug_12402_2()
  foo[0] += raise(bar) rescue nil # error: Unsupported node type OpAsgn
end
