# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class BuilderSyntaxTest < Critic::Unit::UnitTest
    after do
      T::Private::DeclState.current.reset!
    end

    it 'allows params and return type' do
      # `sig` block is run in scope of decl, so it doesn't have assertion methods.
      # Hoist to assert afterwards.
      builder = nil
      mod = Module.new do
        extend T::Sig
        sig do
          builder = params(x: Integer).returns(String)
        end
        def self.fn(x); x.to_s; end
      end
      mod.fn(1) # executes the sig block

      assert(builder)
      assert_equal(mod, builder.decl.mod)
      assert_equal({x: Integer}, builder.decl.params)
      assert_equal(String, builder.decl.returns)
      assert_equal(:always, builder.decl.checked)
      assert_equal('standard', builder.decl.mode)
      assert_equal(true, builder.decl.finalized)
    end

    describe 'modes' do
      # @see T::Test::AbstractValidationTest
      # for error cases with abstract/override/implementation signatures

      it 'correctly constructs abstract/override/implementation modes' do
        # `sig` block is run in scope of decl, so it doesn't have assertion methods.
        # Hoist to assert afterwards.
        builders = {}

        Base = Class.new do
          extend T::Sig
          sig do
            builders[:abstract] = void.abstract
          end
          def implement_me; end

          sig do
            builders[:overridable] = void.overridable
          end
          def override_me; end
        end

        Child1 = Class.new(Base) do
          extend T::Sig
          sig do
            builders[:implementation] = void.implementation
          end
          def implement_me; end

          sig do
            builders[:override] = void.override
          end
          def override_me; end
        end

        Child2 = Class.new(Base) do
          extend T::Sig
          sig do
            builders[:implementation_overridable] = void.implementation.overridable
          end
          def implement_me; end
        end

        Child3 = Class.new(Base) do
          extend T::Sig
          sig do
            builders[:overridable_implementation] = void.overridable.implementation
          end
          def implement_me; end
        end

        Child1.new.implement_me
        assert_equal('abstract', builders[:abstract].decl.mode)
        assert_equal('implementation', builders[:implementation].decl.mode)

        Child1.new.override_me
        assert_equal('overridable', builders[:overridable].decl.mode)
        assert_equal('override', builders[:override].decl.mode)

        Child2.new.implement_me
        assert_equal('overridable_implementation', builders[:implementation_overridable].decl.mode)

        Child3.new.implement_me
        assert_equal('overridable_implementation', builders[:overridable_implementation].decl.mode)
      end

      INVALID_MODE_TESTS = [
        [:abstract, :abstract],
        [:abstract, :override],
        [:abstract, :overridable],
        [:abstract, :implementation],

        [:override, :abstract],
        [:override, :override],
        [:override, :overridable],
        [:override, :implementation],

        [:overridable, :abstract],
        [:overridable, :override],
        [:overridable, :overridable],

        [:implementation, :abstract],
        [:implementation, :override],
        [:implementation, :implementation],

        [:implementation, :overridable, :overridable],
        [:implementation, :overridable, :implementation],
      ]
      INVALID_MODE_TESTS.each do |seq|
        name = (["sig"] + seq).join(".")
        it name do
          cls = Class.new do
            extend T::Sig
            # abstract/overridable/etc only work on instance-level methods
            sig do
              builder = void
              seq.each {|method| builder.public_send(method)}
              builder
            end
            def fn; end
          end

          err = assert_raises(ArgumentError) do
            cls.new.fn
          end
          assert_match(/Error interpreting `sig`/, err.message)
        end
      end
    end

    describe 'declarations' do
      describe '.checked' do
        it 'raises RuntimeError with invalid level' do
          err = assert_raises(ArgumentError) do
            mod = Module.new do
              extend T::Sig
              sig {void.checked(true)}
              def self.test_method; end
            end
            mod.test_method
          end
          assert_match(/Invalid `checked` level/, err.message)

          assert_raises(ArgumentError) do
            mod = Module.new do
              extend T::Sig
              sig {void.checked(false)}
              def self.test_method; end
            end
            mod.test_method
          end

          assert_raises(ArgumentError) do
            mod = Module.new do
              extend T::Sig
              sig {void.checked(:foo)}
              def self.test_method; end
            end
            mod.test_method
          end

          builder = nil
          mod = Module.new do
            extend T::Sig
            sig do
              builder = void.checked(:always)
            end
            def self.test_method; end
          end
          mod.test_method
          assert_equal(:always, builder.decl.checked)
        end

        describe 'runtime levels' do
          before do
            @orig_check_tests = T::Private::RuntimeLevels.check_tests?
            @orig_default_checked_level = T::Private::RuntimeLevels.instance_variable_get(:@default_checked_level)
          end

          after do
            T::Private::RuntimeLevels._toggle_checking_tests(@orig_check_tests)
            T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, @orig_default_checked_level)
          end

          it '`always` is checked' do
            mod = Module.new do
              extend T::Sig
              sig do
                params({x: Integer})
                .returns(String)
                .checked(:always)
              end
              def self.test_method(x); end
            end

            assert_raises(TypeError) do
              mod.test_method(:llamas)
            end
          end

          it '`never` is not checked' do
            mod = Module.new do
              extend T::Sig
              sig do
                params({x: Integer})
                .returns(String)
                .checked(:never)
              end
              def self.test_method(x); end
            end

            mod.test_method(:llamas) # wrong, but ignored
          end

          def make_mod
            Module.new do
              extend T::Sig
              sig do
                params({x: Integer})
                .returns(String)
                .checked(:tests)
              end
              def self.test_method(x); end
            end
          end

          it '`tests` can be toggled to validate or not' do
            T::Private::RuntimeLevels._toggle_checking_tests(false)
            mod = make_mod
            mod.test_method(:llamas) # wrong, but ignored

            T::Private::RuntimeLevels._toggle_checking_tests(true)
            mod = make_mod
            assert_raises(TypeError) do
              mod.test_method(:llamas)
            end
          end

          it 'raises if `tests` is toggled on too late' do
            T::Private::RuntimeLevels._toggle_checking_tests(false)
            mod = make_mod
            mod.test_method(1) # invocation ensures it's wrapped

            err = assert_raises(RuntimeError) do
              T::Configuration.enable_checking_for_sigs_marked_checked_tests
            end
            assert_match(/Toggle `:tests`-level runtime type checking earlier/, err.message)
          end

          it 'override default checked level to :never' do
            T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :never)

            a = Module.new do
              extend T::Sig
              sig {params(x: Integer).void}
              def self.foo(x); end
            end

            a.foo('') # type error ignored

            pass
          end

          it 'override default checked level to :tests, without checking tests' do
            T::Private::RuntimeLevels._toggle_checking_tests(false)
            T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :tests)

            a = Module.new do
              extend T::Sig
              sig {params(x: Integer).void}
              def self.foo(x); end
            end

            a.foo('') # type error ignored

            T::Private::RuntimeLevels._toggle_checking_tests(true)
            pass
          end

          it 'override default checked level to :tests and also check tests' do
            T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :tests)
            T::Private::RuntimeLevels._toggle_checking_tests(true)

            a = Module.new do
              extend T::Sig
              sig {params(x: Integer).void}
              def self.foo(x); end
            end

            assert_raises(TypeError) do
              a.foo('')
            end
          end

          it 'override default checked level to :never but opt in with .checked(:always)' do
            skip
            T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :never)

            a = Module.new do
              extend T::Sig
              sig {params(x: Integer).void.checked(:always)}
              def self.foo(x); end
            end

            assert_raises(TypeError) do
              a.foo('')
            end
          end

          it 'setting the default checked level raises if set too late' do
            Module.new do
              extend T::Sig
              sig {void}
              def self.foo; end
              foo
            end

            err = assert_raises(RuntimeError) do
              T::Configuration.default_checked_level = :never
            end
            assert_match(/Set the default checked level earlier/, err.message)
          end
        end
      end

      it 'forbids multiple .returns calls' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {returns(Integer).returns(Integer)}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't call .returns multiple times in a signature.")
      end

      it 'forbids multiple .checked calls' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {returns(Integer).checked(:always).checked(:always)}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't call .checked multiple times in a signature.")
      end

      it 'forbids multiple .on_failure calls' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {returns(Integer).on_failure(notify: 'me').on_failure(notify: 'you')}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't call .on_failure multiple times in a signature.")
      end

      it 'forbids .on_failure and then .checked' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {returns(Integer).on_failure(notify: 'me').checked(:never)}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't use .checked with .on_failure.")
      end

      it 'forbids .checked and then .on_failure' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {returns(Integer).on_failure(notify: 'me').checked(:never)}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't use .checked with .on_failure.")
      end

      it 'forbids empty notify' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {returns(Integer).on_failure(notify: '')}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't provide an empty notify to .on_failure().")
      end

      it 'forbids unpassed notify' do
        ex = assert_raises(ArgumentError) do
          Class.new do
            extend T::Sig
            sig {returns(Integer).on_failure}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "missing keyword: notify")
      end

      it 'forbids .generated and then .checked' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {generated.returns(Integer).checked(:never)}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't use .checked with .generated.")
      end

      it 'forbids .generated and then .on_failure' do
        ex = assert_raises do
          Class.new do
            extend T::Sig
            sig {generated.returns(Integer).on_failure(notify: '')}
            def self.foo; end; foo
          end
        end
        assert_includes(ex.message, "You can't use .on_failure with .generated.")
      end

      it 'disallows return then void' do
        e = assert_raises(ArgumentError) do
          Class.new do
            extend T::Sig
            sig {returns(Integer).void}
            def self.foo; end; foo
          end
        end
        assert_includes(e.message, "You can't call .void after calling .returns.")
      end

      it 'disallows void then return' do
        e = assert_raises(ArgumentError) do
          Class.new do
            extend T::Sig
            sig {void.returns(Integer)} # rubocop:disable PrisonGuard/SigBuilderOrder
            def self.foo; end; foo
          end
        end
        assert_includes(e.message, "You can't call .returns after calling .void.")
      end
    end

    describe 'type_parameters' do
      it 'allows well formed decls' do
        mod = Module.new do
          extend T::Sig
          sig do
            type_parameters(:U)
            .params(
              blk: T.proc.params(arg0: Integer).returns(T.type_parameter(:U)),
            )
            .returns(T::Array[T.type_parameter(:U)])
          end
          def self.map(&blk)
            [1].map(&blk)
          end
        end
        assert_equal(["hi"], mod.map {|_x| "hi"})
      end

      it 'disallows non-symbols in type_parameter' do
        e = assert_raises(ArgumentError) do
          T.type_parameter(3)
        end
        assert_includes(e.message, "not a symbol: 3")
      end

      it 'disallows non-symbols in type_parameters' do
        e = assert_raises(ArgumentError) do
          Class.new do
            extend T::Sig
            sig {type_parameters(3)}
            def self.foo; end; foo
          end
        end
        assert_includes(e.message, "not a symbol: 3")
      end
    end

    it 'includes line numbers in errors' do
      line = nil
      klass = Class.new do
        extend T::Sig
        line = __LINE__; sig {params(x: Integer)}
        def f(x); end
      end

      e = assert_raises(ArgumentError) do
        klass.new.f
      end

      root = Pathname.new(__dir__).realpath
      path = Pathname.new(__FILE__).realpath.relative_path_from(root).to_s

      assert_includes(e.message, "You must provide a return type")
      assert_includes(e.message, "#{path}:#{line}: Error interpreting `sig`:")
    end
  end
end
