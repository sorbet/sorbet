# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

class NotHash
  def [](arg0)
    "got key: #{arg0}"
  end
end

sig {params(x: T.any(T::Hash[T.untyped, T.untyped], NotHash)).returns(T.untyped)}
def do_aref(x)
  x[:foo]
end

# Try and ensure that we inline for the hash case and don't call the vm-like
# fastpath, since we have type information.

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#7do_aref"
# INITIAL-NOT: call i64 @sorbet_vm_aref
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL-NOT: call i64 @sorbet_vm_aref
# INITIAL: call i64 @sorbet_rb_hash_square_br
# INITIAL-NOT: call i64 @sorbet_vm_aref
# INITIAL{LITERAL}: }

def do_aref_notype(x)
  x[:foo]
end

# Make sure we call our vm-like fastpath for untyped args.

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#14do_aref_notype"
# INITIAL-NOT: call i64 @sorbet_rb_hash_square_br
# INITIAL: call i64 @sorbet_vm_aref
# INITIAL-NOT: call i64 @sorbet_rb_hash_square_br
# INITIAL{LITERAL}: }

hash = {foo: 627}
p hash

3.times do
  y = do_aref(hash)
  p y

  p do_aref_notype(hash)

  z = do_aref(NotHash.new)
  p z

  p do_aref_notype(NotHash.new)
end
