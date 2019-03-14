# typed: true
def foo; end;
def bar; end;
foo::C ||= raise(bar) rescue nil # error: Constant reassignment is not supported
