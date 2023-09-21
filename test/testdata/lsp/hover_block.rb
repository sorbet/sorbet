# typed: strict
extend T::Sig

sig {params(classes: T::Array[T.untyped]).returns(T::Array[[T.nilable(Integer), T.nilable(String)]])}
def f(classes)
  result = classes.map do |cls|
# ^ hover: T::Array[[T.nilable(Integer), T.nilable(String)]]
    begin
      cls.call_func

      [nil, "foo"]
    rescue => e
      [6, nil]
    end
  end

  T.reveal_type(result) # error: Revealed type: `T::Array[[T.nilable(Integer), T.nilable(String)]]`
  result
end

class Parent; end

sig {params(classes: T::Array[Module]).void}
def example(classes)
  thing =
    if classes.empty?
      T.unsafe(nil) ? [Hash] : []
    else
      classes
    end
  T.reveal_type(thing) # error: Revealed type: `T::Array[Module]`
  thing
  # ^ hover: T::Array[Module]
end
