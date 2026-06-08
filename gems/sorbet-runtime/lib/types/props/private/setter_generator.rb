# frozen_string_literal: true
# typed: false

module T::Props
  module Private

    # Compiles prop setters into plain `def`s on first call.
    #
    # The interpreted setters built by `SetterFactory` are `define_method`
    # bmethods over procs that close over the prop's type, name, etc. Those
    # are several times slower than a plain `def` with an inlined `is_a?`
    # check and a literal `@ivar` write (bmethod dispatch is also opaque to
    # YJIT). For eligible props we instead install a tiny stub that, on first
    # call, `class_eval`s a specialized plain-`def` setter and then gets out
    # of the way; all subsequent calls run the compiled definition directly.
    #
    # The prop's type (and the other objects the compiled body needs) are
    # never emitted as constant literals -- the type may be a private
    # constant, an anonymous module, or have no stable global path. Instead
    # they are stashed in a private constant container (a frozen Array) on
    # the OWNER class, which the `class_eval`'d source resolves lexically.
    #
    # The interpreted `setter_proc` remains in the rules hash untouched and
    # is still used by every other consumer (constructors, `prop_set`,
    # `validate_prop_value`, defaults) and by every fallback path here:
    # ineligible props, frozen classes, trap contexts, compile failures,
    # losers of first-call races, instances of `dup`/`clone` copies of the
    # class, and deployments that have called
    # `T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!`.
    module SetterGenerator

      # Pseudo-path prefix for compiled definitions and the source path of
      # the lazy stubs. Built from this file's __FILE__ so that generated
      # frames keep exactly the same caller_locations filtering behavior as
      # the interpreted procs in setter_factory.rb (which live in the same
      # directory): SetterFactory.raise_pretty_error attributes errors to the
      # first frame outside this gem's props directory, and tests assert the
      # resulting "Caller:" line points at user code.
      GENERATOR_FILE = __FILE__
      STUB_DEF_PATH = __FILE__

      # Method names we may emit must be plain lowercase identifiers. This is
      # the injection guard for `class_eval`: the only user-controlled bytes
      # that reach generated source are names matching this allowlist (plus
      # `Integer#to_s` generation counters).
      IDENT_RE = /\A[a-z_][a-zA-Z0-9_]*\z/
      private_constant :IDENT_RE

      # Reserved words and pseudo-variables are excluded even though many of
      # them parse fine in `def <name>=` position: the denylist is shared
      # with the other compiled-dispatch tiers, and correctness over coverage
      # is the rule for anything unusual. (`__END__` additionally truncates
      # eval'd source when it appears at column 0.)
      RESERVED_IDENTS = %w[
        __ENCODING__ __LINE__ __FILE__ __END__ BEGIN END alias and begin
        break case class def defined? do else elsif end ensure false for if
        in module next nil not or redo rescue retry return self super then
        true undef unless until when while yield __method__ __dir__
        __callee__ binding
      ].to_h { |w| [w, true] }.freeze
      private_constant :RESERVED_IDENTS

      @enabled = true
      @warned = {}

      class << self
        def enabled?
          @enabled
        end

        # Kill switch. After this returns, props defined later get the
        # interpreted setter_proc, and already-installed stubs permanently
        # fall back to the interpreted setter on their first call instead of
        # compiling (eventual quiescence: a compile already in flight on
        # another thread may still complete).
        def disable!
          @enabled = false
        end

        def safe_ident?(str)
          IDENT_RE.match?(str) &&
            !RESERVED_IDENTS.key?(str) &&
            !str.start_with?("__t_", "__sorbet")
        end

        # A prop is eligible for compilation when every byte we would emit is
        # provably safe and the emitted body would be exactly equivalent to
        # the interpreted setter_proc. Anything else falls back.
        #
        # Runs for every prop definition, so it is written allocation-free
        # (Symbol#name returns the interned frozen string).
        def eligible?(mod, prop, rules)
          name = prop.name
          ak = rules[:accessor_key]
          safe_ident?(name) &&
            # We only emit `@<prop> = val`, so the accessor key must be
            # exactly :@<prop> (it always is today; guard against overrides).
            ak.is_a?(Symbol) &&
            (ak_name = ak.name).length == name.length + 1 &&
            ak_name.start_with?("@") &&
            ak_name.end_with?(name) &&
            !mod.frozen?
        end

        # Resolve `mod`'s own method-table entry for `name`, walking past any
        # prepended modules. All "is the live definition our stub?" questions
        # must be answered against the own slot, never against a bare
        # `instance_method` (which resolves through the full MRO and would
        # conflate "shadowed by a prepend" with "not ours").
        def own_slot_method(mod, name)
          meth = begin
            mod.instance_method(name)
          rescue NameError
            return nil
          end
          while meth && !meth.owner.equal?(mod)
            meth = meth.super_method
          end
          meth
        end

        def stub_method?(meth)
          !meth.nil? && meth.source_location&.first == STUB_DEF_PATH
        end

        # Builds the constant container and `class_eval`s the compiled setter
        # onto `cls`. Raises on any unexpected state (e.g. a foreign constant
        # already occupying our reserved name); the caller converts any raise
        # into a permanent interpreted fallback.
        def define_compiled_setter!(cls, prop, rules, gen)
          # Container names never repeat ((prop, gen) -> name is injective:
          # prop names match IDENT_RE and the digits-only generation suffix
          # cannot contain "_"), so a pre-existing constant is foreign.
          # Never clobber it.
          container_name = container_const_name(prop, gen)
          if cls.const_defined?(container_name, false)
            raise "constant #{container_name} already defined on #{cls}"
          end

          non_nil_type = T::Utils::Nilable.get_underlying_type_object(rules.fetch(:type_object))
          validate = rules[:setter_validate]

          # Mirrors the shape routing in SetterFactory.build_setter_proc.
          has_explicit_nil_default = rules.key?(:default) && rules.fetch(:default).nil?
          nilable = !T::Props::Utils.need_nil_write_check?(rules) || has_explicit_nil_default
          simple = validate.nil? && non_nil_type.is_a?(T::Types::Simple)

          # The container is a frozen Array, NOT a Module with nested
          # constants: `const_set` permanently names any anonymous module
          # assigned under it (the decorated class in the klass slot, or an
          # anonymous prop type) -- a user-visible behavior change. Array
          # slots carry arbitrary objects without side effects, and an
          # inline-cached constant read plus Array#[] is still cheap.
          # Slots: [0] check target (raw module for Simple shapes, type
          # object otherwise), [1] defining class (error reporting),
          # [2] setter_validate proc (when present).
          container = [simple ? non_nil_type.raw_type : non_nil_type, cls, validate].freeze

          source = generate_setter_source(
            container_name, prop, nilable: nilable, simple: simple, validate: !validate.nil?
          )

          cls.const_set(container_name, container)
          cls.send(:private_constant, container_name)
          cls.class_eval(source, eval_path(prop, gen), 1)
          nil
        end

        def container_const_name(prop, gen)
          :"SORBET_RT_PROPS_#{prop}_G#{gen}"
        end

        def eval_path(prop, gen)
          "#{GENERATOR_FILE}(compiled:#{prop}=:g#{gen})"
        end

        def warn_compile_fallback(cls, method_name, err)
          key = "#{cls.object_id}##{method_name}"
          return if @warned[key]

          @warned[key] = true
          T::Configuration.log_info_handler(
            "T::Props::Private::SetterGenerator: falling back to the interpreted setter " \
            "for #{cls}##{method_name}: #{err.class}: #{err.message}",
            {},
          )
        end

        # The emitted bodies transcribe the four interpreted setter shapes
        # from SetterFactory exactly:
        #
        # * `recursively_valid?` (never `valid?`) for non-Simple types.
        # * The error path calls SetterFactory.raise_pretty_error and then
        #   STILL writes the ivar, preserving the set-after-soft-error quirk
        #   under a non-raising call_validation_error_handler, and reporting
        #   against the DEFINING class (container slot [1], not self.class).
        # * The Simple-shape error arm coerces the raw module at error time,
        #   exactly like simple_non_nil_proc/simple_nilable_proc.
        # * `setter_validate` runs only inside the type-check success branch,
        #   never for nil, and is called with exactly (prop, value).
        # * Return values match the interpreted procs (the ivar write is the
        #   final expression; the nil arm returns nil).
        private def generate_setter_source(container_name, prop, nilable:, simple:, validate:)
          c = container_name
          validate_line = validate ? "#{c}[2].call(:#{prop}, val)" : nil

          body =
            if simple
              # validate is nil by construction for Simple shapes (mirroring
              # SetterFactory's routing precondition).
              nil_check = nilable ? "val.nil? || " : ""
              <<~RUBY
                def #{prop}=(val)
                  unless #{nil_check}val.is_a?(#{c}[0])
                    ::T::Props::Private::SetterFactory.raise_pretty_error(#{c}[1], :#{prop}, ::T::Utils.coerce(#{c}[0]), val)
                  end
                  @#{prop} = val
                end
              RUBY
            elsif nilable
              <<~RUBY
                def #{prop}=(val)
                  if val.nil?
                    @#{prop} = nil
                  elsif #{c}[0].recursively_valid?(val)
                    #{validate_line}
                    @#{prop} = val
                  else
                    ::T::Props::Private::SetterFactory.raise_pretty_error(#{c}[1], :#{prop}, #{c}[0], val)
                    @#{prop} = val
                  end
                end
              RUBY
            elsif validate
              <<~RUBY
                def #{prop}=(val)
                  if #{c}[0].recursively_valid?(val)
                    #{validate_line}
                  else
                    ::T::Props::Private::SetterFactory.raise_pretty_error(#{c}[1], :#{prop}, #{c}[0], val)
                  end
                  @#{prop} = val
                end
              RUBY
            else
              <<~RUBY
                def #{prop}=(val)
                  unless #{c}[0].recursively_valid?(val)
                    ::T::Props::Private::SetterFactory.raise_pretty_error(#{c}[1], :#{prop}, #{c}[0], val)
                  end
                  @#{prop} = val
                end
              RUBY
            end

          "# frozen_string_literal: true\n#{body}"
        end
      end

      # Mixed into T::Props::Decorator. Owns the per-decorator specialization
      # queue: a stub is installed at prop-definition time, and the first call
      # claims the queue entry (under @specialization_lock), compiles outside
      # the lock, publishes the compiled def, and dequeues. Every queue
      # operation is a tiny critical section containing only Hash/flag ops --
      # no user-observable work (no class_eval, no method_added hooks, no
      # visibility sends) ever runs while the lock is held.
      #
      # Convergence invariants (each pinned by tests):
      #
      # * Every method definition that happens at FIRST-CALL time (compiled
      #   def, interpreted fallback, stub reinstalls) runs inside
      #   `T::Private::DeclState.current.without_on_method_added`: a sig
      #   declaration pending on the calling thread must neither bind to our
      #   internal definitions nor be dropped. Definitions made at
      #   prop-definition time (the initial stub) deliberately fire
      #   `method_added` exactly like the interpreted `define_method` they
      #   replace.
      # * A claim is only taken while the live own-slot definition is still
      #   our stub. Stale pre-call handles to the stub (case file [A]) thus
      #   never clobber a manual redefinition or resurrect a removed setter;
      #   they run the interpreted setter with their own generation's
      #   semantics, like stale handles to the interpreted bmethods at base.
      # * A claimant that discovers at the final sync that its generation was
      #   superseded AND already consumed re-enqueues a fresh entry from the
      #   prop's current rules and reinstalls a stub, so a stale-generation
      #   publish can never permanently enforce an old prop type.
      # * The stub NEVER raises on torn states: if dispatch finds the own
      #   slot is (still) a stub or empty, or the receiver is an instance of
      #   a `dup`/`clone` copy of the class, it degrades to the interpreted
      #   setter (re-arming compilation where that can help).
      module DecoratorMethods
        # state is :pending until a thread claims the entry (:compiling).
        SpecEntry = Struct.new(:prop, :rules, :gen, :setter_proc, :state)

        # Called from `define_getter_and_setter`'s fast path. Returns true if
        # a lazily-compiled setter stub was installed; false means the caller
        # should define the interpreted setter_proc as before.
        private def enqueue_compiled_setter!(name, rules)
          return false unless SetterGenerator.enabled?
          return false unless SetterGenerator.eligible?(@class, name, rules)

          setter_name = :"#{name}="
          entry = nil
          begin
            # The generation counter is allocated under the same lock as the
            # queue insert: first-call repair paths also allocate generations
            # concurrently, and (prop, gen) pairs must never repeat (the
            # compiled containers' constant names are derived from them).
            @specialization_lock.synchronize do
              gens = (@specialization_gens ||= {})
              gen = gens[name] = (gens[name] || 0) + 1
              entry = SpecEntry.new(name, rules, gen, rules.fetch(:setter_proc), :pending)
              @specialized_methods[setter_name] = entry
            end
          rescue ThreadError
            # Trap context: don't install machinery we can't coordinate.
            return false
          end
          install_specialization_stub!(setter_name, entry)
          true
        end

        private def install_specialization_stub!(method_name, entry)
          decorator = self
          cls = @class
          setter_proc = entry.setter_proc
          prop = entry.prop
          T::Configuration.without_ruby_warnings do
            cls.send(:define_method, method_name) do |val|
              status = decorator.send(:specialize_lazy_setter!, method_name)
              if status == :interpreted
                # This call frame owns the receiver and argument, so the
                # interpreted fallback runs here (and its value is returned).
                instance_exec(val, &setter_proc)
              else
                # :published or :gone -- some real definition should be live
                # now. Dispatch directly to the own-slot definition (not via
                # __send__, which would restart the MRO: it would re-run any
                # prepended wrapper that already ran before reaching us, and
                # on a dup/clone of the class it would recurse into this very
                # stub).
                meth = T::Props::Private::SetterGenerator.own_slot_method(cls, method_name)
                if !meth.nil? && !T::Props::Private::SetterGenerator.stub_method?(meth) && is_a?(cls)
                  meth.bind_call(self, val)
                else
                  # Defensive convergence arm -- never raise from a user's
                  # setter call. Reachable when:
                  # * `self` is an instance of a dup/clone of `cls` (whose
                  #   copied method table contains this stub forever):
                  #   bind_call would raise TypeError, so run the
                  #   self-contained interpreted setter instead.
                  # * the live own-slot definition is (still) a stub: a stub
                  #   was reinstalled over a published definition by a racing
                  #   prop redefinition. Run this call interpreted with our
                  #   own generation's semantics and re-arm compilation so a
                  #   later call can compile the prop's current rules.
                  # * the own slot is empty (e.g. removed on `cls` while a
                  #   stale handle to this stub survives): interpreted, like
                  #   a stale handle to an interpreted bmethod at base.
                  if T::Props::Private::SetterGenerator.stub_method?(meth)
                    decorator.send(:reenqueue_compiled_setter!, prop, method_name)
                  end
                  instance_exec(val, &setter_proc)
                end
              end
            end
          end
        end

        # Re-arm compilation for a setter whose live definition is a stub but
        # whose queue entry was consumed (a torn outcome of racing prop
        # redefinitions; see the stub's convergence arm). Builds a fresh
        # entry from the prop's CURRENT rules under a fresh generation.
        # No-op if an entry is already queued, the rules are gone or
        # ineligible, or the lock cannot be taken.
        private def reenqueue_compiled_setter!(prop, setter_name)
          return unless SetterGenerator.enabled?

          rules = @props[prop]
          return if rules.nil? || !SetterGenerator.eligible?(@class, prop, rules)

          lock = @specialization_lock
          return if lock.owned?

          begin
            lock.synchronize do
              return if @specialized_methods[setter_name]

              gens = (@specialization_gens ||= {})
              gen = gens[prop] = (gens[prop] || 0) + 1
              @specialized_methods[setter_name] =
                SpecEntry.new(prop, rules, gen, rules.fetch(:setter_proc), :pending)
            end
          rescue ThreadError
            nil
          end
        end

        # Compile (or permanently fall back) the queued setter. Returns:
        #   :published   -- a real definition (compiled, or the interpreted
        #                   fallback bmethod) is installed under method_name
        #   :gone        -- another thread already published and dequeued
        #   :interpreted -- the caller must run the interpreted setter_proc
        #                   itself (frozen class, trap context, compile in
        #                   flight on another thread, lock re-entrancy, a
        #                   stale handle whose own slot is no longer our
        #                   stub, or a superseded generation)
        private def specialize_lazy_setter!(method_name)
          lock = @specialization_lock
          # Re-entrancy guard: e.g. a signal trap interrupting our own
          # critical section. Never touch the lock on the owning thread.
          return :interpreted if lock.owned?

          claimed = nil
          begin
            lock.synchronize do
              entry = @specialized_methods[method_name]
              return :gone if entry.nil?
              return :interpreted if entry.state == :compiling
              # Don't consume the entry for a frozen class: nothing can be
              # defined, so every call takes this interpreted arm forever.
              return :interpreted if @class.frozen?
              # Stale-handle guard (case file [A]): only compile while the
              # live own-slot definition is still our stub. If user code
              # redefined or removed the setter after prop definition, a
              # surviving pre-call handle to the stub must neither clobber
              # the user's definition nor resurrect a removed one: run
              # interpreted (the stale handle keeps its own generation's
              # semantics, exactly like a stale handle to an interpreted
              # bmethod at base) and leave the entry alone (a later prop
              # redefinition through the DSL replaces it; the eager drain
              # drops it).
              unless SetterGenerator.stub_method?(SetterGenerator.own_slot_method(@class, method_name))
                return :interpreted
              end
              entry.state = :compiling
              claimed = entry
            end
          rescue ThreadError
            # Mutex#synchronize raises in trap context even when the lock is
            # free. Run this call interpreted; a later non-trap call compiles.
            return :interpreted
          end

          # ---- All user-observable work happens outside the lock. ----

          # Re-read the effective visibility at publish time (the stub is
          # installed public, but user code may have privatized it between
          # prop definition and first call). Read BEFORE class_eval, which
          # would reset the name to public.
          visibility = begin
            T::Private::ClassUtils.visibility_method_name(@class, method_name)
          rescue NameError
            :public
          end

          # Definitions made at first-call time must not fire the sig
          # machinery's method_added hook: a sig declaration pending on this
          # thread (e.g. the first call happens inside a class body, between
          # a `sig` and its `def`) must neither bind to our published setter
          # nor be dropped by a cross-module "method added on a different
          # class/module" error.
          decl_state = T::Private::DeclState.current

          begin
            if SetterGenerator.enabled? &&
               (T::Props::HasLazilySpecializedMethods.lazy_evaluation_enabled? || Thread.current[:__t_props_force_eval])
              decl_state.without_on_method_added do
                T::Configuration.without_ruby_warnings do
                  SetterGenerator.define_compiled_setter!(@class, claimed.prop, claimed.rules, claimed.gen)
                end
              end
            else
              # Source evaluation is disabled (go/M8yrvzX2 hardening) or the
              # generator was disabled: permanently install the interpreted
              # setter_proc -- today's exact behavior and performance.
              decl_state.without_on_method_added do
                T::Configuration.without_ruby_warnings do
                  @class.send(:define_method, method_name, &claimed.setter_proc)
                end
              end
            end
            @class.send(visibility, method_name)
          rescue ScriptError, StandardError => err
            if @class.frozen?
              # The class froze between claim and publish: un-claim so future
              # calls take the in-lock frozen arm, and run interpreted.
              begin
                lock.synchronize { claimed.state = :pending }
              rescue ThreadError
                nil
              end
              return :interpreted
            end
            begin
              decl_state.without_on_method_added do
                T::Configuration.without_ruby_warnings do
                  @class.send(:define_method, method_name, &claimed.setter_proc)
                end
              end
              @class.send(visibility, method_name)
            rescue ScriptError, StandardError
              begin
                lock.synchronize { claimed.state = :pending }
              rescue ThreadError
                nil
              end
              return :interpreted
            end
            SetterGenerator.warn_compile_fallback(@class, method_name, err)
          end

          reinstall = nil
          begin
            lock.synchronize do
              current = @specialized_methods[method_name]
              if current.equal?(claimed)
                # Publish first, dequeue last; and only dequeue our own entry.
                @specialized_methods.delete(method_name)
              elsif current
                # A prop redefinition re-enqueued while we were compiling; our
                # published definition may have clobbered its stub. Reinstall
                # the new stub (outside the lock) so the redefinition wins.
                reinstall = current
              else
                # Our entry is gone but we did not dequeue it: a prop
                # redefinition REPLACED it while we were compiling, and the
                # successor entry has already been claimed, published, and
                # dequeued by another call. Our publish was therefore STALE
                # and just clobbered the successor's definition -- left
                # alone, the old prop type would be enforced forever.
                # Re-enqueue a fresh entry built from the prop's CURRENT
                # rules under a fresh generation, and reinstall a stub so a
                # later call compiles the latest semantics.
                latest_gen = (gens = @specialization_gens) && gens[claimed.prop]
                if latest_gen && latest_gen != claimed.gen
                  rules = @props[claimed.prop]
                  if rules && SetterGenerator.eligible?(@class, claimed.prop, rules)
                    gen = gens[claimed.prop] = latest_gen + 1
                    reinstall =
                      SpecEntry.new(claimed.prop, rules, gen, rules.fetch(:setter_proc), :pending)
                    @specialized_methods[method_name] = reinstall
                  end
                end
              end
            end
          rescue ThreadError
            # Trap fired exactly around the final dequeue: the method is
            # already published so behavior is correct; the entry leaks in
            # :compiling state (benign, bounded).
          end
          if reinstall
            begin
              decl_state.without_on_method_added do
                install_specialization_stub!(method_name, reinstall)
              end
            rescue ScriptError, StandardError
              # e.g. the class froze concurrently: nothing can be installed,
              # so leave whatever is live; nothing may escape the user's call.
              nil
            end
            return :interpreted
          end
          :published
        end

        # Eagerly compile every queued specialized setter. This is an
        # explicit opt-in hook (the props analogue of the sig tier's
        # compile-pending hook): it is deliberately NOT wired into
        # `eagerly_define_lazy_methods!`, because compiling every setter at
        # boot costs real time per prop, and deployments that don't want
        # post-boot eval but do want compiled setters should opt in:
        #
        #   decorator.eagerly_define_lazy_methods!
        #   decorator.eagerly_specialize_prop_methods!
        #   T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
        #
        # Without the opt-in, first calls after `disable_lazy_evaluation!`
        # permanently install the interpreted setter (today's behavior and
        # performance; never an error, unlike the serde lazy methods).
        # Like `eagerly_define_lazy_methods!`, this works even after
        # `disable_lazy_evaluation!` (same documented exemption: explicit
        # operator action over inputs in version control).
        def eagerly_specialize_prop_methods!
          names = begin
            @specialization_lock.synchronize { @specialized_methods.keys }
          rescue ThreadError
            return
          end
          names.each do |method_name|
            # Only compile entries whose lazy stub is still the live own-slot
            # definition. If user code redefined or removed the method after
            # prop definition, drop the entry rather than resurrect or
            # clobber the user's definition.
            meth = SetterGenerator.own_slot_method(@class, method_name)
            unless SetterGenerator.stub_method?(meth)
              begin
                @specialization_lock.synchronize do
                  current = @specialized_methods[method_name]
                  if current && current.state != :compiling
                    @specialized_methods.delete(method_name)
                  end
                end
              rescue ThreadError
                nil
              end
              next
            end
            begin
              Thread.current[:__t_props_force_eval] = true
              specialize_lazy_setter!(method_name)
            ensure
              Thread.current[:__t_props_force_eval] = nil
            end
          end
        end
      end
    end
  end
end

class T::Props::Decorator
  include T::Props::Private::SetterGenerator::DecoratorMethods
end
