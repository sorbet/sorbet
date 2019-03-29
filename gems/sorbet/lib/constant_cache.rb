# frozen_string_literal: true
# typed: true

# This class walks global namespace to find all modules and discover all of their names.
# At the time you ask for it it, it takes a "spashot" of the world.
# If new modules were defined after an instance of this class was created,
# they won't be visible through a previously returned instance.
class SorbetRBIGeneration::ConstantLookupCache

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
    'ERB::Compiler::SimpleScanner2',
    'Net::OpenSSL',
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
  ].freeze

  def initialize
    @all_constants = {}
    dfs_module(Object, nil, @all_constants, nil)
    @consts_by_name = {}
    @all_constants.each_value do |struct|
      fill_primary_name(struct)
      @consts_by_name[struct.primary_name] = struct.const
    end
  end

  def all_module_names
    ret = @all_constants.select {|_k, v| real_is_a?(v.const, Module)}.map do |_key, struct|
      raise "should never happen" if !struct.primary_name
      struct.primary_name
    end
    ret
  end

  def all_named_modules
    ret = @all_constants.select {|_k, v| real_is_a?(v.const, Module)}.map do |_key, struct|
      raise "should never happen" if !struct.primary_name
      struct.const
    end
    ret
  end

  def all_module_aliases
    ret = {}

    @all_constants.map do |_key, struct|
      next if struct.nil? || !real_is_a?(struct.const, Module) || struct.aliases.size < 2
      ret[struct.primary_name] = struct.aliases.reject {|name| name == struct.primary_name}
    end
    ret
  end

  def class_by_name(name)
    @consts_by_name[name]
  end

  def name_by_class(klass)
    @all_constants[get_object_id(klass)]&.primary_name
  end

  private def get_constants(mod)
    @real_constants ||= Module.instance_method(:constants)
    @real_constants.bind(mod).call(false)
  end

  private def get_object_id(o)
    @real_object_id ||= Object.instance_method(:object_id)
    @real_object_id.bind(o).call
  end

  private def real_is_a?(o, klass)
    @real_is_a ||= Object.instance_method(:is_a?)
    @real_is_a.bind(o).call(klass)
  end

  private def real_name(o)
    @real_name ||= Module.instance_method(:name)
    @real_name.bind(o).call
  end

  private def dfs_module(mod, prefix, ret, owner)
    raise "error #{prefix}" if !mod.is_a?(Module)
    begin
      constants = get_constants(mod)
    rescue TypeError
      puts "Failed to call `constants` on #{prefix}"
      return nil
    end
    go_deeper = [] # make this a bfs to prefer shorter names
    constants.each do |nested|
      begin
        next if mod.autoload?(nested) # some constants aren't autoloaded even after require_everything, e.g. StateMachine::Graph

        begin
          next if DEPRECATED_CONSTANTS.include?("#{prefix}::#{nested}") # avoid stdout spew
        rescue NameError
          # If it isn't to_s-able, that is ok, it won't be in our deprecated list
          nil
        end

        begin
          nested_constant = mod.const_get(nested, false) # rubocop:disable PrisonGuard/NoDynamicConstAccess
        rescue LoadError
          puts "Failed to load #{mod.name}::#{nested}"
          next
        rescue NameError
          puts "Failed to load #{mod.name}::#{nested}"
          # some stuff fails to load, like
          # `const_get': uninitialized constant YARD::Parser::Ruby::Legacy::RipperParser (NameError)
          # Did you mean?  YARD::Parser::Ruby::Legacy::RipperParser
        end
        object_id = get_object_id(nested_constant)
        maybe_seen_already = ret[object_id]
        if Object.eql?(mod) || !prefix
          nested_name = nested.to_s
        else
          nested_name = prefix + "::" + nested.to_s
        end
        if maybe_seen_already
          if nested_name != maybe_seen_already.primary_name
            maybe_seen_already.aliases << nested_name
          end
          if maybe_seen_already.primary_name.nil? && real_is_a?(nested_constant, Module)
            realName = real_name(nested_constant)
            maybe_seen_already.primary_name = realName
          end
        else
          entry = ConstantEntry.new(nested, nested_name, nil, [nested_name], nested_constant, owner)
          ret[object_id] = entry

          if real_is_a?(nested_constant, Module) && Object != nested_constant
            go_deeper << [entry, nested_constant, nested_name]
          end
        end

      end
    end
    go_deeper.each do |entry, nested_constant, nested_name|
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
