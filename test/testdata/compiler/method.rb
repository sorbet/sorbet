# frozen_string_literal: true
# typed: true
# compiled: true

def hello(name)
   i = 0
   while (i < 10)
     puts("hello " + name)
     i += 1
   end
end
hello("sorbet")

