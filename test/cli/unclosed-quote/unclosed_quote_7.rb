# typed: false

def before; end

def foo
  x = "first line
  second line"

  x = "unclosed string
end

def after; end
