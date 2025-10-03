# typed: true

class A
  extend T::Sig

  sig { params(arg1: Integer, arg2: String, message: String).void }
  def self.test(arg1:, arg2: "", message: "")
  end
end

A.test(a) # error: Method `a` does not exist
      # ^ completion: alias, and, arg1: (keyword argument), arg2: (keyword argument), ...

A.test(mess)
#          ^ completion: message: (keyword argument)
#      ^^^^  error: Method `mess` does not exist

A.test(arg1: 10, mess)
#                    ^ completion: message: (keyword argument)
#                ^^^^  error: Method `mess` does not exist
#                ^^^^  error: Unrecognized keyword argument
#                ^^^^  error: positional arg

A.test(mess)
#      ^^^^  error: Method `mess` does not exist
#          ^ apply-completion: [A] item: 0

A.test(message: "hi", ar)
#                       ^ apply-completion: [B] item: 0
#                     ^^  error: Method `ar` does not exist
#                       ^  error: Missing required keyword argument
#                     ^^  error: Unrecognized keyword argument
#                     ^^  error: positional arg
