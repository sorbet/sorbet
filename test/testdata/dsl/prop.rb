# typed: true
class NotAODM
    def self.prop(*args); end
    prop
    prop :foo, :not_a_string
    prop "not_a_symbol", String
    prop :foo, String, "not_a_hash"
    prop "too", String, {}, "many"
    optional :company_name, String, :nonempty_string # error: Method `optional` does not exist
    optional :day, IntegerParam.new(min: 1, max: 31) # error: Method `optional` does not exist on `T.class_of(NotAODM)`
                 # ^^^^^^^^^^^^ error: Unable to resolve constant `IntegerParam`
    optional :name, StringParam.alphanumeric # error: Method `optional` does not exist on `T.class_of(NotAODM)`
                  # ^^^^^^^^^^^ error: Unable to resolve constant `StringParam`
    optional :how_many, Opus::Param::CaseParam.new(self.how_many_cases, Opus::Param::ParamSpecsParam.new(Default)) # error: Method `optional` does not exist on `T.class_of(NotAODM)`
                      # ^^^^^^^^^^^ error: Unable to resolve constant `Param`
                                                 # ^^^^^^^^^^^^^^^^^^^ error: Method `how_many_cases` does not exist on `T.class_of(NotAODM)`
                                                                      # ^^^^^^^^^^^ error: Unable to resolve constant `Param`
                                                                                                       # ^^^^^^^ error: Unable to resolve constant `Default`
    optional :optional_param, IntegerParam.new # error: Method `optional` does not exist
                            # ^^^^^^^^^^^^ error: Unable to resolve constant `IntegerParam`
end

class SomeODM
    extend T::Sig

    prop :foo, String

    sig {returns(T.nilable(String))}
    def foo2; T.cast(T.unsafe(nil), T.nilable(String)); end
    sig {params(arg0: String).returns(String)}
    def foo2=(arg0); T.cast(nil, String); end
end

class ForeignClass
end

class AdvancedODM
    prop :default, String, default: ""
    prop :t_nilable, T.nilable(String)

    prop :type, type: String
    prop :object
    prop :array, Array
    prop :array_of, array: String
    prop :array_of_explicit, Array, array: String
    prop :t_array, T::Array[String]
    prop :hash_of, T::Hash[Symbol, String]

    prop :const_explicit, String, immutable: true
    const :const, String

    prop :no_class_arg, type: Array, immutable: true, array: String

    prop :enum_prop, enum: ["hello", "goodbye"]

    prop :foreign, String, foreign: ForeignClass
    prop :foreign_lazy, String, foreign: -> {ForeignClass}
    prop :foreign_proc, String, foreign: proc {ForeignClass}
    prop :foreign_invalid, String, foreign: proc { :not_a_type }

    prop :ifunset, String, ifunset: ''
    prop :ifunset_nilable, T.nilable(String), ifunset: ''
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

class EncryptedProp
  encrypted_prop :foo
  encrypted_prop :bar, migrating: true, immutable: true
end

def main
    T.reveal_type(SomeODM.new.foo) # error: Revealed type: `String`
    T.reveal_type(SomeODM.new.foo = 'b') # error: Revealed type: `String("b")`
    T.reveal_type(SomeODM.new.foo2) # error: Revealed type: `T.nilable(String)`
    T.reveal_type(SomeODM.new.foo2 = 'b') # error: Revealed type: `String("b")`

    T.reveal_type(AdvancedODM.new.default) # error: Revealed type: `String`
    T.reveal_type(AdvancedODM.new.t_nilable) # error: Revealed type: `T.nilable(String)`

    T.reveal_type(AdvancedODM.new.type) # error: Revealed type: `String`
    T.reveal_type(AdvancedODM.new.object) # error: Revealed type: `Object`
    T.reveal_type(AdvancedODM.new.array) # error: Revealed type: `T::Array[<any>]`
    T.reveal_type(AdvancedODM.new.array_of) # error: Revealed type: `T::Array[String]`
    T.reveal_type(AdvancedODM.new.array_of_explicit) # error: Revealed type: `T::Array[<any>]`
    T.reveal_type(AdvancedODM.new.t_array) # error: Revealed type: `T::Array[String]`
    T.reveal_type(AdvancedODM.new.hash_of) # error: Revealed type: `T::Hash[Symbol, String]`

    T.reveal_type(AdvancedODM.new.const_explicit) # error: Revealed type: `String`
    AdvancedODM.new.const_explicit = 'b' # error: Method `const_explicit=` does not exist on `AdvancedODM`
    T.reveal_type(AdvancedODM.new.const) # error: Revealed type: `String`
    AdvancedODM.new.const = 'b' # error: Method `const=` does not exist on `AdvancedODM`

    T.reveal_type(AdvancedODM.new.no_class_arg) # error: Revealed type: `T::Array[<any>]`
    AdvancedODM.new.no_class_arg = ['b'] # error: Method `no_class_arg=` does not exist on `AdvancedODM`

    T.reveal_type(AdvancedODM.new.enum_prop) # error: Revealed type: `T.untyped`
    AdvancedODM.new.enum_prop = "hello" # error: Method `enum_prop=` does not exist

    T.reveal_type(AdvancedODM.new.foreign_) # error: Revealed type: `T.nilable(ForeignClass)`
    T.reveal_type(AdvancedODM.new.foreign_!) # error: Revealed type: `ForeignClass`
    T.reveal_type(AdvancedODM.new.foreign_lazy_) # error: Revealed type: `T.nilable(ForeignClass)`

    # Check that the method still exists even if we can't parse the type
    AdvancedODM.new.foreign_invalid_

    T.reveal_type(PropHelpers.new.token) # error: Revealed type: `String`
    PropHelpers.new.token = "tok_token"
    PropHelpers.new.token = nil # error: does not match expected type

    T.reveal_type(PropHelpers.new.created) # error: Revealed type: `Float`
    PropHelpers.new.created = 0.0
    PropHelpers.new.created = nil # error: does not match expected type

    T.reveal_type(PropHelpers2.new.token) # error: Revealed type: `String`
    PropHelpers2.new.token = "tok_token"
    PropHelpers2.new.token = nil # error: does not match expected type

    T.reveal_type(PropHelpers2.new.created) # error: Revealed type: `Float`
    PropHelpers2.new.created = 0.0 # error: Method `created=` does not exist

    T.reveal_type(ShardingProp.new.merchant) # error: Revealed type: `String`
    ShardingProp.new.merchant = "hi" # error: Method `merchant=` does not exist

    T.reveal_type(EncryptedProp.new.foo) # error: Revealed type: `T.nilable(String)`
    T.reveal_type(EncryptedProp.new.encrypted_foo) # error: Revealed type: `T.nilable(Opus::DB::Model::Mixins::Encryptable::EncryptedValue)`
    EncryptedProp.new.foo = "hello"
    EncryptedProp.new.foo = nil
    EncryptedProp.new.bar = "hello" # error: Method `bar=` does not exist

    T.reveal_type(AdvancedODM.new.ifunset) # error: Revealed type: `String`
    T.reveal_type(AdvancedODM.new.ifunset_nilable) # error: Revealed type: `T.nilable(String)`
    AdvancedODM.new.ifunset = nil # error: does not match expected type
    AdvancedODM.new.ifunset_nilable = nil
end
