# typed: false

def m
  super # Invoke super, forwarding all the arguments, unmodified

  super() # Invoke super, explicitly with no arguments

  super(1, 2, 3)

  super 1, 2, 3

  # Invoke super with a block
  super() do
    yield
  end

  super(1, 2, 3) do
    yield
  end

  super 1, 2, 3 do
    yield
  end

  # what about operators and square brackets?
  # super[1, 2] = 3

  # Invoke super with a block, forwarding all arguments
  super do
    yield
  end
end
