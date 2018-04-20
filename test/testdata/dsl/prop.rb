# typed: strict
class NotAODM
    def self.prop(*args); end
    prop
    prop :foo, :not_a_string
    prop "not_a_symbol", String
    prop :foo, String, "not_a_hash"
    prop "too", String, {}, "many"
    optional :company_name, String, :nonempty_string # error: Method `optional` does not exist
    optional :day, IntegerParam.new(min: 1, max: 31) # error: MULTI
    optional :name, StringParam.alphanumeric # error: MULTI
    optional :how_many, Opus::Param::CaseParam.new(self.how_many_cases, Opus::Param::ParamSpecsParam.new(Default)) # error: MULTI
    optional :optional_param, IntegerParam.new # error: Method `optional` does not exist
end

class SomeODM
    prop :foo, String

    sig.returns(T.nilable(String))
    def foo2; T.cast(nil, T.nilable(String)); end
    sig(arg0: String).returns(String)
    def foo2=(arg0); T.cast(nil, String); end
end

class ForeignClass
end

class AdvancedODM
    prop :default_without_migrate, String, default: ""
    prop :default, String, default: "", optional: :migrate
    prop :nodefault, String
    prop :factory_without_migrate, String, factory: -> {""}
    prop :factory, String, factory: -> {""}, optional: :migrate

    prop :type, type: String, default: "", optional: :migrate
    prop :object
    prop :array, Array, default: [], optional: :migrate
    prop :array_of, array: String, default: "", optional: :migrate
    prop :array_of_explicit, Array, array: String, default: [""], optional: :migrate
    prop :t_array, T::Array[String], default: [""], optional: :migrate
    prop :hash_of, T::Hash[Symbol, String], default: {a: ""}, optional: :migrate

    prop :optional_explicit, String, optional: true, default: "", optional: :migrate
    prop :optional_existing, String, optional: :existing, default: "", optional: :migrate
    prop :optional_false, String, optional: false
    optional :optional, String, default: "", optional: :migrate
    optional :optional_nilable, T.nilable(String), optional: :migrate

    prop :const_explicit, String, immutable: true, default: "", optional: :migrate
    const :const, String, default: "", optional: :migrate

    prop :no_class_arg, type: Array, immutable: true, optional: true, array: String, default: "", optional: :migrate

    prop :enum_prop, enum: ["hello", "goodbye"]

    prop :foreign, String, foreign: ForeignClass
    prop :foreign_lazy, String, foreign: -> {ForeignClass}
    prop :foreign_proc, String, foreign: proc {ForeignClass}
    prop :foreign_invalid, String, foreign: proc { :not_a_type }
end

class PropHelpers
  token_prop
  created_prop
end

class PropHelpers2
  timestamped_token_prop
  created_prop(immutable: true)
end

class ShardingProp
  merchant_prop
end

def main
    T.assert_type!(SomeODM.new.foo, T.nilable(String))
    T.assert_type!(SomeODM.new.foo, String) # error: Argument does not have asserted type
    T.assert_type!(SomeODM.new.foo = 'b', String)
    T.assert_type!(SomeODM.new.foo2, T.nilable(String))
    T.assert_type!(SomeODM.new.foo2, String) # error: Argument does not have asserted type
    T.assert_type!(SomeODM.new.foo2 = 'b', String)

    T.assert_type!(AdvancedODM.new.default_without_migrate, T.nilable(String))
    T.assert_type!(AdvancedODM.new.default, String)
    T.assert_type!(AdvancedODM.new.nodefault, T.nilable(String))
    T.assert_type!(AdvancedODM.new.nodefault, String) # error: Argument does not have asserted type
    T.assert_type!(AdvancedODM.new.factory_without_migrate, T.nilable(String))
    T.assert_type!(AdvancedODM.new.factory, String)

    T.assert_type!(AdvancedODM.new.type, String)
    T.assert_type!(AdvancedODM.new.object, T.nilable(Object))
    # Sadly I can't check this for allowed nillnes since NilClass is an Object
    # T.assert_type!(AdvancedODM.new.object, Object) # not-a-error: argument does not have asserted type
    T.assert_type!(AdvancedODM.new.array, Array)
    T.assert_type!(AdvancedODM.new.array_of, T::Array[String])
    T.assert_type!(AdvancedODM.new.array_of_explicit, T::Array[String])
    T.assert_type!(AdvancedODM.new.t_array, T::Array[String])
    T.assert_type!(AdvancedODM.new.hash_of, T::Hash[Symbol, String])

    T.assert_type!(AdvancedODM.new.optional_explicit, T.nilable(String))
    T.assert_type!(AdvancedODM.new.optional_existing, T.nilable(String))
    AdvancedODM.new.optional_existing = 'b'
    AdvancedODM.new.optional_existing = nil # error: does not match expected type
    T.assert_type!(AdvancedODM.new.optional_false, T.nilable(String))
    T.assert_type!(AdvancedODM.new.optional, T.nilable(String))
    AdvancedODM.new.optional = 'b'
    AdvancedODM.new.optional = nil
    T.assert_type!(AdvancedODM.new.optional_nilable, T.nilable(String))

    T.assert_type!(AdvancedODM.new.const_explicit, String)
    AdvancedODM.new.const_explicit = 'b' # error: Method `const_explicit=` does not exist on `AdvancedODM`
    T.assert_type!(AdvancedODM.new.const, String)
    AdvancedODM.new.const = 'b' # error: Method `const=` does not exist on `AdvancedODM`

    T.assert_type!(AdvancedODM.new.no_class_arg, T.nilable(T::Array[String]))
    AdvancedODM.new.no_class_arg = ['b'] # error: Method `no_class_arg=` does not exist on `AdvancedODM`

    # T.assert_type!(AdvancedODM.new.enum_prop, T.noreturn) # no longer true
    AdvancedODM.new.enum_prop = "hello" # error: Method `enum_prop=` does not exist

    T.assert_type!(AdvancedODM.new.foreign_, T.nilable(ForeignClass))
    T.assert_type!(AdvancedODM.new.foreign_, ForeignClass) # error: Argument does not have asserted type
    T.assert_type!(AdvancedODM.new.foreign_lazy_, T.nilable(ForeignClass))

    # Check that the method still exists even if we can't parse the type
    AdvancedODM.new.foreign_invalid_

    T.assert_type!(PropHelpers.new.token, String)
    PropHelpers.new.token = "tok_token"
    PropHelpers.new.token = nil # error: does not match expected type

    T.assert_type!(PropHelpers.new.created, Float)
    PropHelpers.new.created = 0.0
    PropHelpers.new.created = nil # error: does not match expected type

    T.assert_type!(PropHelpers2.new.token, String)
    PropHelpers2.new.token = "tok_token"
    PropHelpers2.new.token = nil # error: does not match expected type

    T.assert_type!(PropHelpers2.new.created, Float)
    PropHelpers2.new.created = 0.0 # error: Method `created=` does not exist

    T.assert_type!(ShardingProp.new.merchant, String)
    ShardingProp.new.merchant = "hi" # error: Method `merchant=` does not exist
end
