# typed: true

module Runnable
#      ^^^^^^^^ find-implementation: Runnable
  extend T::Sig
  extend T::Helpers
  interface!

  sig { abstract.params(args: T::Array[String]).void }
  def run(args); end
#     ^^^ find-implementation: run
end

 class HelloWorld
#^^^^^^^^^^^^^^^^ implementation: Runnable
  extend T::Sig
  include Runnable
#         ^^^^^^^^ find-implementation: Runnable
  sig { override.params(args: T::Array[String]).void }
  def run(args)
# ^^^^^^^^^^^^^ implementation: run
#     ^^^ find-implementation: run
    puts 'Hello World!'
  end
end

 class ByeByeWorld
#^^^^^^^^^^^^^^^^^ implementation: Runnable
  extend T::Sig
  include Runnable
#         ^^^^^^^^ find-implementation: Runnable
  sig { override.params(args: T::Array[String]).void }
  def run(args)
# ^^^^^^^^^^^^^ implementation: run
#     ^^^ find-implementation: run
    puts 'Farewell World!'
  end
end

class Main
    extend T::Sig
  
    sig {params(x: String).void}
    def self.main(x)
        hello = T.let(HelloWorld.new, Runnable)
#                                     ^^^^^^^^ find-implementation: Runnable
        byebye = T.let(ByeByeWorld.new, Runnable)
#                                       ^^^^^^^^ find-implementation: Runnable
        hello.run([])
#             ^^^ find-implementation: run
        HelloWorld.new.run([])
#                      ^^^ find-implementation: run
    end

    sig {params(runnable: Runnable).void}
#                         ^^^^^^^^ find-implementation: Runnable
    def self.run(runnable)
        runnable.run([])
#                ^^^ find-implementation: run
    end
end
