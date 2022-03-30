# frozen_string_literal: true
# typed: true

require_relative './real_stdlib'
require_relative './status'

# This class walks global namespace to find all modules and discover all of their names.
# At the time you ask for it it, it takes a "spashot" of the world.
# If new modules were defined after an instance of this class was created,
# they won't be visible through a previously returned instance.
class Sorbet::Private::ConstantLookupCache

  ConstantEntry = Struct.new(:const_name, :found_name, :primary_name, :aliases, :const, :owner)

  # We won't be able to see if someone mankeypatches these.
  DEPRECATED_CONSTANTS = [
    '::Bignum',
    '::Config',
    '::Data',
    '::FALSE',
    '::Fixnum',
    '::NIL',
    '::TRUE',
    '::TimeoutError',
    '::SortedSet',
    'ERB::Compiler::SimpleScanner2',
    'Net::OpenSSL',
    'Object::Bignum',
    'Object::Config',
    'Object::Data',
    'Object::FALSE',
    'Object::Fixnum',
    'Object::NIL',
    'Object::TRUE',
    'Object::TimeoutError',
    'OpenSSL::Cipher::Cipher',
    'OpenSSL::Cipher::Digest',
    'OpenSSL::Digest::Cipher',
    'OpenSSL::Digest::Digest',
    'OpenSSL::SSL::SSLContext::METHODS',
    'Pry::Platform',
    'Pry::Prompt::MAP',
    'Sequel::BeforeHookFailed',
    'Sequel::Database::ResetIdentifierMangling',
    'Sequel::Error::AdapterNotFound',
    'Sequel::Error::InvalidOperation',
    'Sequel::Error::InvalidValue',
    'Sequel::Error::PoolTimeoutError',
    'Sequel::Error::Rollback',
    'Sequel::Model::ANONYMOUS_MODEL_CLASSES',
    'Sequel::Model::ANONYMOUS_MODEL_CLASSES_MUTEX',
    'Sequel::UnbindDuplicate',
    'Sequel::Unbinder',
    'YARD::Parser::Ruby::Legacy::RipperParser',
    'Java::ComHeadiusRacc::Cparse::CparseParams::NEVER',
    'Java::ComHeadiusRacc::Cparse::CparseParams::UNDEF',
    'Java::ComHeadiusRacc::Cparse::Parser::NEVER',
    'Java::ComHeadiusRacc::Cparse::Parser::UNDEF',
    'Java::OrgJruby::RubyBasicObject::NEVER',
    'Java::OrgJruby::RubyBasicObject::UNDEF',
    'Java::OrgJruby::RubyClass::NEVER',
    'Java::OrgJruby::RubyClass::UNDEF',
    'Java::OrgJruby::RubyModule::NEVER',
    'Java::OrgJruby::RubyModule::UNDEF',
    'Java::OrgJruby::RubyObject::NEVER',
    'Java::OrgJruby::RubyObject::UNDEF',
  ].freeze

  def initialize
    @all_constants = {}
    dfs_module(Object, nil, @all_constants, nil)
    Sorbet::Private::Status.done
    @consts_by_name = {}
    @all_constants.each_value do |struct|
      fill_primary_name(struct)
      @consts_by_name[struct.primary_name] = struct.const
    end
  end

  def all_module_names
    ret = @all_constants.select {|_k, v| Sorbet::Private::RealStdlib.real_is_a?(v.const, Module)}.map do |_key, struct|
      raise "should never happen" if !struct.primary_name
      struct.primary_name
    end
    ret
  end

  def all_named_modules
    ret = @all_constants.select {|_k, v| Sorbet::Private::RealStdlib.real_is_a?(v.const, Module)}.map do |_key, struct|
      raise "should never happen" if !struct.primary_name
      struct.const
    end
    ret
  end

  def all_module_aliases
    ret = {}

    @all_constants.map do |_key, struct|
      next if struct.nil? || !Sorbet::Private::RealStdlib.real_is_a?(struct.const, Module) || struct.aliases.size < 2
      if struct.owner != nil
        ret[struct.primary_name] = struct.aliases.reject do |name|
          # ignore the primary
          next true if name == struct.primary_name

          prefix, _, _ = name.rpartition('::')

          # an alias that exists at the top-level
          next false if prefix == ""

          # if the prefix is the same syntactically, then this is a good alias
          next false if prefix == struct.owner.primary_name

          # skip the alias if the owner is the same
          other_owner_const = Sorbet::Private::RealStdlib.real_const_get(Object, prefix, false)
          struct.owner.const == other_owner_const
        end
      else
        # top-level names
        ret[struct.primary_name] = struct.aliases.reject {|name| name == struct.primary_name }
      end
    end
    ret
  end

  def class_by_name(name)
    @consts_by_name[name]
  end

  def name_by_class(klass)
    @all_constants[Sorbet::Private::RealStdlib.real_object_id(klass)]&.primary_name
  end

  private def dfs_module(mod, prefix, ret, owner)
    raise "error #{prefix}: #{mod} is not a module" if !Sorbet::Private::RealStdlib.real_is_a?(mod, Module)
    name = Sorbet::Private::RealStdlib.real_name(mod)
    Sorbet::Private::Status.say("Naming #{name}", print_without_tty: false)
    return if name == 'RSpec::ExampleGroups' # These are all anonymous classes and will be quadratic in the number of classes to name them. We also know they don't have any hidden definitions
    begin
      constants = Sorbet::Private::RealStdlib.real_constants(mod)
    rescue TypeError
      puts "Failed to call `constants` on #{prefix}"
      return nil
    end
    go_deeper = [] # make this a bfs to prefer shorter names
    constants.each do |nested|
      begin
        next if Sorbet::Private::RealStdlib.real_autoload?(mod, nested) # some constants aren't autoloaded even after require_everything, e.g. StateMachine::Graph

        begin
          next if DEPRECATED_CONSTANTS.include?("#{prefix}::#{nested}") # avoid stdout spew
        rescue NameError
          # If it isn't to_s-able, that is ok, it won't be in our deprecated list
          nil
        end

        begin
          nested_constant = Sorbet::Private::RealStdlib.real_const_get(mod, nested, false) # rubocop:disable PrisonGuard/NoDynamicConstAccess
        rescue LoadError => err
          puts "Got #{err.class} when trying to get nested name #{name}::#{nested}"
          next
        rescue NameError, ArgumentError => err
          puts "Got #{err.class} when trying to get nested name #{name}::#{nested}_"
          # some stuff fails to load, like
          # `const_get': uninitialized constant YARD::Parser::Ruby::Legacy::RipperParser (NameError)
          # Did you mean?  YARD::Parser::Ruby::Legacy::RipperParser
        end

        object_id = Sorbet::Private::RealStdlib.real_object_id(nested_constant)
        maybe_seen_already = ret[object_id]
        if Object.eql?(mod) || !prefix
          nested_name = nested.to_s
        else
          nested_name = prefix + "::" + nested.to_s
        end
        if maybe_seen_already
          if nested_name != maybe_seen_already.primary_name
            maybe_seen_already.aliases << nested_name
            maybe_seen_already.owner = owner
          end
          if maybe_seen_already.primary_name.nil? && Sorbet::Private::RealStdlib.real_is_a?(nested_constant, Module)
            realName = Sorbet::Private::RealStdlib.real_name(nested_constant)
            maybe_seen_already.primary_name = realName
          end
        else
          entry = ConstantEntry.new(nested, nested_name, nil, [nested_name], nested_constant, owner)
          ret[object_id] = entry

          if Sorbet::Private::RealStdlib.real_is_a?(nested_constant, Module) && Object != nested_constant
            go_deeper << [entry, nested_constant, nested_name]
          end
        end

      end
    end

    # Horrible horrible hack to get private constants that are `include`d.
    # This doesn't recurse so you can't get private constants inside the private
    # constants, but I really hope you don't need those...
    Sorbet::Private::RealStdlib.real_ancestors(mod).each do |ancestor|
      object_id = Sorbet::Private::RealStdlib.real_object_id(ancestor)
      name = Sorbet::Private::RealStdlib.real_name(ancestor)
      next unless name
      next if ret[object_id]
      prefix, _, const_name = name.rpartition('::')
      entry = ConstantEntry.new(const_name.to_sym, name, name, [name], ancestor, nil)
      ret[object_id] = entry
    end
    sorted_deeper = go_deeper.sort_by do |entry, nested_constant, nested_name|
      Sorbet::Private::RealStdlib.real_name(nested_constant)
    end
    sorted_deeper.each do |entry, nested_constant, nested_name|
      dfs_module(nested_constant, nested_name, ret, entry)
    end
    nil
  end

  private def fill_primary_name(struct)
    return if struct.primary_name
    if !struct.owner
      struct.primary_name = struct.found_name
    else
      fill_primary_name(struct.owner)
      struct.primary_name = struct.owner.primary_name + "::" + struct.const_name.to_s
    end
  end
end
