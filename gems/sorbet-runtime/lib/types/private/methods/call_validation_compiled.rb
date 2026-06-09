# frozen_string_literal: true
# typed: true

# Source-compiled sig dispatch for fixed-arity positional methods.
#
# Instead of installing a `define_method` wrapper (a bmethod) that re-dispatches
# to the original method via the (slow) `UnboundMethod#bind_call`, eligible
# methods get a plain `def` wrapper compiled from a source string:
#
#   def foo(arg0, &)
#     unless arg0.is_a?(SORBET_RT_SIG_foo_G1_Nxx::A0) ... end
#     return_value = __t_sig_orig_<owner_id>_g1_nxx_foo(arg0, &)
#     unless return_value.is_a?(SORBET_RT_SIG_foo_G1_Nxx::R) ... end
#     return_value
#   end
#
# The wrapper ALWAYS declares and forwards a block -- there is deliberately no
# "does the original use its block?" analysis (no static analysis of a Ruby
# method body is sound against aliasing: `alias_method :bg, :block_given?`
# can be created even after compile). On Ruby >= 3.1 the block is forwarded
# anonymously (`&`, no Proc materialization); on 3.0 a named `&__t_blk` is
# forwarded, which is exactly the capture the bind_call families do today.
#
# The original method is stashed under a private, deterministic-per-install
# alias (a real method entry sharing the original's iseq, so `super`,
# `__method__`, and definition introspection keep working), and the type/sig
# objects the wrapper needs are stored in a private constant ("container") on
# the owner so the eval'd body resolves them through the lexical scope with
# inline-cached constant reads. Plain defs with inlined `is_a?` checks are
# also YJIT-friendly, where bmethods and bind_call are not.
#
# Anything that does not provably fit (kwargs, rest, optional positionals,
# required blocks, `.bind`, unsafe method names, refinements, methods whose
# owner is not the wrap target, frozen owners, sig types whose raw module has
# no permanent name -- binding an anonymous module in the container constant
# would permanently rename it) falls back to the existing `define_method`
# validator families, which remain intact. Compilation failures of any kind
# also fall back; the compiled path is strictly an optimization.
#
# Eval policy: compiling honors the same post-boot eval boundary as
# `T::Props::HasLazilySpecializedMethods` (see go/M8yrvzX2). After
# `disable_lazy_evaluation!`, no new wrapper source is eval'd: wraps install
# the family validators instead. Deployments that want compiled dispatch in a
# hardened configuration can call `T::Utils.compile_pending_sig_wrappers!`
# (the analogue of `eagerly_define_lazy_methods!`) before disabling.
module T::Private::Methods::CallValidation::Compiled
  CallValidation = T::Private::Methods::CallValidation

  # All wrapper pseudo-paths start with this prefix; `__FILE__` keeps them
  # inside the sorbet-runtime lib directory so caller-location filters (e.g.
  # `T::Private::CallerUtils.find_caller`) skip wrapper frames like they skip
  # every other sorbet-runtime frame.
  WRAPPER_PATH_PREFIX = "#{__FILE__}(compiled:"
  SHIM_DEF_PATH = __FILE__

  # Reserved words and pseudo-variables. These all *parse* as plain method
  # names in `def` position (verified: `def if(x)`, `def end(x)`, `def
  # self.true` are legal Ruby), but we conservatively refuse to compile
  # methods with these names rather than reason about every position the name
  # could appear in emitted source. Such methods take the family validators.
  # NOTE: this list is load-bearing for safety, not redundant with the regex
  # below: keyword-named methods match the identifier regex just fine.
  RESERVED_WORDS = %w[
    __ENCODING__ __LINE__ __FILE__ __END__ BEGIN END alias and begin break
    case class def defined? do else elsif end ensure false for if in module
    next nil not or redo rescue retry return self super then true undef
    unless until when while yield __method__ __dir__ __callee__ binding
  ].to_h { |w| [w, true] }.freeze
  private_constant(:RESERVED_WORDS)

  # Method names: a plain identifier with at most one trailing `?`, `!`, `=`.
  METHOD_NAME_RE = /\A[a-zA-Z_][a-zA-Z0-9_]*[?!=]?\z/
  private_constant(:METHOD_NAME_RE)

  # Whether to emit anonymous block forwarding (`def m(arg0, &)` forwarding
  # via `stash(arg0, &)`). The syntax parses only on Ruby >= 3.1, so the gate
  # is decided at codegen time and the 3.1+ syntax only ever appears inside
  # eval'd strings -- this file itself stays 3.0-parseable. Anonymous
  # forwarding never materializes a Proc; the 3.0 arm forwards a named
  # `&__t_blk` instead, which is the same named capture the bind_call
  # families do today (cost parity, never worse).
  ANONYMOUS_BLOCK_FORWARDING = (RUBY_VERSION.split(".").first(2).map(&:to_i) <=> [3, 1]) >= 0
  private_constant(:ANONYMOUS_BLOCK_FORWARDING)

  # `Module#name`, read unbound so a user override of `name` can neither mask
  # nor crash the anonymous-module guard below.
  UNBOUND_MODULE_NAME = Module.instance_method(:name)
  private_constant(:UNBOUND_MODULE_NAME)

  # `Module#to_s`, read unbound for the refinement-module detection fallback
  # (hoisted: `Module.instance_method(:to_s)` allocates an UnboundMethod per
  # call, and this runs on every eligible-shaped method wrap).
  UNBOUND_MODULE_TO_S = Module.instance_method(:to_s)
  private_constant(:UNBOUND_MODULE_TO_S)

  # The core `::Refinement` class (3.2+), captured at load time so a later
  # shadowing of the constant cannot confuse the check. On >= 3.2 every
  # refinement module is an instance of this core class, so the check is
  # complete there and the (String-allocating) `Module#to_s` marker fallback
  # is only consulted on the 3.0/3.1 floor, where `::Refinement` does not
  # exist (or names an unrelated user constant).
  NATIVE_REFINEMENT_CLASS = defined?(::Refinement) ? ::Refinement : nil
  REFINEMENT_CLASS_IS_COMPLETE =
    !NATIVE_REFINEMENT_CLASS.nil? && (RUBY_VERSION.split(".").first(2).map(&:to_i) <=> [3, 2]) >= 0
  private_constant(:NATIVE_REFINEMENT_CLASS, :REFINEMENT_CLASS_IS_COMPLETE)

  # A module name that is a valid (ASCII) constant path. This is the guard
  # against renaming user modules: `const_set` assigns (or re-derives) a name
  # for any module that does not already have a *permanent* one -- verified
  # on 3.3, `anything.const_set(:A0, Module.new)` immediately renames the
  # value, even under an anonymous parent (to `#<Module:0x...>::A0`), and
  # naming the parent later renames it again. A permanent name is always a
  # valid constant path, and nothing else can be one:
  # `Module#set_temporary_name` rejects constant paths outright ("must not
  # be a constant path"), and the pseudo-names derived under anonymous or
  # temporarily-named parents (`#<Module:0x...>::X`, `temp name::X`) contain
  # segments that are not constants. So: name matches => binding the module
  # in a wrapper container cannot rename it. Anything else (nil, temporary,
  # pseudo, and -- conservatively -- exotic non-ASCII constant names) must
  # never be bound: the rename would be a permanent, user-visible mutation of
  # the module's `name`/`to_s`/`inspect` and would change validation
  # error-message text. Such sigs take the family validators instead.
  PERMANENT_MODULE_NAME_RE = /\A[A-Z][a-zA-Z0-9_]*(?:::[A-Z][a-zA-Z0-9_]*)*\z/
  private_constant(:PERMANENT_MODULE_NAME_RE)

  # Registry of per-(owner, method name) compile state. Mirrors the
  # single-Hash-op discipline of the registries in `_methods.rb` (each
  # store/lookup/delete is one Hash operation; no locks), so this is legal in
  # any execution context, including signal trap handlers.
  #
  # Entry states:
  #   :compiling  - a build is in flight for `orig`
  #   :compiled   - `eval_path` identifies the live wrapper source
  #   :failed     - a compile for `orig` failed; never retried for that orig
  #   :shimmed    - an eager (boot-time) first-call shim was installed for
  #                 `orig`; `installed_um` is the shim. Recording this in the
  #                 registry (not just in @pending_shims) is what lets a
  #                 compile racing a re-shim see that a NEWER shim owns the
  #                 key and surrender instead of clobbering it.
  #   :superseded - the most recent install decision for this key was NOT a
  #                 compiled wrapper (family validator, restored original,
  #                 abstract wrapper, ...). `installed_um` records what was
  #                 installed so stale pre-wrap handles can be told apart
  #                 from genuine redefinitions.
  Entry = Struct.new(:orig, :gen, :state, :eval_path, :sig, :visibility, :installed_um)
  private_constant(:Entry)

  PendingShim = Struct.new(:mod, :install_name, :orig, :sig, :visibility, :shim_um)
  private_constant(:PendingShim)

  if defined?(Concurrent::Hash)
    @compiled = Concurrent::Hash.new
    @pending_shims = Concurrent::Hash.new
    @mods_with_entries = Concurrent::Hash.new
  else
    @compiled = {}
    @pending_shims = {}
    # Pre-filter for the per-wrap registry probes: object_id => true for
    # every module that has (or is about to get) an entry in `@compiled`.
    # Keyed by Integer so a miss -- the overwhelmingly common case on the
    # wrap path -- costs no allocation (building the "#{oid}##{name}" key
    # String for a lookup that misses costs two). Markers are written
    # BEFORE the corresponding `@compiled` write and never deleted, so this
    # is always a superset of the modules present in `@compiled` (a false
    # positive just falls through to the precise lookup). Single-Hash-op
    # discipline like the registries themselves.
    @mods_with_entries = {}
  end
  @enabled = true
  @nonce_seq = 0

  EMPTY_ARGS = [].freeze
  private_constant(:EMPTY_ARGS)

  class << self
    # Kill switch. Provides eventual quiescence: no compile *begins* after the
    # flag write becomes visible to a thread; a thread already past the check
    # may finish one in-flight compile. (Parity with the TOCTOU window of
    # `disable_lazy_evaluation!`.)
    def disable!
      @enabled = false
    end

    def enable!
      @enabled = true
    end

    def enabled?
      @enabled
    end

    # True when the method currently installed for `name` in `mod`'s own
    # method table is one of our compiled wrappers.
    def compiled_wrapper?(mod, name)
      um = own_method(mod, name)
      !um.nil? && wrapper_path?(um.source_location&.first)
    end

    # Entry point from `CallValidation.create_validator_method`. Returns true
    # when the compiled tier installed (or already owns, or deliberately left
    # in place) a validator for this method; false means the caller must
    # install a family validator.
    def try_compile(mod, original_method, method_sig, original_visibility)
      return false unless @enabled
      return false unless eligible?(mod, original_method, method_sig)
      install_name = method_sig.method_name

      if eager_context?
        # Boot-time wrapping (`run_all_sig_blocks`): defer the (compara-
        # tively expensive) source compile to the first call by installing a
        # cheap shim, so eagerly-wrapped-but-never-called methods don't slow
        # boot down. Registry statuses are always consulted first (one hash
        # read; the expensive part -- the source compile -- stays deferred)
        # so a shim is never installed over an already-live compiled wrapper
        # and re-shims of a key go through the state machine.
        status = compile_now!(mod, install_name, original_method, method_sig, original_visibility, build: false)
        case status
        when :would_build
          install_shim(mod, install_name, original_method, method_sig, original_visibility)
          true
        when nil, false
          false
        else
          true
        end
      else
        !!compile_now!(mod, install_name, original_method, method_sig, original_visibility)
      end
    end

    # Re-entry gate, called at the very top of
    # `CallValidation.wrap_method_if_needed` (the single choke point both
    # stale-handle vectors flow through: `_handle_missing_method_signature`'s
    # alias branch via `unwrap_method`, and the public
    # `T::Utils.wrap_method_with_call_validation_if_needed`).
    #
    # Returns the live, already-correct method (as an UnboundMethod) when the
    # wrap request is a re-entry for a (owner, name) whose current install
    # decision is already in effect -- in which case the caller must install
    # NOTHING (zero artifact growth, zero redefinition). Returns nil to fall
    # through to the normal wrap path.
    def intercept_reentry(mod, method_sig, original_method)
      return nil unless @mods_with_entries[mod.object_id]
      name = method_sig.method_name
      e = @compiled["#{mod.object_id}##{name}"]
      return nil unless e

      # The passed "original" is itself one of our compiled wrappers (the
      # `T::Utils.wrap_method_with_call_validation_if_needed(mod, sig,
      # mod.instance_method(name))` vector). Never compile a wrapper around a
      # wrapper: same-sig re-wraps are no-ops; a *different* sig falls
      # through, which stacks a family validator on top (today's behavior for
      # repeated public-API wraps).
      if wrapper_path?(original_method.source_location&.first)
        return sig_matches?(e, method_sig) ? (own_method(mod, name) || mod.instance_method(name)) : nil
      end

      return nil unless e.orig == original_method || sig_matches?(e, method_sig)

      case e.state
      when :compiling
        # A build for this same method is in flight; install nothing and let
        # the in-flight call validate interpreted. (Without this, a stale
        # handle landing in the build window could clobber the build.)
        own_method(mod, name) || mod.instance_method(name)
      when :compiled
        own = own_method(mod, name)
        own if own && own.source_location&.first == e.eval_path
      when :shimmed, :superseded, :failed
        own = own_method(mod, name)
        own if own && e.installed_um && own == e.installed_um
      end
    end

    # Records that a non-compiled install decision (family validator,
    # restored original, abstract wrapper, raw def) was just made for this
    # method, superseding any previous compiled-tier state for the key.
    # Without this, a later stale pre-wrap handle could be misclassified as a
    # redefinition and mint a wrapper around an outdated method body.
    def note_install_decision(mod, install_name, original_method, method_sig, visibility)
      return unless @mods_with_entries[mod.object_id]
      key = "#{mod.object_id}##{install_name}"
      e = @compiled[key]
      return unless e
      @pending_shims.delete(key)
      state = (e.state == :failed && e.orig == original_method) ? :failed : :superseded
      @compiled[key] = Entry.new(
        original_method, e.gen, state, nil, method_sig, visibility, own_method(mod, install_name)
      ).freeze
      nil
    end

    # Compiles every still-pending (shimmed, not yet called) method now.
    # Explicit operator hook, exempt from `disable_lazy_evaluation!` exactly
    # like `eagerly_define_lazy_methods!` is: all inputs are in version
    # control. Intended deployment recipe for hardened configurations:
    #
    #   T::Utils.run_all_sig_blocks
    #   T::Utils.compile_pending_sig_wrappers!
    #   T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
    def compile_pending!
      prev = Thread.current[:__t_sig_force_eval]
      Thread.current[:__t_sig_force_eval] = true
      @pending_shims.keys.each do |key|
        ps = @pending_shims[key]
        next unless ps
        # Only compile if OUR shim is still the live def for the name. A
        # redefinition (sigged or not), remove_method, or alias-decoration
        # since the shim was installed makes this entry stale; compiling it
        # would resurrect an outdated body over the user's current method.
        # Stale records are dropped (a stale shim firing later finds no
        # pending record and stays inert).
        own = own_method(ps.mod, ps.install_name)
        unless own && own == ps.shim_um
          @pending_shims.delete(key) if @pending_shims[key].equal?(ps)
          next
        end
        # A frozen owner can never be compiled or family-installed; the shim
        # stays live forever and validates every call (see shim_fired).
        next if ps.mod.frozen?
        status = compile_now!(ps.mod, ps.install_name, ps.orig, ps.sig, ps.visibility)
        if status
          @pending_shims.delete(key) if @pending_shims[key].equal?(ps)
        elsif install_family_fallback(ps.mod, ps.install_name, ps.orig, ps.sig, ps.visibility)
          @pending_shims.delete(key) if @pending_shims[key].equal?(ps)
        end
      end
      nil
    ensure
      Thread.current[:__t_sig_force_eval] = prev
    end

    # First call of an eagerly-installed shim. Compiles (or falls back to a
    # family validator); the caller (the shim block itself) then validates
    # the in-flight call via `CallValidation.validate_call`, exactly like the
    # first-call thunk in `_on_method_added` does.
    #
    # A fired shim is only allowed to mutate the method table when ALL of:
    #   1. our pending record still owns the key (a re-shim for a sigged
    #      redefinition replaces it, making us stale),
    #   2. OUR shim is still `mod`'s own live def for the name (an
    #      alias_method-decoration, unsigged redefinition, or stale captured
    #      Method handle fires us through a method entry that is no longer
    #      the name's dispatch target -- compiling would clobber the user's
    #      current method), and
    #   3. the owner is not frozen (no method-table or constant write can
    #      succeed; the shim stays live forever and validates every call).
    # When any check fails we install NOTHING and KEEP the pending record,
    # so later fires through the same stale shim stay equally inert (zero
    # growth per call); the in-flight call still validates via the shim's
    # validate_call tail.
    def shim_fired(mod, install_name, original_method, method_sig, visibility)
      key = "#{mod.object_id}##{install_name}"
      pending = @pending_shims[key]
      return unless pending && pending.orig.equal?(original_method)
      own = own_method(mod, install_name)
      return unless own && own == pending.shim_um
      return if mod.frozen?
      status = compile_now!(mod, install_name, original_method, method_sig, visibility)
      if status
        # Drop only our own record (a racing re-shim may have replaced it;
        # the registry state machine and `build_wrapper`'s post-claim guard
        # ensure the newer shim's method-table slot was not clobbered).
        @pending_shims.delete(key) if @pending_shims[key].equal?(pending)
      elsif install_family_fallback(mod, install_name, original_method, method_sig, visibility)
        @pending_shims.delete(key) if @pending_shims[key].equal?(pending)
      end
      nil
    end

    # Compile (or resolve the registry state for) `install_name` on `mod`.
    #
    # Truthy returns mean "a validator for this method is in place (or will
    # be momentarily); install nothing further": :compiled, :already, :stale,
    # (and with build: false, :would_build). nil/false returns mean "fall
    # back to the family validators".
    def compile_now!(mod, install_name, original_method, method_sig, visibility, build: true)
      return nil unless @enabled
      return nil unless eval_allowed?

      key = "#{mod.object_id}##{install_name}".freeze
      e = @compiled[key]
      gen = 1
      if e
        # The passed "original" is itself one of our wrappers; never wrap a
        # wrapper (see intercept_reentry; this arm is defense in depth for
        # flows that reach routing directly).
        if wrapper_path?(original_method.source_location&.first)
          return sig_matches?(e, method_sig) ? :stale : nil
        end

        same = e.orig == original_method || sig_matches?(e, method_sig)
        case e.state
        when :compiled
          live = wrapper_live?(mod, install_name, e.eval_path)
          if same
            return :already if live
            # Our registry says compiled but our wrapper is not the live
            # def (cross-tier race, or an unsigged redefinition followed by a
            # stale-handle re-wrap, which is today-parity clobber behavior):
            # rebuild at the same generation.
            gen = e.gen
          elsif live
            # Different method, but OUR current-generation wrapper is the
            # live def: the passed handle is a STALE pre-wrap handle. Define
            # nothing, eval nothing; the in-flight call (if any) validates
            # interpreted and converges. Zero growth per stale call.
            return :stale
          else
            # Genuine redefinition: mint exactly one new generation.
            gen = e.gen + 1
          end
        when :compiling
          if same
            # Sanctioned co-proceed: an equivalent build is in flight; build
            # our own (nonce-isolated) artifact set. Whichever def lands last
            # wins and is internally consistent.
            gen = e.gen
          else
            # A different method's build is in flight; do not race it.
            return nil
          end
        when :shimmed
          if same
            # The normal first-fire flow for this key's shim (or an explicit
            # `compile_pending!`): build now, replacing the shim.
            gen = e.gen
          elsif original_method == e.installed_um
            # The caller is wrapping the LIVE SHIM itself with a different
            # sig (public `wrap_method_with_call_validation_if_needed`
            # vector). Fall back so a family validator stacks on top of the
            # shim, today-parity with stacking over any other validator.
            return nil
          elsif e.installed_um && (own = own_method(mod, install_name)) && own == e.installed_um
            # A NEWER shim owns this key and is the live def; the passed
            # method is an outdated original (a first call through a now
            # stale shim, racing a sigged redefinition's re-shim). Build
            # NOTHING: the newer shim's own first call decides. The
            # in-flight call still validates via its shim's validate_call.
            return :stale
          else
            # The shim was replaced by something we did not install (e.g. an
            # unsigged redefinition) and a wrap re-entry got here: genuine
            # redefinition handling, today-parity.
            gen = e.gen + 1
          end
        when :failed
          if same
            return nil # terminal for this orig; no retry, family validator
          elsif e.installed_um && (own = own_method(mod, install_name)) && own == e.installed_um
            return :stale # the failure-decision install is live; stale handle
          else
            gen = e.gen + 1
          end
        when :superseded
          if !same && e.installed_um && (own = own_method(mod, install_name)) && own == e.installed_um
            return :stale # the superseding install is live; stale handle
          end
          gen = e.gen + 1
        end
      end

      return :would_build unless build
      build_wrapper(mod, key, install_name, original_method, method_sig, visibility, gen)
    end

    private

    def eval_allowed?
      return true if Thread.current[:__t_sig_force_eval]
      # Adopt the existing post-boot eval boundary (go/M8yrvzX2). The props
      # subsystem loads after this file; until it is loaded there is no
      # policy to consult (and nothing can have disabled it). Read the ivar
      # directly: `lazy_evaluation_enabled?` is itself a sigged method, so
      # calling it from inside the wrap path would recurse into wrapping.
      !defined?(T::Props::HasLazilySpecializedMethods) ||
        !T::Props::HasLazilySpecializedMethods.instance_variable_get(:@lazy_evaluation_disabled)
    end

    def eager_context?
      depth = Thread.current[:__t_sig_eager_depth]
      depth ? depth > 0 : false
    end

    def wrapper_path?(path)
      !path.nil? && path.start_with?(WRAPPER_PATH_PREFIX)
    end

    # Resolves `mod`'s OWN definition of `name`, skipping prepended modules
    # and other ancestors. `mod.instance_method(name)` resolves through the
    # full MRO, so under `Module#prepend` it would answer questions about the
    # prepended override instead of our slot.
    def own_method(mod, name)
      um = mod.instance_method(name)
      um = um.super_method while um && !um.owner.equal?(mod)
      um
    rescue NameError
      nil
    end
    public :own_method # used by tests; not API

    def wrapper_live?(mod, name, eval_path)
      return false if eval_path.nil?
      um = own_method(mod, name)
      !um.nil? && um.source_location&.first == eval_path
    end

    def sig_matches?(e, method_sig)
      s = e.sig
      return false unless s
      s.equal?(method_sig) ||
        (s.method.equal?(method_sig.method) && s.method_name == method_sig.method_name)
    end

    def current_visibility(mod, name, fallback)
      T::Private::ClassUtils.visibility_method_name(mod, name)
    rescue NameError
      fallback
    end

    # Installs the family validator as the fallback for a shim whose compile
    # was refused or failed. Returns false (leaving the live shim in place,
    # where it correctly validates every call) when the owner was frozen
    # between the caller's `frozen?` check and the install: raising
    # FrozenError out of a plain method call -- repeatedly, forever -- would
    # brick a method that works on the interpreted path.
    def install_family_fallback(mod, install_name, original_method, method_sig, visibility)
      CallValidation.install_family_validator(mod, original_method, method_sig, current_visibility(mod, install_name, visibility))
      note_install_decision(mod, install_name, original_method, method_sig, visibility)
      true
    rescue ::FrozenError
      false
    end

    def eligible?(mod, original_method, method_sig)
      # Ordered cheapest-and-most-discriminating first: this runs once per
      # method wrap at boot, and most ineligible sigs fail on shape (kwargs,
      # optionals, rest), which costs only attr reads -- the checks that
      # allocate (refinement_module?'s unbound to_s String, safe_method_name?,
      # types_bindable?'s Module#name reads) only run for shapes that pass.
      CallValidation.is_allowed_to_have_fast_path &&
        !method_sig.bind &&
        method_sig.kwarg_types.empty? &&
        method_sig.rest_type.nil? &&
        method_sig.keyrest_type.nil? &&
        method_sig.arg_types.length < 7 &&
        method_sig.parameters.all? { |(kind, _name)| kind == :req || kind == :block } &&
        method_sig.block_type&.valid?(nil) != false &&
        mod.equal?(original_method.owner) &&
        !mod.frozen? &&
        safe_method_name?(method_sig.method_name) &&
        !refinement_module?(mod) &&
        types_bindable?(method_sig)
    end

    # Whether every Module value the wrapper would `const_set` into its
    # container -- `Simple` raw types and `SimplePairUnion` halves, for both
    # parameters and the return slot -- is permanently named. Anonymous (or
    # temporarily/pseudo-named) modules must never be bound: the binding
    # would permanently rename the user's module (a global, user-visible
    # mutation of `name`/`to_s`/`inspect`) and change validation
    # error-message text. Such sigs take the family validators (correctness
    # over coverage, the same pattern as the reserved-name allowlist).
    # Complex types are bound as `T::Types::Base` OBJECTS (not Modules),
    # which `const_set` never renames, and their emitted checks call
    # `.valid?` on the bound object -- no raw module is bound for them.
    #
    # Decided at wrap time (so calls 1..N take the identical family path) and
    # re-checked at the binding site in `build_wrapper_body`. The decision is
    # stable: a permanent name can never be removed, so a sig that passes
    # here still passes when a deferred (shimmed) compile runs later.
    def types_bindable?(method_sig)
      method_sig.arg_types.each do |(_name, type)|
        return false unless type_bindable?(type)
      end
      type_bindable?(method_sig.effective_return_type)
    end

    def type_bindable?(type)
      if type.is_a?(T::Types::Simple)
        permanently_named?(type.raw_type)
      elsif type.instance_of?(T::Private::Types::SimplePairUnion)
        raw_a, raw_b = simple_pair_raw_types(type)
        permanently_named?(raw_a) && permanently_named?(raw_b)
      else
        true
      end
    end

    # True when binding `raw` under a constant cannot rename it (see
    # PERMANENT_MODULE_NAME_RE).
    def permanently_named?(raw)
      name = UNBOUND_MODULE_NAME.bind_call(raw)
      !name.nil? && PERMANENT_MODULE_NAME_RE.match?(name)
    end

    # Refinement modules must never compile: the wrapper's receiverless
    # private call to the stash cannot resolve from a `module_eval`'d def on
    # a refinement module (NoMethodError at call time -- after the registry
    # was finalized, so there would be no fallback). The bind_call families
    # work on refinements, so they take that path.
    #
    # `::Refinement` only exists on Ruby >= 3.2; on the 3.0/3.1 floor a
    # refinement module is a plain Module, so `is_a?` alone is inert there
    # and we additionally check `Module#to_s`'s uninheritable
    # `#<refinement:...>` marker (stable output across 2.x-3.x, read through
    # the unbound `Module#to_s` so user overrides cannot mask it).
    def refinement_module?(mod)
      return true if NATIVE_REFINEMENT_CLASS && mod.is_a?(NATIVE_REFINEMENT_CLASS)
      return false if REFINEMENT_CLASS_IS_COMPLETE
      UNBOUND_MODULE_TO_S.bind_call(mod).start_with?("#<refinement:")
    end
    public :refinement_module? # used by tests; not API

    def safe_method_name?(name)
      # `Symbol#name` (3.0+) returns the interned frozen String: no per-call
      # allocation, unlike `Symbol#to_s`.
      s = name.name
      return false unless METHOD_NAME_RE.match?(s)
      return false if RESERVED_WORDS.key?(s) || s.start_with?("__t_", "SORBET_RT_")
      # Names like `if=`/`end?` reduce to a reserved base; only slice when a
      # suffix is actually present (METHOD_NAME_RE allows at most one).
      if s.end_with?("=", "?", "!")
        return false if RESERVED_WORDS.key?(s[0..-2])
      end
      true
    end

    # `?`/`!`/`=` cannot appear inside identifiers, so escape them (and the
    # escape character) injectively when embedding the install name in stash
    # and container names.
    def escape_name(name)
      name.to_s.gsub("_", "_u").gsub("?", "_q").gsub("!", "_b").gsub("=", "_e")
    end

    # Unique-enough token for one build attempt. Two concurrent builds for
    # the same (key, gen) get disjoint artifact names, so even a torn race
    # ends with whole, internally-consistent artifacts (the last `def` to
    # land wins and references only its own stash/constants).
    def next_nonce
      seq = (@nonce_seq += 1) # benign data race: thread id disambiguates
      "#{Thread.current.object_id.to_s(36)}x#{seq.to_s(36)}"
    end

    def build_wrapper(mod, key, install_name, original_method, method_sig, visibility, gen)
      nonce = next_nonce
      eval_path = "#{WRAPPER_PATH_PREFIX}#{key}:g#{gen}:n#{nonce})"
      claim = Entry.new(original_method, gen, :compiling, eval_path, method_sig, visibility, nil).freeze
      # Pre-filter marker first, so it is a superset of `@compiled`'s modules
      # at every instant (see its definition).
      @mods_with_entries[mod.object_id] = true
      @compiled[key] = claim

      # Post-claim guard: if a pending shim for a DIFFERENT method owns this
      # key (a sigged redefinition re-shimmed it between our state-machine
      # read and our claim write), surrender -- build nothing, restore the
      # newer shim's registry entry. Without this, a first call racing a
      # boot-time re-wrap could compile an outdated original over the new
      # shim.
      ps = @pending_shims[key]
      if ps && !(ps.orig == original_method)
        @compiled[key] = Entry.new(ps.orig, gen, :shimmed, nil, ps.sig, ps.visibility, ps.shim_um).freeze
        return :stale
      end

      begin
        build_wrapper_body(mod, key, install_name, original_method, method_sig, visibility, gen, nonce, eval_path, claim)
      ensure
        # A non-StandardError unwind (Thread#kill, SystemExit, Interrupt)
        # between claim and finalize would otherwise latch this key in
        # :compiling forever, short-circuiting every future wrap for the
        # method. Demote the orphaned claim to :failed so the next wrap
        # installs a family validator.
        cur = @compiled[key]
        if cur.equal?(claim)
          @compiled[key] = Entry.new(original_method, gen, :failed, nil, method_sig, visibility, nil).freeze
        end
      end
    end

    def build_wrapper_body(mod, key, install_name, original_method, method_sig, visibility, gen, nonce, eval_path, claim)
      esc = escape_name(install_name)
      container_name = :"SORBET_RT_SIG_#{esc}_G#{gen}_N#{nonce}"
      stash_name = :"__t_sig_orig_#{mod.object_id}_g#{gen}_n#{nonce}_#{esc}"

      # Foreign-name probes: never clobber (or silently privatize) anything
      # that already exists under these names. The nonce makes a collision
      # all but impossible; if one happens anyway, fall back.
      if mod.const_defined?(container_name, false) ||
         mod.method_defined?(stash_name, false) || mod.private_method_defined?(stash_name, false)
        @compiled[key] = Entry.new(original_method, gen, :failed, nil, method_sig, visibility, nil).freeze
        return nil
      end

      # Anonymous-module guard, re-checked at the binding site: `const_set`
      # of an unnamed (or temporarily/pseudo-named) module would permanently
      # rename it (see PERMANENT_MODULE_NAME_RE). Wrap-time eligibility
      # already refused such sigs before installing anything; this recheck
      # covers any flow that reaches a build without that gate (e.g. a direct
      # `compile_now!`). Must run before ANY module value is bound below --
      # even binding into the still-anonymous container renames immediately.
      unless types_bindable?(method_sig)
        @compiled[key] = Entry.new(original_method, gen, :failed, nil, method_sig, visibility, nil).freeze
        return nil
      end

      container = Module.new
      container.const_set(:SIG, method_sig)
      arg_types = method_sig.arg_types
      arg_types.each_with_index do |(_name, type), i|
        if type.is_a?(T::Types::Simple)
          container.const_set(:"A#{i}", type.raw_type)
        elsif type.instance_of?(T::Private::Types::SimplePairUnion)
          # Inline `SimplePairUnion#valid?` (`obj.is_a?(@raw_a) ||
          # obj.is_a?(@raw_b)`) to skip the `valid?` dispatch; the type
          # object itself is kept for error reporting.
          container.const_set(:"A#{i}", type)
          raw_a, raw_b = simple_pair_raw_types(type)
          container.const_set(:"A#{i}A", raw_a)
          container.const_set(:"A#{i}B", raw_b)
        else
          container.const_set(:"A#{i}", type)
        end
      end
      effective_return_type = method_sig.effective_return_type
      return_kind = classify_return(effective_return_type)
      case return_kind
      when :simple
        container.const_set(:R, effective_return_type.raw_type)
      when :simple_pair
        raw_a, raw_b = simple_pair_raw_types(effective_return_type)
        container.const_set(:RA, raw_a)
        container.const_set(:RB, raw_b)
      when :complex
        container.const_set(:R, effective_return_type)
      end

      # Re-read the effective visibility just before installing: it may have
      # changed since the wrap decision (e.g. `private :foo` after the def,
      # with the compile deferred behind a shim).
      effective_visibility = current_visibility(mod, install_name, visibility)
      source = wrapper_source(install_name, container_name, stash_name, method_sig, return_kind, effective_visibility)

      T::Configuration.without_ruby_warnings do
        T::Private::DeclState.current.without_on_method_added do
          mod.const_set(container_name, container)
          mod.send(:private_constant, container_name)
          T::Private::ClassUtils.def_with_visibility(mod, stash_name, :private, original_method)
          mod.module_eval(source, eval_path, 1)
          # The visibility region line in the source already applied the
          # visibility (so `method_added` observers see the right visibility
          # from the beginning), but this explicit send is the load-bearing
          # mechanism for names Ruby auto-privatizes (`initialize` & co.): a
          # visibility region does not override that special case.
          mod.send(effective_visibility, install_name)
        end
      end

      final = Entry.new(original_method, gen, :compiled, eval_path, method_sig, visibility, nil).freeze
      cur = @compiled[key]
      if cur.equal?(claim)
        @compiled[key] = final
        # Identity-guarded: only drop a pending shim that belongs to the
        # method we just compiled. A re-shim that raced us must keep its
        # pending record (the demotion paths below and the re-shim's own
        # post-verify converge the method table).
        pending = @pending_shims[key]
        @pending_shims.delete(key) if pending && pending.orig == original_method
      elsif cur && !(cur.orig == original_method)
        case cur.state
        when :shimmed
          # A sigged redefinition re-shimmed this key while we were building,
          # and our module_eval may have just clobbered the new shim.
          # Restore it from the recorded UnboundMethod so the redefined
          # method's own first call decides.
          if cur.installed_um
            T::Configuration.without_ruby_warnings do
              T::Private::DeclState.current.without_on_method_added do
                T::Private::ClassUtils.def_with_visibility(mod, install_name, cur.visibility, cur.installed_um)
                mod.send(cur.visibility, install_name)
              end
            end
            # The restored def is a new method entry; refresh the pending
            # record's liveness token so the shim's first fire still passes
            # its own-def check and compiles.
            ps2 = @pending_shims[key]
            if ps2 && ps2.orig == cur.orig
              @pending_shims[key] = PendingShim.new(
                ps2.mod, ps2.install_name, ps2.orig, ps2.sig, ps2.visibility, own_method(mod, install_name)
              ).freeze
            end
          end
        when :superseded, :failed
          # While we were building, another thread recorded a different
          # install decision for this key (e.g. a redefinition wrap lost the
          # :compiling race and installed a family validator that our
          # module_eval may have just clobbered). Demote: reinstall that
          # decision's validator so the newest method body wins.
          CallValidation.install_family_validator(mod, cur.orig, cur.sig, current_visibility(mod, install_name, cur.visibility))
          note_install_decision(mod, install_name, cur.orig, cur.sig, cur.visibility)
        end
      end
      :compiled
    rescue ::ScriptError, ::StandardError
      # Compilation is strictly an optimization: any failure (SyntaxError,
      # FrozenError, NameError, ...) falls back to the proven family
      # validators. Terminal for this method: no retry, no growth.
      @compiled[key] = Entry.new(original_method, gen, :failed, nil, method_sig, visibility, nil).freeze
      nil
    end

    def classify_return(effective_return_type)
      if effective_return_type.is_a?(T::Private::Types::Void)
        :void
      elsif effective_return_type.equal?(T::Types::Untyped::Private::INSTANCE) ||
            effective_return_type.equal?(T::Types::Anything::Private::INSTANCE) ||
            effective_return_type.equal?(T::Types::AttachedClassType::Private::INSTANCE) ||
            effective_return_type.equal?(T::Types::SelfType::Private::INSTANCE) ||
            effective_return_type.is_a?(T::Types::TypeParameter) ||
            effective_return_type.is_a?(T::Types::TypeVariable) ||
            (effective_return_type.is_a?(T::Types::Simple) && effective_return_type.raw_type.equal?(BasicObject))
        :ignorable
      elsif effective_return_type.is_a?(T::Types::Simple)
        :simple
      elsif effective_return_type.instance_of?(T::Private::Types::SimplePairUnion)
        :simple_pair
      else
        :complex
      end
    end

    def simple_pair_raw_types(type)
      # SimplePairUnion keeps `@raw_a`/`@raw_b` private (its `types` method
      # reconstructs the Simple types); read them via the public `types`.
      a, b = type.types
      [a.raw_type, b.raw_type]
    end

    # `is_a?(raw)` test for one half of an inlined SimplePairUnion check.
    # Deliberately NOT specialized to `arg.nil?` for a NilClass half:
    # `SimplePairUnion#valid?` is `obj.is_a?(@raw_a) || obj.is_a?(@raw_b)`,
    # and `nil?` dispatches to a user-overridable method, so a null-object
    # with `def nil?; true; end` would be accepted where the interpreted
    # check (and every base validator family) rejects it. `is_a?(NilClass)`
    # is exact parity and still an inline-cached fast check.
    def simple_pair_test(arg_expr, const_ref)
      "#{arg_expr}.is_a?(#{const_ref})"
    end

    # The wrapper bodies transcribe the generated validator families in
    # `call_validation_2_7.rb` (fast/medium x method/procedure/skip_return):
    # same checks, same `report_error` arguments (including which family's
    # `type:` argument shape gets reported -- the raw type for the all-simple
    # "fast" family, the `T::Types::Base` object otherwise), same
    # caller_offset (the compiled wrapper is exactly one stack frame, like
    # the bmethod wrappers it replaces).
    def wrapper_source(install_name, container_name, stash_name, method_sig, return_kind, visibility)
      c = container_name
      arg_types = method_sig.arg_types
      n = arg_types.length
      all_simple = arg_types.all? { |_name, type| type.is_a?(T::Types::Simple) }
      # Matches the family routing in `install_family_validator`: the fast
      # (all-simple) families report the *raw* type to the error handler.
      fast_family = all_simple && (return_kind != :complex && return_kind != :simple_pair)

      bare_args = (0...n).map { |i| "arg#{i}" }.join(", ")
      # ALWAYS declare and forward the block, exactly like the bind_call
      # families (`define_method(...) do |arg0, &blk| ... bind_call(self,
      # arg0, &blk)`). There is deliberately no attempt to prove the original
      # ignores its block: any such proof is unsound against aliased frame
      # reflection (`alias_method :bg, :block_given?` -- creatable even AFTER
      # compile) and `method(:block_given?)`-style indirection. On >= 3.1 the
      # anonymous form costs nothing (no Proc materialization); on 3.0 the
      # named form matches today's wrapper cost.
      blk = ANONYMOUS_BLOCK_FORWARDING ? "&" : "&__t_blk"
      params = n.zero? ? blk : "#{bare_args}, #{blk}"
      dispatch = "#{stash_name}(#{params})"

      checks = +""
      arg_types.each_with_index do |(_name, type), i|
        if type.is_a?(T::Types::Simple)
          test = "arg#{i}.is_a?(#{c}::A#{i})"
          reported = fast_family ? "#{c}::A#{i}" : "#{c}::SIG.arg_types[#{i}][1]"
        elsif type.instance_of?(T::Private::Types::SimplePairUnion)
          # Inlined `SimplePairUnion#valid?`, exactly: two `is_a?` checks.
          test = "#{simple_pair_test("arg#{i}", "#{c}::A#{i}A")} || " \
            "#{simple_pair_test("arg#{i}", "#{c}::A#{i}B")}"
          reported = "#{c}::A#{i}"
        else
          test = "#{c}::A#{i}.valid?(arg#{i})"
          reported = "#{c}::A#{i}"
        end
        checks << <<~RUBY
          unless #{test}
            ::T::Private::Methods::CallValidation.report_error(
              #{c}::SIG,
              #{c}::SIG.arg_types[#{i}][1].error_message_for_obj(arg#{i}),
              'Parameter',
              #{c}::SIG.arg_types[#{i}][0],
              #{reported},
              arg#{i},
              caller_offset: -1
            )
          end
        RUBY
      end

      tail =
        case return_kind
        when :void
          <<~RUBY
            #{dispatch}
            ::T::Private::Types::Void::VOID
          RUBY
        when :ignorable
          <<~RUBY
            #{dispatch}
          RUBY
        else
          test =
            case return_kind
            when :simple
              "return_value.is_a?(#{c}::R)"
            when :simple_pair
              "#{simple_pair_test('return_value', "#{c}::RA")} || " \
                "#{simple_pair_test('return_value', "#{c}::RB")}"
            else
              "#{c}::R.valid?(return_value)"
            end
          <<~RUBY
            return_value = #{dispatch}
            unless #{test}
              message = #{c}::SIG.effective_return_type.error_message_for_obj(return_value)
              if message
                ::T::Private::Methods::CallValidation.report_error(
                  #{c}::SIG,
                  message,
                  'Return value',
                  nil,
                  #{c}::SIG.effective_return_type,
                  return_value,
                  caller_offset: -1
                )
              end
            end
            return_value
          RUBY
        end

      <<~RUBY
        # frozen_string_literal: true
        #{visibility}

        def #{install_name}(#{params})
          # This method is a compiled version of more general code in `validate_call`
        #{checks.gsub(/^(?!$)/, "  ")}
          # The following line breaks are intentional to show nice pry message










          # PRY note:
          # this code is sig validation code.
          # Please issue `finish` to step out of it

        #{tail.gsub(/^(?!$)/, "  ")}
        end
      RUBY
    end

    # Cheap first-call shim used for boot-time (eager) wrapping: a
    # parameter-faithful bmethod that compiles on first call and validates
    # the in-flight call exactly like the first-call thunk in
    # `_on_method_added` (same `validate_call`, same stack depth).
    def install_shim(mod, install_name, original_method, method_sig, visibility)
      compiled = self
      cv = CallValidation
      shim =
        case method_sig.arg_types.length
        when 0
          proc do |&blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, EMPTY_ARGS, blk)
          end
        when 1
          proc do |arg0, &blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, [arg0], blk)
          end
        when 2
          proc do |arg0, arg1, &blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, [arg0, arg1], blk)
          end
        when 3
          proc do |arg0, arg1, arg2, &blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, [arg0, arg1, arg2], blk)
          end
        when 4
          proc do |arg0, arg1, arg2, arg3, &blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, [arg0, arg1, arg2, arg3], blk)
          end
        when 5
          proc do |arg0, arg1, arg2, arg3, arg4, &blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, [arg0, arg1, arg2, arg3, arg4], blk)
          end
        else
          proc do |arg0, arg1, arg2, arg3, arg4, arg5, &blk|
            compiled.shim_fired(mod, install_name, original_method, method_sig, visibility)
            cv.validate_call(self, original_method, method_sig, [arg0, arg1, arg2, arg3, arg4, arg5], blk)
          end
        end

      key = "#{mod.object_id}##{install_name}".freeze
      prev = @compiled[key]
      gen = prev ? prev.gen + 1 : 1

      # Pre-filter marker first (superset invariant; see its definition).
      @mods_with_entries[mod.object_id] = true
      installed = false
      3.times do
        T::Configuration.without_ruby_warnings do
          T::Private::DeclState.current.without_on_method_added do
            T::Private::ClassUtils.def_with_visibility(mod, install_name, visibility, ruby2_keywords: false, &shim)
            # def_with_visibility's visibility region does not override
            # Ruby's auto-private rule (initialize & co.); the explicit send
            # is load-bearing, exactly as in the family validators.
            mod.send(visibility, install_name)
          end
        end
        shim_um = own_method(mod, install_name)
        @pending_shims[key] = PendingShim.new(
          mod, install_name, original_method, method_sig, visibility, shim_um
        ).freeze
        # Recording :shimmed in the registry (not only in @pending_shims) is
        # what makes a compile racing this re-shim surrender (see the
        # :shimmed arm of compile_now! and build_wrapper's post-claim guard).
        @compiled[key] = Entry.new(
          original_method, gen, :shimmed, nil, method_sig, visibility, shim_um
        ).freeze
        # Post-verify: a concurrent compile of an OLDER generation (whose
        # first call was already in flight) may have module_eval'd its
        # wrapper over our shim between our def and our registry writes.
        # Whoever writes the method table last must leave the NEWEST
        # method's validator in place, so re-install until our shim is the
        # live def (bounded retries; the racing builder converges too via
        # its own demotion arm).
        if own_method(mod, install_name) == shim_um
          installed = true
          break
        end
      end
      unless installed
        # Give up racing: install the proven family validator synchronously
        # (we are inside the wrap path, so this is always legal here).
        @pending_shims.delete(key)
        CallValidation.install_family_validator(mod, original_method, method_sig, visibility)
        note_install_decision(mod, install_name, original_method, method_sig, visibility)
      end
      nil
    end
  end
end
