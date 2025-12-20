# This is a regression test for a stack corruption bug introduced by
# https://github.com/stripe/sorbet_llvm/pull/437 (since reverted).

# This function takes "a lot" of arguments.
def many_args(
  a0,b0,c0,d0,e0,f0,g0,h0,i0,j0,k0,l0,m0,n0,o0,p0,q0,r0,s0,t0,u0,v0,w0,x0,y0,z0,
  a1,b1,c1,d1,e1,f1,g1,h1,i1,j1,k1,l1,m1,n1,o1,p1,q1,r1,s1,t1,u1,v1,w1,x1,y1,z1,
  a2,b2,c2,d2,e2,f2,g2,h2,i2,j2,k2,l2,m2,n2,o2,p2,q2,r2,s2,t2,u2,v2,w2,x2,y2,z2
  )
  puts "in many_args"
end

# Let's call it once, to ~fill the stack area with object references.
class C ; end
def c
  C.new
end

many_args(c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,
          c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,
          c,c,c,c,c,c,c,c,c,c)

# GC once, to make sure all the objects that were on many_args's stack get
# freed.
GC.start

# Now 'each_slice' will invoke 'each' with an IFUNC-type handler and a "gap" in
# the stack which will probably contain a stale reference to a freed object. If
# gc happens under those circumstances, it will trigger a "try to mark T_NONE"
# assertion during the mark phase.
["whatever"].each_slice(1) do |_|
  GC.start
end

puts "done"
