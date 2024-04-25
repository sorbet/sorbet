# frozen_string_literal: true
# typed: true
# compiled: true
arglist = [1,2,3,4]
p(*arglist)
p(0,*arglist,5)
p(*[])
p(0,*[],22)

def f(a,b,c)
  yield (a+b+c)
end

f(*[1,2,3]) { |x| p x }
f(1,*[2,3]) { |x| p x }
f(1,*[2],3) { |x| p x }
