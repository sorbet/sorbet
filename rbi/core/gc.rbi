# typed: __STDLIB_INTERNAL

# The [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) module provides an
# interface to Ruby's mark and sweep garbage collection mechanism.
#
# Some of the underlying methods are also available via the
# [`ObjectSpace`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html) module.
#
# You may obtain information about the operation of the
# [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) through
# [`GC::Profiler`](https://docs.ruby-lang.org/en/2.7.0/GC/Profiler.html).
module GC
  # internal constants
  INTERNAL_CONSTANTS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) build options
  OPTS = T.let(T.unsafe(nil), T::Array[T.untyped])

  def self.compact; end

  # The number of times [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html)
  # occurred.
  #
  # It returns the number of times
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) occurred since the
  # process started.
  sig {returns(Integer)}
  def self.count(); end

  # Disables garbage collection, returning `true` if garbage collection was
  # already disabled.
  #
  # ```ruby
  # GC.disable   #=> false
  # GC.disable   #=> true
  # ```
  sig {returns(T::Boolean)}
  def self.disable(); end

  # Enables garbage collection, returning `true` if garbage collection was
  # previously disabled.
  #
  # ```ruby
  # GC.disable   #=> false
  # GC.enable    #=> true
  # GC.enable    #=> false
  # ```
  sig {returns(T::Boolean)}
  def self.enable(); end

  # Returns information about the most recent garbage collection.
  def self.latest_gc_info(*_); end

  #  Returns whether or not automatic compaction has been enabled.
  #
  # ```ruby
  # GC.auto_compact    #=> true or false
  # ```
  sig {returns(T::Boolean)}
  def self.auto_compact(); end

  #
  #  Updates automatic compaction mode.
  #
  #  When enabled, the compactor will execute on every major collection.
  #
  #  Enabling compaction will degrade performance on major collections.
  #
  # ```ruby
  # GC.auto_compact = flag
  # ```
  def self.auto_compact=(_); end

  # Initiates garbage collection, even if manually disabled.
  #
  # This method is defined with keyword arguments that default to true:
  #
  # ```ruby
  # def GC.start(full_mark: true, immediate_sweep: true); end
  # ```
  #
  # Use full\_mark: false to perform a minor
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html). Use immediate\_sweep:
  # false to defer sweeping (use lazy sweep).
  #
  # Note: These keyword arguments are implementation and version dependent. They
  # are not guaranteed to be future-compatible, and may be ignored if the
  # underlying implementation does not support them.
  sig do
    params(
        full_mark: T::Boolean,
        immediate_sweep: T::Boolean,
    )
    .returns(NilClass)
  end
  def self.start(full_mark: T.unsafe(nil), immediate_sweep: T.unsafe(nil)); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing
  # information about the [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html).
  #
  # The hash includes information about internal statistics about
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) such as:
  #
  # ```ruby
  # {
  #     :count=>0,
  #     :heap_allocated_pages=>24,
  #     :heap_sorted_length=>24,
  #     :heap_allocatable_pages=>0,
  #     :heap_available_slots=>9783,
  #     :heap_live_slots=>7713,
  #     :heap_free_slots=>2070,
  #     :heap_final_slots=>0,
  #     :heap_marked_slots=>0,
  #     :heap_eden_pages=>24,
  #     :heap_tomb_pages=>0,
  #     :total_allocated_pages=>24,
  #     :total_freed_pages=>0,
  #     :total_allocated_objects=>7796,
  #     :total_freed_objects=>83,
  #     :malloc_increase_bytes=>2389312,
  #     :malloc_increase_bytes_limit=>16777216,
  #     :minor_gc_count=>0,
  #     :major_gc_count=>0,
  #     :remembered_wb_unprotected_objects=>0,
  #     :remembered_wb_unprotected_objects_limit=>0,
  #     :old_objects=>0,
  #     :old_objects_limit=>0,
  #     :oldmalloc_increase_bytes=>2389760,
  #     :oldmalloc_increase_bytes_limit=>16777216
  # }
  # ```
  #
  # The contents of the hash are implementation specific and may be changed in
  # the future.
  #
  # This method is only expected to work on C Ruby.
  sig {params(arg0: T::Hash[Symbol, Integer]).returns(T::Hash[Symbol, Integer])}
  sig {params(arg0: Symbol).returns(Integer)}
  def self.stat(arg0={}); end

  # Returns information for memory pools in the \GC.
  #
  # If the first optional argument, +heap_name+, is passed in and not +nil+, it
  # returns a +Hash+ containing information about the particular memory pool.
  # Otherwise, it will return a +Hash+ with memory pool names as keys and
  # a +Hash+ containing information about the memory pool as values.
  #
  # If the second optional argument, +hash_or_key+, is given as +Hash+, it will
  # be overwritten and returned. This is intended to avoid the probe effect.
  #
  # If both optional arguments are passed in and the second optional argument is
  # a symbol, it will return a +Numeric+ of the value for the particular memory
  # pool.
  #
  # On CRuby, +heap_name+ is of the type +Integer+ but may be of type +String+
  # on other implementations.
  #
  # The contents of the hash are implementation specific and may change in
  # the future without notice.
  #
  # If the optional argument, hash, is given, it is overwritten and returned.
  #
  # This method is only expected to work on CRuby.
  sig { returns(T::Hash[Integer, T::Hash[Symbol, Integer]]) }
  sig { params(heap_name: T.any(Integer, String), hash_or_key: Symbol).returns(Integer) }
  sig do
    params(
      heap_name: T.any(Integer, String, NilClass),
      hash_or_key: T::Hash[T.untyped, T.untyped],
    ).returns(T::Hash[T.untyped, T.untyped])
  end
  sig { params(heap_name: T.any(Integer, String)).returns(T::Hash[Symbol, Integer]) }
  def self.stat_heap(heap_name = nil, hash_or_key = nil); end

  # Returns current status of
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) stress mode.
  sig {returns(T.any(Integer, TrueClass, FalseClass))}
  def self.stress(); end

  # Updates the [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) stress mode.
  #
  # When stress mode is enabled, the
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) is invoked at every
  # [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) opportunity: all memory
  # and object allocations.
  #
  # Enabling stress mode will degrade performance, it is only for debugging.
  #
  # flag can be true, false, or an integer bit-ORed following flags.
  #
  # ```ruby
  # 0x01:: no major GC
  # 0x02:: no immediate sweep
  # 0x04:: full mark after malloc/calloc/realloc
  # ```
  def self.stress=(_); end

  # Verify compaction reference consistency.
  #
  # This method is implementation specific. During compaction, objects that were
  # moved are replaced with T\_MOVED objects. No object should have a reference
  # to a T\_MOVED object after compaction.
  #
  # This function doubles the heap to ensure room to move all objects, compacts
  # the heap to make sure everything moves, updates all references, then
  # performs a full [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html). If any
  # object contains a reference to a T\_MOVED object, that object should be
  # pushed on the mark stack, and will make a SEGV.
  sig {params(toward: T.nilable(Symbol), double_heap: T::Boolean).void}
  def self.verify_compaction_references(toward: nil, double_heap: false); end

  # Verify internal consistency.
  #
  # This method is implementation specific. Now this method checks generational
  # consistency if RGenGC is supported.
  def self.verify_internal_consistency; end
