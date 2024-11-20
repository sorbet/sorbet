# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

def show_type_error(&blk)
  begin
    yield
  rescue TypeError => exception
    # sorbet-runtime includes three lines, something like:
    #
    #   TypeError Parameter 'x': Expected type Integer, got type String with value "hello"
    #   Caller /home/aprocter/stripe/sorbet/foo.rb:20
    #   Definition /home/aprocter/stripe/sorbet/foo.rb:16
    #
    # The compiler currently only produces the first of these lines.
    exception_line = exception.message.split("\n").fetch(0)

    # We also don't render the value quite correctly in some cases.
    exception_line = exception_line.gsub(/with value .*$/, '')

    puts exception_line
  end
end

sig {params(x: Integer).void}
def f(x) ; STDERR.puts "in f, x is #{x}" ; end

sig {params(x: Integer).void}
def g(x:) ; STDERR.puts "in g, x is #{x}" ; end

sig {params(x: Integer).void}
def h(x=38) ; STDERR.puts "in h, x is #{x}" ; end

sig {params(x: Integer).void}
def i(x: 38) ; STDERR.puts "in i, x is #{x}" ; end

sig {params(x: Integer, y: Float).void}
def j(x, y) ; STDERR.puts "in j, x is #{x} and y is #{y}" ; end

sig {params(x: T.any(Float, Integer)).void}
def k(x) ; STDERR.puts "in j, x is #{x}" ; end

show_type_error { f(T.unsafe(33.0)) }
show_type_error { g(x: T.unsafe(33.0)) }
show_type_error { h(T.unsafe(33.0)) }
show_type_error { i(x: T.unsafe(33.0)) }
show_type_error { j(T.unsafe(33.0), T.unsafe(5)) }
show_type_error { k(T.unsafe("hocus pocus")) }
