#!/usr/bin/env ruby
# frozen_string_literal: true
# typed: true

class Sorbet; end
module Sorbet::Private; end

require_relative './t'
require_relative './step_interface'
require_relative './serialize'
require_relative './constant_cache'
require_relative './require_everything'
require_relative './real_stdlib'

require 'fileutils'
require 'json'
require 'set'
require 'tmpdir'

class Sorbet::Private::HiddenMethodFinder
  PATH = "sorbet/rbi/hidden-definitions/"
  TMP_PATH = Dir.mktmpdir + "/"
  TMP_RBI = TMP_PATH + "reflection.rbi"
  DIFF_RBI = TMP_PATH + "hidden.rbi.tmp"
  RBI_CONSTANTS = TMP_PATH + "reflection.json"
  RBI_CONSTANTS_ERR = RBI_CONSTANTS + ".err"
  SOURCE_CONSTANTS = TMP_PATH + "from-source.json"
  SOURCE_CONSTANTS_ERR = SOURCE_CONSTANTS + ".err"

  HIDDEN_RBI = PATH + "hidden.rbi"
  ERRORS_RBI = PATH + "errors.txt"

  HEADER = Sorbet::Private::Serialize.header('autogenerated', 'hidden-definitions')

  include Sorbet::Private::StepInterface

  def self.main
    self.new.main
  end

  def main
    mk_dir
    require_everything
    classes, aliases = all_modules_and_aliases
    gen_source_rbi(classes, aliases)
    rm_rbis
    write_constants
    source, rbi = read_constants
    write_diff(source, rbi)
    split_rbi
    rm_dir
  end

  def mk_dir
    FileUtils.mkdir_p(PATH) unless Dir.exist?(PATH)
  end

  def rm_dir
    FileUtils.rm_r(TMP_PATH)
  end

  def require_everything
    puts "Requiring all of your code"
    Sorbet::Private::RequireEverything.require_everything
  end

  def constant_cache
    @cache ||= Sorbet::Private::ConstantLookupCache.new
    @cache
  end


  def all_modules_and_aliases
    puts "Naming all Modules"
    [constant_cache.all_module_names.sort, constant_cache.all_module_aliases]
  end

  def real_name(mod)
    constant_cache.name_by_class(mod)
  end

  def gen_source_rbi(classes, aliases)
    puts "Generating #{TMP_RBI} with #{classes.count} modules and #{aliases.count} aliases"
    serializer = Sorbet::Private::Serialize.new(constant_cache)
    buffer = []
    buffer << Sorbet::Private::Serialize.header

    # should we do something with these errors?
    capture_stderr do
      classes.each do |class_name|
        buffer << serializer.class_or_module(class_name)
      end
      aliases.each do |base, other_names|
        other_names.each do |other_name|
          buffer << serializer.alias(base, other_name)
        end
      end
    end
    File.write(TMP_RBI, buffer.join("\n"))
  end

  def write_constants
    puts "Printing your code's symbol table into #{SOURCE_CONSTANTS}"
    io = IO.popen(
      [
        File.realpath("#{__dir__}/../bin/srb"),
        'tc',
        '--print=symbol-table-full-json',
        '--stdout-hup-hack',
        '--silence-dev-message',
        '--no-error-count',
        '-e', # this is additive with any files / dirs
        '""',
      ],
      err: SOURCE_CONSTANTS_ERR
    )
    File.write(SOURCE_CONSTANTS, io.read)
    io.close
    raise "Your source can't be read by Sorbet.\nYou can try `find . -type f | xargs -L 1 -t bundle exec srb tc --no-config --isolate-error-code 1000` and hopefully the last file it is processing before it dies is the culprit.\nIf not, maybe the errors in this file will help: #{SOURCE_CONSTANTS_ERR}" if File.read(SOURCE_CONSTANTS).empty?

    puts "Printing #{TMP_RBI}'s symbol table into #{RBI_CONSTANTS}"
    io = IO.popen(
      [
        {'SRB_SKIP_GEM_RBIS' => 'true'},
        File.realpath("#{__dir__}/../bin/srb"),
        'tc',
        # Make sure we don't load a sorbet/config in your cwd
        '--no-config',
        '--print=symbol-table-full-json',
        # The hidden-definition serializer is not smart enough to put T::Enum
        # constants it discovers inside an `enums do` block. We probably want
        # to come up with a better long term solution here.
        '--suppress-error-code=3506',
        # Method redefined with mismatched argument is ok since sometime
        # people monkeypatch over method
        '--suppress-error-code=4010',
        # Redefining constant is needed because we serialize things both as
        # aliases and in-class constants.
        '--suppress-error-code=4012',
        # Invalid nesting is ok because we don't generate all the intermediate
        # namespaces for aliases
        '--suppress-error-code=4015',
        # The `Random` class has a super class of `Object` in ruby-2.7 and
        # `Random::Base` in ruby-2. We can remove this once we no longer support
        # ruby-2.7.
        '--suppress-error-code=5012',
        '--stdout-hup-hack',
        '--silence-dev-message',
        '--no-error-count',
        TMP_RBI,
      ],
      err: RBI_CONSTANTS_ERR
    )
    File.write(RBI_CONSTANTS, io.read)
    io.close
    raise "#{TMP_RBI} had unexpected errors. Check this file for a clue: #{RBI_CONSTANTS_ERR}" unless $?.success?
  end

  def read_constants
    puts "Reading #{SOURCE_CONSTANTS}"
    source = JSON.parse(File.read(SOURCE_CONSTANTS))
    puts "Reading #{RBI_CONSTANTS}"
    rbi = JSON.parse(File.read(RBI_CONSTANTS))
    [source, rbi]
  end

  def write_diff(source, rbi)
    puts "Building rbi id to symbol map"
    rbi_symbols = symbols_id_to_name(rbi, '')
    puts "Building source id to symbol map"
    source_symbols = symbols_id_to_name(source, '')
    puts "Writing #{DIFF_RBI}"
    diff = serialize_constants(
      source.fetch("children", []),
      rbi.fetch("children", []),
      Object, false, source_symbols, rbi_symbols)
    File.write(DIFF_RBI, diff)
  end

  def symbols_id_to_name(entry, prefix)
    ret = {}
    symbols_id_to_name_real(entry, prefix, ret)
    ret
  end

  private def symbols_id_to_name_real(entry, prefix, ret)
    name = entry["name"]["name"]
    if prefix == '' || prefix == "<root>"
      fqn = name.to_s
    else
      fqn = "#{prefix}::#{name}"
    end

    ret[entry["id"]] = fqn
    entry.fetch("children", []).each do |child|
      symbols_id_to_name_real(child, fqn, ret)
    end
  end

  def serialize_constants(source, rbi, klass, is_singleton, source_symbols, rbi_symbols)
    source_by_name = source.map {|v| [v["name"]["name"], v]}.to_h
    ret = []

    rbi.each do |rbi_entry|
      # skip duplicated constant fields
      next if rbi_entry["name"]["kind"] == "UNIQUE" and rbi_entry["name"]["unique"] == "MANGLE_RENAME"

      source_entry = source_by_name[rbi_entry["name"]["name"]]

      ret << serialize_alias(source_entry, rbi_entry, klass, source_symbols, rbi_symbols)
      ret << serialize_class(source_entry, rbi_entry, klass, source_symbols, rbi_symbols, source_by_name)
    end

    ret.compact.join("\n")
  end

  def serialize_class(source_entry, rbi_entry, klass, source_symbols, rbi_symbols, source_by_name)
    return if rbi_entry["kind"] != "CLASS_OR_MODULE"

    name = rbi_entry["name"]["name"]
    if name.start_with?('<Class:')
      name = name.sub('<Class:', '').sub('>', '')
      my_klass_is_singleton = true
    else
      my_klass_is_singleton = false
    end
    begin
      my_klass = klass.const_get(name, false) # rubocop:disable PrisonGuard/NoDynamicConstAccess
    rescue LoadError, NameError, ArgumentError => e
      return "# #{e.message.gsub("\n", "\n# ")}"
    end

    return if !Sorbet::Private::RealStdlib.real_is_a?(my_klass, Class) && !Sorbet::Private::RealStdlib.real_is_a?(my_klass, Module)

    # We specifically don't typecheck anything in T:: since it is hardcoded
    # into sorbet. We don't include anything in Sorbet::Private:: because
    # it's private.
    return if ['T', 'Sorbet::Private'].include?(real_name(my_klass))

    source_type = nil
    if !source_entry
      if source_by_name[name]
        source_type = source_by_name[name]["kind"]
      end
    else
      source_type = source_entry["kind"]
    end
    if source_type && source_type != "CLASS_OR_MODULE"
      return "# The source says #{real_name(my_klass)} is a #{source_type} but reflection says it is a #{rbi_entry['kind']}"
    end

    if !source_entry
      source_children = []
      source_mixins = []
      is_stub = true
    else
      source_children = source_entry.fetch("children", [])
      source_mixins = source_entry.fetch("mixins", [])
      is_stub = source_entry['superClass'] && source_symbols[source_entry['superClass']] == 'Sorbet::Private::Static::StubModule'
    end
    rbi_children = rbi_entry.fetch("children", [])
    rbi_mixins = rbi_entry.fetch("mixins", [])

    methods = serialize_methods(source_children, rbi_children, my_klass, my_klass_is_singleton)
    includes = serialize_includes(source_mixins, rbi_mixins, my_klass, my_klass_is_singleton, source_symbols, rbi_symbols)
    values = serialize_values(source_children, rbi_children, my_klass, source_symbols)

    ret = []
    if !without_errors(methods).empty? || !without_errors(includes).empty? || !without_errors(values).empty? || is_stub
      fqn = real_name(my_klass)
      if fqn
        klass_str = String.new
        klass_str << (Sorbet::Private::RealStdlib.real_is_a?(my_klass, Class) ? "class #{fqn}\n" : "module #{fqn}\n")
        klass_str << includes.join("\n")
        klass_str << "\n" unless klass_str.end_with?("\n")
        klass_str << methods.join("\n")
        klass_str << "\n" unless klass_str.end_with?("\n")
        klass_str << values.join("\n")
        klass_str << "\n" unless klass_str.end_with?("\n")
        klass_str << "end\n"
        ret << klass_str
      end
    end

    children = serialize_constants(source_children, rbi_children, my_klass, my_klass_is_singleton, source_symbols, rbi_symbols)
    if children != ""
      ret << children
    end

    ret.empty? ? nil : ret.join("\n")
  end

  private def without_errors(lines)
    lines.reject {|line| line.start_with?("#")}
  end

  def serialize_alias(source_entry, rbi_entry, my_klass, source_symbols, rbi_symbols)
    return if rbi_entry["kind"] != "STATIC_FIELD"
    return if source_entry == rbi_entry
    if source_entry
      is_stub = source_entry['superClass'] && source_symbols[source_entry['superClass']] == 'Sorbet::Private::Static::StubModule'
      if !is_stub
        return
      end
    end
    return if !rbi_entry["aliasTo"]

    fqn = rbi_symbols[rbi_entry["id"]]
    other_fqn = rbi_symbols[rbi_entry["aliasTo"]]
    return if looks_like_stub_name(fqn)
    ret = String.new
    ret << "#{fqn} = #{other_fqn}\n"
    return ret
  end

  def looks_like_stub_name(name)
    name.include?('$')
  end

  private def serialize_values(source, rbi, klass, source_symbols)
    source_by_name = source.map {|v| [v["name"]["name"], v]}.to_h
    ret = []
    rbi.each do |rbi_entry|
      name = rbi_entry["name"]["name"]
      source_entry = source_by_name[name]
      if source_entry
        is_stub = source_entry['superClass'] && source_symbols[source_entry['superClass']] == 'Sorbet::Private::Static::StubModule'
        next unless is_stub
      end
      next if Sorbet::Private::ConstantLookupCache::DEPRECATED_CONSTANTS.include?("#{Sorbet::Private::RealStdlib.real_name(klass)}::#{name}")
      begin
        my_value = klass.const_get(name, false) # rubocop:disable PrisonGuard/NoDynamicConstAccess
      rescue StandardError, LoadError => e
        ret << "# #{e.message.gsub("\n", "\n# ")}"
        next
      end
      next if Sorbet::Private::RealStdlib.real_is_a?(my_value, Class) || Sorbet::Private::RealStdlib.real_is_a?(my_value, Module)
      if defined?(T::Types::TypeMember) && Sorbet::Private::RealStdlib.real_is_a?(my_value, T::Types::TypeMember)
        ret << (my_value.variance == :invariant ? "  #{name} = type_member" : "  #{name} = type_member(#{my_value.variance.inspect})")
      elsif defined?(T::Types::TypeTemplate) && Sorbet::Private::RealStdlib.real_is_a?(my_value, T::Types::TypeTemplate)
        ret << (my_value.variance == :invariant ? "  #{name} = type_template" : "  #{name} = type_template(#{my_value.variance.inspect})")
      else
        ret << "  #{name} = ::T.let(nil, ::T.untyped)"
      end
    end
    ret
  end

  # These methods are defined in C++ and we want our C++ definition to
  # win instead of a shim.
  DENYLIST = Set.new([
    [Class.object_id, "new"],
    [BasicObject.object_id, "initialize"],
  ]).freeze

  private def serialize_methods(source, rbi, klass, is_singleton)
    source_by_name = source.map {|v| [v["name"]["name"], v]}.to_h
    ret = []
    maker = Sorbet::Private::Serialize.new(constant_cache)
    rbi.each do |rbi_entry|
      next if rbi_entry["kind"] != "METHOD"
      name = rbi_entry["name"]["name"]
      next if source_by_name[name]

      next if DENYLIST.include?([klass.object_id, name])
      next if name.start_with?('<') && name.end_with?('>')

      begin
        if is_singleton
          method = klass.singleton_method(name)
        else
          method = klass.instance_method(name)
        end
      rescue => e
        ret << "# #{e.message.gsub("\n", "\n# ")}"
        next
      end

      errors = capture_stderr do
        ret << maker.serialize_method(method, is_singleton, with_sig: false)
      end
      errors.split("\n").each do |line|
        ret << "# #{line}"
      end
    end

    ret
  end

  private def serialize_includes(source, rbi, klass, is_singleton, source_symbols, rbi_symbols)
    ret = []
    source_mixins = source.map {|id| source_symbols[id]}
    rbi_mixins = rbi.map {|id| rbi_symbols[id]}
    rbi_mixins.each do |rbi_mixin|
      if !source_mixins.include?(rbi_mixin)
        keyword = is_singleton ? "extend" : "include"
        ret << "  #{keyword} ::#{rbi_mixin}"
      end
    end
    ret
  end

  def capture_stderr
    real_stderr = $stderr
    $stderr = StringIO.new
    yield
    $stderr.string
  ensure
    $stderr = real_stderr
  end

  private def rm_rbis
    File.delete(HIDDEN_RBI) if File.exist?(HIDDEN_RBI)
    File.delete(ERRORS_RBI) if File.exist?(ERRORS_RBI)
  end

  private def split_rbi
    puts "Generating split RBIs into #{PATH}"
    output = {
      hidden: String.new,
      errors: String.new,
    }

    valid = File.read(DIFF_RBI)
    cur_output = T.let(nil, T.untyped)

    valid.split("\n").each do |line|
      category = categorize(line)
      if category == :errors
        # Don't ever switch to errors output permanantly
        output[category] << line + "\n"
        next
      end
      if !category.nil?
        cur_output = output[category]
      end
      cur_output << line + "\n"
    end

    File.write(HIDDEN_RBI, HEADER + "\n" + output[:hidden])
    File.write(ERRORS_RBI, HEADER + "\n" + output[:errors])
  end

  private def categorize(line)
    if line.start_with?('#')
      return :errors
    end
    return :hidden
  end

  def self.output_file
    PATH
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::HiddenMethodFinder.main
end
