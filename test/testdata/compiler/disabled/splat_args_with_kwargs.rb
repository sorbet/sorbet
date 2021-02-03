# frozen_string_literal: true
# typed: true
# compiled: true
arglist = [1,2,3,4]
arghash = {a: 9, b: 10}
p(0,*arglist,**arghash)
p(0,*arglist,c: 22,**arghash)
p(*arglist,c: 22,**arghash)
p(*arglist,c: 22)
p(*arglist,c: 22, d:23)

p(*[],c: 22)
p(0,*[],c: 22)
p(0,*[],**{})
p(0,*[],c: 22,**{})
p(*[],c: 22,**{})

def f(a,b,c,d:)
  yield (a+b+c+d)
end

f(*[1,2,3],**{d:4}) { |x| p x }
