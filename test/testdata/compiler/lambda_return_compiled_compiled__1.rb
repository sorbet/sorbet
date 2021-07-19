# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './lambda_return_compiled_compiled__2'

# This test is the one that motivates the changes to vm_yield_with_cfunc (see third_party/ruby/is-lambda-ifunc.patch)
# which pop arguments to ifuncs _before_ calling them. If the VM is not patched, the EC_JUMP_TAG for the "return"
# statement inside the compiled lambda will jump over the POPN(argc) that is supposed to happen after the ifunc
# returns.
puts F0.call(100)
puts M::F1.call(200)
puts C::F2.call(400)
puts C.f3.call(300)
puts C.new.f4.call(500)
puts F5.call.call(600)
puts F8.call(449,451)