end

# The [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) profiler provides
# access to information on [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html)
# runs including time, length and object space size.
#
# Example:
#
# ```ruby
# GC::Profiler.enable
#
# require 'rdoc/rdoc'
#
# GC::Profiler.report
#
# GC::Profiler.disable
# ```
#
# See also
# [`GC.count`](https://docs.ruby-lang.org/en/2.7.0/GC.html#method-c-count),
# [`GC.malloc_allocated_size`](https://docs.ruby-lang.org/en/2.7.0/GC.html#method-c-malloc_allocated_size)
# and
# [`GC.malloc_allocations`](https://docs.ruby-lang.org/en/2.7.0/GC.html#method-c-malloc_allocations)
module GC::Profiler
  # Clears the [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) profiler
  # data.
  sig {void}
  def self.clear(); end

  # Stops the [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) profiler.
  sig {void}
  def self.disable(); end

  # Starts the [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) profiler.
  sig {void}
  def self.enable(); end

  # The current status of [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html)
  # profile mode.
  sig {returns(T::Boolean)}
  def self.enabled?(); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # individual raw profile data Hashes ordered from earliest to latest by
  # `:GC_INVOKE_TIME`.
  #
  # For example:
  #
  # ```ruby
  # [
  #   {
  #      :GC_TIME=>1.3000000000000858e-05,
  #      :GC_INVOKE_TIME=>0.010634999999999999,
  #      :HEAP_USE_SIZE=>289640,
  #      :HEAP_TOTAL_SIZE=>588960,
  #      :HEAP_TOTAL_OBJECTS=>14724,
  #      :GC_IS_MARKED=>false
  #   },
  #   # ...
  # ]
  # ```
  #
  # The keys mean:
  #
  # `:GC_TIME`
  # :   [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) elapsed in
  #     seconds for this [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) run
  # `:GC_INVOKE_TIME`
  # :   [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) elapsed in
  #     seconds from startup to when the
  #     [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) was invoked
  # `:HEAP_USE_SIZE`
  # :   Total bytes of heap used
  # `:HEAP_TOTAL_SIZE`
  # :   Total size of heap in bytes
  # `:HEAP_TOTAL_OBJECTS`
  # :   Total number of objects
  # `:GC_IS_MARKED`
  # :   Returns `true` if the
  #     [`GC`](https://docs.ruby-lang.org/en/2.7.0/GC.html) is in mark phase
  #
  #
  # If ruby was built with `GC_PROFILE_MORE_DETAIL`, you will also have access
  # to the following hash keys:
  #
  # `:GC_MARK_TIME`
  # `:GC_SWEEP_TIME`
  # `:ALLOCATE_INCREASE`
  # `:ALLOCATE_LIMIT`
  # `:HEAP_USE_PAGES`
  # `:HEAP_LIVE_OBJECTS`
  # `:HEAP_FREE_OBJECTS`
  # `:HAVE_FINALIZE`
  # :
  sig {returns(T::Array[T::Hash[Symbol, T.untyped]])}
  def self.raw_data(); end

  # Writes the
  # [`GC::Profiler.result`](https://docs.ruby-lang.org/en/2.7.0/GC/Profiler.html#method-c-result)
  # to `$stdout` or the given
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  sig do
    params(
      io: IO
    )
    .void
  end
  def self.report(io=T.unsafe(nil)); end

  # Returns a profile data report such as:
  #
  # ```
  # GC 1 invokes.
  # Index    Invoke Time(sec)       Use Size(byte)     Total Size(byte)         Total Object                    GC time(ms)
  #     1               0.012               159240               212940                10647         0.00000000000001530000
  # ```
  sig {returns(String)}
  def self.result(); end

  # The total time used for garbage collection in seconds
  sig {returns(Float)}
  def self.total_time(); end
end
