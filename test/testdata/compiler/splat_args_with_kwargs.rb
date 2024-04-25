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

class A
  def self.f(a,b,c,d: 5)
    yield (a+b+c+d)
  end
end

A.f(*[1,2,3],**{d:4}) { |x| p x }
A.f(*[1,2,3],d: 4) { |x| p x }
A.f(*[1,2,3]) { |x| p x }
A.f(*[1,2,3,{}]) { |x| p x }
A.f(*[1,2,3,{d: 10}]) { |x| p x }

def expects_ArgumentError
  begin
    yield
  rescue ArgumentError => e
    p e.message
  rescue => e
    puts "Didn't get ArgumentError: #{e.class} #{e.message}"
  end
end

# Regular splat arguments are not re-processed as keyword args.
expects_ArgumentError do
  T.unsafe(A).f(*[1,2,3,:d,10]) {|x| p x}
end
expects_ArgumentError do
  T.unsafe(A).f(*[1,2,3,:d,10], d: 9) {|x| p x}
end
# Splat args with a kwargs-style hash are not re-processed if the call has kwargs.
expects_ArgumentError do
  T.unsafe(A).f(*[1,2,3,{d: 10}], d: 9) {|x| p x}
end
# Splat args with a kwargs-style hash are not re-processed if the call has a kwsplat
expects_ArgumentError do
  T.unsafe(A).f(*[1,2,3,{d: 10}], **{d: 9}) {|x| p x}
end
# Number of args checks are done before valid/invalid kwargs checks.
expects_ArgumentError do
  T.unsafe(A).f(*[1,2,3,{d: 10}], e: 9) {|x| p x}
end

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
