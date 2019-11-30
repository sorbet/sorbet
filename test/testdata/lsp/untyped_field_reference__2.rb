# typed: true

class A
  def bar
    @x
    #^ usage: x-def
    #^ type: x-type
  end
end
