# coding: utf-8
# frozen_string_literal: true
# typed: strict

require 'set'
require 'optparse'
require 'stringio'
require_relative '../../../gems/sorbet-runtime/lib/sorbet-runtime'

class Module
  include T::Sig
end

module Intrinsics
  class ExportStatus < T::Enum
    enums do
      Exported = new
      NotExported = new
      PatchExported = new
    end
  end

  class Unlimited; end
  class ArgRubyArray; end
  class FixedArity < T::Struct
    const :n, Integer
  end

  class Method < T::Struct
    prop :export_status, ExportStatus
    prop :file, String
    prop :klass, String
    prop :rb_name, String
    prop :c_name, String
    prop :argc, T.any(FixedArity, ArgRubyArray, Unlimited)

    sig {params(argc: Integer).returns(T.any(FixedArity, ArgRubyArray, Unlimited))}
    def self.arity_for_ruby_callconv(argc)
      case argc
      when -2
        return ArgRubyArray.new
      when -1
        return Unlimited.new
      else
        return FixedArity.new(n: argc)
      end
    end

    sig {returns(T::Boolean)}
    def already_exported
      export_status == ExportStatus::Exported
    end

    sig {returns(T::Boolean)}
    def did_export
      export_status == ExportStatus::PatchExported
    end

    sig {void}
    def mark_patch_exported
      case export_status
      when ExportStatus::NotExported
        @export_status = ExportStatus::PatchExported
      else
        T.absurd(self)
      end
    end

    sig {returns(T::Boolean)}
    def takes_ruby_array
      argc.is_a?(ArgRubyArray)
    end

    sig {returns(Integer)}
    def ruby_callconv
      case argc
      when ArgRubyArray
        -2
      when Unlimited
        -1
      when FixedArity
        argc.n
      end
    end

    sig {params(func_name: String).returns(String)}
    def signature(func_name)
      StringIO.open do |io|
        io << "VALUE #{func_name}("

        case argc
        when Unlimited
          io << 'int argc, const VALUE *args, VALUE obj'
        when ArgRubyArray
          $stderr.puts 'Need to implement calling convention -2'
          exit 1
        when FixedArity
          args = ['VALUE obj']

          argc.n.times do |i|
            args << "VALUE arg_#{i}"
          end

          io << args.join(', ')
        end

        io << ')'

        io.string
      end
    end

    sig {returns(String)}
    def function_signature
      signature(c_name)
    end

    sig {returns(String)}
    def exported_wrapper_signature
      signature(exported_c_name)
    end

    sig {returns(String)}
    def exported_c_name
      case export_status
      when ExportStatus::Exported
        c_name
      when ExportStatus::PatchExported
        "sorbet_#{c_name}"
      else
        T.absurd(self.c_name)
      end
    end

    sig {returns(String)}
    def sorbet_payload_wrapper_name
      return "sorbet_int_#{c_name}"
    end

    sig {returns(String)}
    def sorbet_payload_wrapper
      # NOTE: the sorbet calling convention differs from -1, in that we put the
      # receiver first, and pass the ID of the function explicitly.
      StringIO.open do |io|
        io << "VALUE #{sorbet_payload_wrapper_name}("
        io << 'VALUE recv, '
        io << 'ID fun, '
        io << 'int argc, '
        io << "VALUE *const restrict args, BlockFFIType blk, VALUE closure) {\n"

        case argc

        when Unlimited
          # dispatch directly to the underlying intrinsic
          io << "    return #{exported_c_name}(argc, args, recv);\n"

        when ArgRubyArray
          puts "Need to implement calling convention -2"
          exit 1

        when FixedArity
          # check the arity
          io << "    rb_check_arity(argc, #{argc.n}, #{argc.n});\n"

          # extract individual arguments
          args = ["recv"]
          argc.n.times do |i|
            args << "arg_#{i}"
            io << "    VALUE arg_#{i} = args[#{i}];\n"
          end

          # call the underlying intrinsic
          io << "    return #{exported_c_name}(#{args.join(', ')});\n"
        end

        io << "}\n"

        io.string
      end
    end
  end

  class Main

    SORBET_SYMBOL_REFS = T.let(Set[
      'Array',
      'BasicObject',
      'Class',
      'Complex',
      'Enumerable',
      'Enumerator',
      'FalseClass',
      'File',
      'Float',
      'Hash',
      'Integer',
      'Kernel',
      'Module',
      'NilClass',
      'Object',
      'Proc',
      'Range',
      'Rational',
      'Regexp',
      'Set',
      'Singleton',
      'StandardError',
      'String',
      'Struct',
      'Symbol',
      'TrueClass',
    ], T::Set[String])

    EXTERN_OVERRIDES = T.let({
      'rb_int_powm' => 'extern VALUE rb_int_powm(int const argc, VALUE * const argv, VALUE const num);',
      'rb_f_send' => 'extern VALUE rb_f_send(int argc, VALUE *argv, VALUE recv);',
    }, T::Hash[String, String])

    METHOD_WHITELIST = T.let(Set[
      "Array#+",
      "Array#<<",
      "Array#<=>",
      "Array#[]",
      "Array#assoc",
      "Array#at",
      "Array#clear",
      "Array#delete",
      "Array#include?",
      "Array#initialize_copy",
      "Array#last",
      "Array#rassoc",
      "Array#replace",
      "Array#slice",
      "Array#sort!",
      "Array#sort",
      "Float#*",
      "Float#**",
      "Float#+",
      "Float#-@",
      "Float#>",
      "Float#>=",
      "Float#<",
      "Float#<=",
      "Float#abs",
      "Float#finite?",
      "Float#infinite?",
      "Float#magnitude",
      "Hash#fetch",
      "Hash#has_key?",
      "Hash#include?",
      "Hash#key?",
      "Hash#member?",
      "Integer#%",
      "Integer#&",
      "Integer#*",
      "Integer#**",
      "Integer#+",
      "Integer#-",
      "Integer#-@",
      "Integer#/",
      "Integer#<<",
      "Integer#<=>",
      "Integer#==",
      "Integer#===",
      "Integer#>",
      "Integer#>=",
      "Integer#abs",
      "Integer#div",
      "Integer#divmod",
      "Integer#fdiv",
      "Integer#gcd",
      "Integer#gcdlcm",
      "Integer#lcm",
      "Integer#magnitude",
      "Integer#modulo",
      "Integer#odd?",
      "Integer#pow",
      "Regexp#encoding",
      "String#*",
      "String#+",
      "String#<<",
      "String#==",
      "String#===",
      "String#[]",
      "String#dump",
      "String#encoding",
      "String#eql?",
      "String#freeze",
      "String#initialize_copy",
      "String#inspect",
      "String#intern",
      "String#length",
      "String#next",
      "String#ord",
      "String#replace",
      "String#size",
      "String#slice",
      "String#succ",
      "String#to_sym",
    ], T::Set[String])

    sig {params(topdir: String, ruby: String, ruby_source: String).void}
    def self.run(topdir:, ruby:, ruby_source:)
      puts "ruby object: #{ruby}"
      puts "ruby source: #{ruby_source}"

      exported = exported_symbols(ruby: ruby)
      methods = methods_defined(ruby_source: ruby_source, exported: exported)

      expose_methods(topdir, ruby_source, methods)

      File.open('intrinsic-report.md', 'w') do |report|
        puts 'Writing intrinsic-report.md'
        write_report(report, methods)
      end

      # group methods together
      grouped_methods = methods.values.flatten.group_by(&:c_name).values

      # if there are multiple methods with the same underlying C implementation,
      # then they should all be specified as intrinsics.
      grouped_methods.each do |methods|
        next unless methods.any? {|method| method_whitelisted?(method)}
        unless methods.all? {|method| method_whitelisted?(method)}
          ruby_methods = methods.map {|method| "#{method.klass}##{method.rb_name}"}
          raise "Must specify all of #{ruby_methods} as intrinsics"
        end
      end

      # ensure consistent output order
      grouped_methods.sort_by! do |methods|
        method = methods.fetch(0)
        "#{method.klass}##{method.rb_name}"
      end

      File.open('WrappedIntrinsics.h', 'w') do |header|
        puts 'Writing WrappedIntrinsics.h'
        write_header(header, grouped_methods)
      end

      File.open('PayloadIntrinsics.c', 'w') do |wrapper|
        puts 'Writing PayloadIntrinsics.c'
        write_wrapper(wrapper, grouped_methods)
      end
    end

    # Collect a set of exported symbols from the ruby binary or shared object, using 'nm'
    sig {params(ruby: String).returns(T::Set[String])}
    def self.exported_symbols(ruby:)
      exported = Set.new

      if !File.exist?(ruby)
        puts "Ruby binary or shared object is missing: #{ruby}"
        exit 1
      end

      IO.popen([ 'nm', ruby ]) do |io|
        io.each_line do |line|
          line = T.let(line, String)
          if line =~ / T /

            # the line output format for nm is '<addr> <type> <name>'
            symbol = line.split(' ')[2]

            # There ends up being leading underscores in front of all the
            # symbols we care about on macOS
            exported << T.must(symbol).delete_prefix('_')
          end
        end
      end

      exported
    end

    # Collect methods defined by file
    sig {params(ruby_source: String, exported: T::Set[String]).returns(T::Hash[String, T::Array[Method]])}
    def self.methods_defined(ruby_source:, exported:)
      defined = T.let({}, T::Hash[String, T::Array[Method]])

      # change to the source dir to avoid absolute paths in the output
      Dir.chdir(ruby_source) do
        files = T.let([], T::Array[String])
        IO.popen(
          [
            'find', '.',
            '-name', '*.c',
            '-not', '-path', '*/spec/*',
            '-not', '-path', '*/ext/*',
            '-not', '-path', '*/gems/*',
          ]) do |io|
            io.each_line do |file|
              file = T.let(file, String)
              # trim the trailing newline, and the leading './'
              files << T.must(file.chomp[2..])
            end
        end

        files.sort!

        files.each do |file|
          methods = methods_from(exported: exported, file: file)

          if !methods.empty?
            defined[file] = methods
          end
        end
      end

      defined
    end

    RB_DEFINE_CLASS = /([^\s]+)\s+=\s*rb_define_class\("([^"]+)/
    RB_DEFINE_MODULE = /([^\s]+)\s+=\s*rb_define_module\("([^"]+)/
    RB_DEFINE_CLASS_UNDER = /([^\s]+)\s+=\s*rb_define_class_under\([^,]+,\s*"([^"]+)/
    RB_DEFINE_MODULE_UNDER = /([^\s]+)\s+=\s*rb_define_module_under\([^,]+,\s*"([^"]+)/

    RB_DEFINE_METHOD = /rb_define_method\(([^,]+),\s*\"([^,]+)\",([^,]+),(.*)/

    # Collect methods defined in a single file.
    sig {params(exported: T::Set[String], file: String).returns(T::Array[Method])}
    def self.methods_from(exported:, file:)
      methods = T.let([], T::Array[Method])

      File.open(file, 'r') do |io|

        # Seed the class list with cases that are defined outside of the source
        # file that defines its methods.
        klasses = T.let({
          "rb_cArray" => "Array",
          "rb_cBasicObject" => "BasicObject",
          "rb_cFile" => "File",
          "rb_cFloat" => "Float",
          "rb_cIO" => "IO",
          "rb_cInteger" => "Integer",
          "rb_cModule" => "Module",
          "rb_cNilClass" => "NilClass",
          "rb_cNumeric" => "Numeric",
          "rb_cRange" => "Range",
          "rb_cString" => "String",
          "rb_cThread" => "Thread",
          "rb_eInterrupt" => "Interrupt",
          "rb_eSignal" => "SignalException",
          "rb_mEnumerable" => "Enumerable",
          "rb_mKernel" => "Kernel",
        }, T::Hash[String, String])

        io.each_line do |line|

          # This is a heuristic, but Init functions usually define a class as a
          # symbol, and thread that through the method definitions.
          klass_match =
            RB_DEFINE_CLASS.match(line) ||
            RB_DEFINE_MODULE.match(line) ||
            RB_DEFINE_CLASS_UNDER.match(line) ||
            RB_DEFINE_MODULE_UNDER.match(line)

          if klass_match
            match_1 = T.must(klass_match[1])
            match_2 = T.must(klass_match[2])
            klasses[match_1.chomp] = match_2
          end

          if m = RB_DEFINE_METHOD.match(line)
            m1, m2, m3, m4 = T.must(m[1]), T.must(m[2]), T.must(m[3]), T.must(m[4])
            klass = m1.chomp

            # It would be nice to log a message here if the class can't be
            # resolved, but in most cases they're classes we're not interested
            # in.
            if klass_value = klasses[klass]
              rb_name = m2
              c_name = m3.lstrip.rstrip
              argc = m4.chomp.to_i

              status = if exported.include?(c_name)
                         ExportStatus::Exported
                       else
                         ExportStatus::NotExported
                       end
              methods << Method.new(
                export_status: status,
                file: file,
                klass: klass_value,
                rb_name: rb_name,
                c_name: c_name,
                argc: Method.arity_for_ruby_callconv(argc)
              )
            end
          end
        end
      end

      methods
    end

    sig {params(report: File, files: T::Hash[String, T::Array[Method]]).void}
    def self.write_report(report, files)
      total_intrinsics = 0
      visible_intrinsics = 0

      report << "# Intrinsics by file\n"
      report << "\n"

      files.each do |file,methods|
        report << "## `#{file}`\n"

        methods.each do |method|

          total_intrinsics += 1
          if method.already_exported || method.did_export
            visible_intrinsics += 1
          end

          already_exported = method.already_exported ? "✔" : " "
          did_export = method.did_export ? "✔" : " "

          report << "* [#{already_exported} #{did_export}] `#{method.c_name}` (`#{method.klass}##{method.rb_name}`)\n"
        end

        report << "\n"
      end

      report << "## Stats\n"
      report << "* Total:   #{total_intrinsics}\n"
      report << "* Visible: #{visible_intrinsics}\n"
    end

    sig do
      params(
        topdir: String,
        ruby_source: String,
        methods: T::Hash[String, T::Array[Method]]
      )
        .void
    end
    def self.expose_methods(topdir, ruby_source, methods)
      methods.each do |file,methods|
        ruby_file = File.join(ruby_source, file)
        hidden = methods.filter {|m| should_patch?(m)}
        if !hidden.empty?
          wrappers = File.open(ruby_file, 'r') do |io|
            nonstatic_wrappers_for_methods(topdir, hidden, io)
          end

          if !wrappers.empty?
            static_export_file = "#{topdir}/compiler/ruby-static-exports/#{file}"
            puts "Writing #{static_export_file}\n"
            File.open(static_export_file, 'w') do |io|
              io << wrappers.join("\n")
            end
          end
        end
      end
    end

    sig {params(topdir: String, hidden: T::Array[Method], io: File).returns(T::Array[String])}
    def self.nonstatic_wrappers_for_methods(topdir, hidden, io)
      to_export = T.let([], T::Array[Method])

      lines = io.each_line.to_a
      # Assume that we're never going to see a definition on the very first line.
      lines.each_cons(2).each_with_index do |window, idx|
        previous = window[0]
        line = window[1]
        # idx is 0-based, but the first line is actually line two.
        idx = idx + 2
        mi = hidden.find_index do |method|
          if !line.match?(Regexp.new("#{method.c_name}\\("))
            next false
          end

          if previous.match?('^static VALUE$')
            next true
          end

          next line.match?('static VALUE')
        end

        if !mi.nil?
          method = hidden[mi]
          to_export << method
          method.mark_patch_exported
        end
      end

      wrappers = T.let([], T::Array[String])

      to_export.each do |method|
         wrappers << StringIO.open do |strio|
          strio << method.exported_wrapper_signature
          strio << " {\n"
          case method.argc
          when ::Intrinsics::Unlimited
            strio << "    return #{method.c_name}(argc, args, obj);\n"
            strio << "}\n"

          when ::Intrinsics::ArgRubyArray
            puts "Need to implement calling convention -2"
            exit 1

          when ::Intrinsics::FixedArity
            args = ['obj']

            method.argc.n.times do |i|
              args << "arg_#{i}"
            end

            strio << "    return #{method.c_name}(#{args.join(', ')});\n"
            strio << "}\n"
          end

          strio.string
        end
      end

      wrappers
    end

    # Make this method return `true` to emit all methods
    sig {params(method: Method).returns(T::Boolean)}
    def self.method_whitelisted?(method)
      METHOD_WHITELIST.include?("#{method.klass}##{method.rb_name}")
    end

    sig {params(klass: String).returns(T::Boolean)}
    def self.have_symbol_ref?(klass)
      SORBET_SYMBOL_REFS.include?(klass)
    end

    sig {params(method: Method).returns(T::Boolean)}
    def self.should_patch?(method)
      !method.already_exported && \
        have_symbol_ref?(method.klass) && \
        method_whitelisted?(method) && \
        !method.takes_ruby_array
    end

    sig {params(method: Method).returns(T.nilable(T::Boolean))}
    def self.should_wrap?(method)
      (method.already_exported || method.did_export) && \
        have_symbol_ref?(method.klass) && \
        method_whitelisted?(method) && \
        !method.takes_ruby_array
    end

    sig {params(header: File, grouped_methods: T::Array[T::Array[Method]]).void}
    def self.write_header(header, grouped_methods)
      header << "// This file is autogenerated. Do not edit it by hand. Regenerate it with:\n"
      header << "//   cd compiler/IREmitter/Intrinsics && make\n"
      header << "\n"
      header << "// clang-format off\n"

      grouped_methods.each do |methods|
        method = methods.fetch(0)
        if should_wrap?(method)
          methods.each do |method|
            header << "    {core::Symbols::#{method.klass}(), "
            header << "\"#{method.rb_name}\", "
            header << "CMethod{\"#{method.sorbet_payload_wrapper_name}\"}},\n"
          end
        end
      end
      header << "    // clang-format on\n"
    end

    sig {params(wrapper: File, grouped_methods: T::Array[T::Array[Method]]).void}
    def self.write_wrapper(wrapper, grouped_methods)

      wrapper << <<~EOF
      #ifndef SORBET_COMPILER_IMPORTED_INTRINSICS_H
      #define SORBET_COMPILER_IMPORTED_INTRINSICS_H

      // This file is autogenerated. Do not edit it by hand. Regenerate it with:
      //   cd compiler/IREmitter/Intrinsics && make

      #include "ruby.h"

      typedef VALUE (*BlockFFIType)(VALUE firstYieldedArg, VALUE closure, int argCount, const VALUE *args, VALUE blockArg);

      EOF

      grouped_methods.each do |methods|
        method = methods.fetch(0)
        if should_wrap?(method)

          # TODO: comment about the ruby method
          wrapper << "\n"
          forward_declare_intrinsic(wrapper, method, methods)
          wrapper << "\n"
          wrapper_implementation(wrapper, method)
        end
      end

      wrapper << "#endif /* SORBET_COMPILER_IMPORTED_INTRINSICS_H */\n"
    end

    sig {params(wrapper: File, method: Method, methods: T::Array[Method]).void}
    def self.forward_declare_intrinsic(wrapper, method, methods)

      methods.each do |m|
        wrapper  << "// #{m.klass}##{m.rb_name}\n"
      end
      wrapper << "// Calling convention: #{method.ruby_callconv}\n"

      extern_override = EXTERN_OVERRIDES[method.c_name]
      if extern_override
        wrapper << extern_override
        wrapper << "\n"
        return
      end

      wrapper << "extern #{method.exported_wrapper_signature};\n"
    end

    sig {params(wrapper: File, method: Method).void}
    def self.wrapper_implementation(wrapper, method)
      wrapper << method.sorbet_payload_wrapper
    end

  end
end

if __FILE__ == $0

  topdir = File.dirname($0) + '/../../..'

  if /darwin/ =~ RUBY_PLATFORM
    ruby = topdir + '/bazel-bin/external/sorbet_ruby_2_7_unpatched/toolchain/lib/libruby.2.7.dylib'
  else
    ruby = topdir + '/bazel-bin/external/sorbet_ruby_2_7_unpatched/toolchain/lib/libruby.so.2.7'
  end

  ruby_source = topdir + '/bazel-sorbet/external/sorbet_ruby_2_7_unpatched'

  OptionParser.new do |opts|

    opts.banner = "Usage: wrap-intrinsics.rb [options]"

    opts.on '-rPATH', '--ruby=PATH', 'Path to the ruby executable or shared object to analyze' do |path|
      ruby = File.realpath(path)
    end

    opts.on '-sPATH', '--ruby-source=PATH', 'Path to the ruby source to analyze' do |path|
      ruby_source = File.realpath(path)
    end

    opts.on '-h', '--help', 'Display this message' do
      puts opts
      exit
    end

  end.parse!

  Intrinsics::Main.run(topdir: topdir, ruby: ruby, ruby_source: ruby_source)
end
