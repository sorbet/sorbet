# typed: true
extend T::Sig

sig {params(x: String).void}
def handling_after_absurd(x)
  unless x.is_a?(String)
    T.absurd(x)
    raise "Unexpected value: #{x}"
  end
end

sig {params(x: String).void}
def code_after_raise(x)
  unless x.is_a?(String)
    raise "Unexpected value: #{x}"
    puts x
    T.absurd(x)
  end
end

sig {void}
def ordinary_code_after_raise
  raise "stop"
  puts "later"
end
