# typed: __STDLIB_INTERNAL

# The [GC](GC) module provides an interface to Ruby’s
# mark and sweep garbage collection mechanism.
#
# Some of the underlying methods are also available via the
# [ObjectSpace](https://ruby-doc.org/core-2.6.3/ObjectSpace.html) module.
#
# You may obtain information about the operation of the
# [GC](GC) through
# [GC::Profiler](https://ruby-doc.org/core-2.6.3/GC/Profiler.html) .
module GC
  INTERNAL_CONSTANTS = T.let(T.unsafe(nil), Hash)
  OPTS = T.let(T.unsafe(nil), Array)

  # The number of times [GC](GC.downloaded.ruby_doc) occurred.
  #
  # It returns the number of times [GC](GC.downloaded.ruby_doc) occurred
  # since the process started.
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

  # Initiates garbage collection, unless manually disabled.
  #
  # This method is defined with keyword arguments that default to true:
  #
  #     def GC.start(full_mark: true, immediate_sweep: true); end
  #
  # Use full\_mark: false to perform a minor [GC](GC.downloaded.ruby_doc) .
  # Use immediate\_sweep: false to defer sweeping (use lazy sweep).
  #
  # Note: These keyword arguments are implementation and version dependent.
  # They are not guaranteed to be future-compatible, and may be ignored if
  # the underlying implementation does not support them.
  sig do
    params(
        full_mark: T::Boolean,
        immediate_sweep: T::Boolean,
    )
    .returns(NilClass)
  end
  def self.start(full_mark: T.unsafe(nil), immediate_sweep: T.unsafe(nil)); end

  # Returns a [Hash](https://ruby-doc.org/core-2.6.3/Hash.html) containing
  # information about the [GC](GC.downloaded.ruby_doc) .
  #
  # The hash includes information about internal statistics about
  # [GC](GC.downloaded.ruby_doc) such as:
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
  # The contents of the hash are implementation specific and may be changed
  # in the future.
  #
  # This method is only expected to work on C Ruby.
  sig {params(arg0: T::Hash[Symbol, Integer]).returns(T::Hash[Symbol, Integer])}
  sig {params(arg0: Symbol).returns(Integer)}
  def self.stat(arg0={}); end

  # Returns current status of [GC](GC.downloaded.ruby_doc) stress mode.
  sig {returns(T.any(Integer, TrueClass, FalseClass))}
  def self.stress(); end
end

module GC::Profiler
  sig {void}
  def self.clear(); end

  sig {void}
  def self.disable(); end

  sig {void}
  def self.enable(); end

  sig {returns(T::Boolean)}
  def self.enabled?(); end

  sig {returns(T::Array[T::Hash[Symbol, T.untyped]])}
  def self.raw_data(); end

  sig do
    params(
      io: IO
    )
    .void
  end
  def self.report(io=T.unsafe(nil)); end

  sig {returns(String)}
  def self.result(); end

  sig {returns(Float)}
  def self.total_time(); end
end
