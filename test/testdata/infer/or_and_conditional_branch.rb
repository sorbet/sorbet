# typed: strong
extend T::Sig

sig {params(lines: T::Array[String]).void}
def foo1(lines)
  min_space = lines
    .map do |line|
      if line.strip.empty?
        nil
      else
        T.unsafe(line)
      end
    end
  # ^^^ error: Value returned from block is `T.untyped`
    .compact
    .min || 0
    #   ^^^^ error: Conditional branch on `T.untyped`
end

sig {params(lines: T::Array[String]).void}
def foo2(lines)
  min_space = lines
    .map do |line|
      if line.strip.empty?
        nil
      else
        T.unsafe(line)
      end
    end
  # ^^^ error: Value returned from block is `T.untyped`
    .compact
    .min && 0
    #   ^^^^ error: Conditional branch on `T.untyped`
end

def dont_crash # error: does not have a `sig`
  x = begin; end || begin; end
  #   ^^^^^^^^^^ error: Left side of `||` condition was always `falsy`
  x = begin; end && begin; end
  #                 ^^^^^^^^^^ error: This code is unreachable
  x = () || ()
  #   ^^ error: Left side of `||` condition was always `falsy`
  x = () && ()
  #         ^^ error: This code is unreachable
end
