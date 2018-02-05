# @typed
class NotAODM
    def self.prop(*args); end
    prop
    prop :foo, :not_a_string
    prop "not_a_symbol", String
    prop :foo, String, "not_a_hash"
    prop "too", String, {}, "many"
    optional :company_name, String, :nonempty_string # error: Method optional does not exist
    optional :day, IntegerParam.new(min: 1, max: 31) # error: MULTI
    optional :how_many, Opus::Param::CaseParam.new(self.how_many_cases, Opus::Param::ParamSpecsParam.new(Default)) # error: MULTI
    optional :optional_param, IntegerParam.new # error: Method optional does not exist
end

class SomeODM
    prop :foo, String

    sig.returns(T.nilable(String))
    def foo2; T.cast(nil, T.nilable(String)); end
    sig(arg0: String).returns(NilClass)
    def foo2=(arg0); end
end

class AdvancedODM
    prop :default, String, default: ""
    prop :nodefault, String
    prop :factory, String, factory: -> {""}

    prop :type, type: String, default: ""
    prop :object
    prop :array, Array, default: []
    prop :array_of, array: String, default: ""
    prop :array_of_explicit, Array, array: String, default: [""]
    prop :t_array, T::Array[String], default: [""]
    prop :hash_of, T::Hash[Symbol, String], default: {a: ""}

    prop :optional_explicit, String, optional: true, default: ""
    prop :optional_existing, String, optional: :existing, default: ""
    prop :optional_false, String, optional: false, default: ""
    optional :optional, String, default: ""

    prop :const_explicit, String, immutable: true, default: ""
    const :const, String, default: ""

    prop :no_class_arg, type: Array, immutable: true, optional: true, array: String, default: ""
end

def main
    T.assert_type!(SomeODM.new.foo, T.nilable(String))
    T.assert_type!(SomeODM.new.foo = 'b', NilClass)
    T.assert_type!(SomeODM.new.foo2, T.nilable(String))
    T.assert_type!(SomeODM.new.foo2 = 'b', NilClass)

    T.assert_type!(AdvancedODM.new.default, String)
    T.assert_type!(AdvancedODM.new.nodefault, T.nilable(String))
    T.assert_type!(AdvancedODM.new.factory, String)

    T.assert_type!(AdvancedODM.new.type, String)
    T.assert_type!(AdvancedODM.new.object, T.nilable(Object))
    T.assert_type!(AdvancedODM.new.array, Array)
    T.assert_type!(AdvancedODM.new.array_of, T::Array[String])
    T.assert_type!(AdvancedODM.new.array_of_explicit, T::Array[String])
    T.assert_type!(AdvancedODM.new.t_array, T::Array[String])
    T.assert_type!(AdvancedODM.new.hash_of, T::Hash[Symbol, String])

    T.assert_type!(AdvancedODM.new.optional_explicit, T.nilable(String))
    T.assert_type!(AdvancedODM.new.optional_existing, T.nilable(String))
    T.assert_type!(AdvancedODM.new.optional_false, String)
    T.assert_type!(AdvancedODM.new.optional, T.nilable(String))
    AdvancedODM.new.optional = 'b'
    AdvancedODM.new.optional = nil

    T.assert_type!(AdvancedODM.new.const_explicit, String)
    AdvancedODM.new.const_explicit = 'b' # error: Method const_explicit= does not exist on AdvancedODM
    T.assert_type!(AdvancedODM.new.const, String)
    AdvancedODM.new.const = 'b' # error: Method const= does not exist on AdvancedODM

    T.assert_type!(AdvancedODM.new.no_class_arg, T.nilable(T::Array[String]))
    AdvancedODM.new.no_class_arg = ['b'] # error: Method no_class_arg= does not exist on AdvancedODM
end
