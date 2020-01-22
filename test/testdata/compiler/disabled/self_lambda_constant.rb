# typed: true
# compiled: true

module Main
  def self.foo(x)
    p x
  end

  XYZ = -> {foo(1)}

  def self.main
    XYZ.call
  end
end

Main.main
