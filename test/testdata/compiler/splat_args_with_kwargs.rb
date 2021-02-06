# frozen_string_literal: true
# typed: true
# compiled: true
arglist = [1,2,3,4]
arghash = {a: 9, b: 10}
p(*arglist,**arghash)
p arglist
p(6,3,2,99,*arglist,**arghash)
p(0,*arglist,c: 22,**arghash)
p(*arglist,c: 22,**arghash)
p(*arglist,c: 22)
p(*arglist,c: 22, d:23)

p(*[],c: 22)
p(0,*[],c: 22)
p(0,*[],**{})
p(0,*[],c: 22,**{})
p(*[],c: 22,**{})

def f(a,b,c,d: 5)
  yield (a+b+c+d)
end

f(*[1,2,3],**{d:4}) { |x| p x }
f(*[1,2,3],d: 4) { |x| p x }
f(*[1,2,3]) { |x| p x }

def g(*args,**kwargs)
  args.push "oops"
  kwargs[:oops] = "oh no"
  kwargs[:we] = "oh no"
end

l = [1,2,3,4]
h = {we: "are fine"}

g(*l,**h)
p l
p h
