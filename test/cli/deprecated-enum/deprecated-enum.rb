# typed: true

extend T::Sig

sig {params(x: T.enum([:yes, :no])).void}
def example1(x)
end

sig {params(x: ::T.enum([:yes, :no])).void}
def example2(x)
end

sig {params(x: T.enum(


  [:yes, :no])).void}
def example3(x)
end

sig {params(x: ::T.enum(


  [:yes, :no])).void}
def example4(x)
end


T.cast(:yes, T.enum)
T.cast(:yes, T.enum([:yes]))
puts nil
