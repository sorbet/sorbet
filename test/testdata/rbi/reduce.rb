# typed: true
class Thing
end

class Main
  extend T::Sig

  sig {params(x: Integer).returns(T::Array[Thing])}
  def bar(x)
    result = [Thing.new, Thing.new].reduce(0) do |i, thing|
      T.reveal_type(thing) # error: Revealed type: `Thing`
      thing.non_existant_method # error: does not exist
    end
    result # error: does not conform to method result type
  end
end

def main
  Main.new.bar(91)
end
