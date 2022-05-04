# frozen_string_literal: true
# typed: true

require_relative '../serialize'
require_relative '../real_stdlib'

require 'set'
require 'fileutils'
require 'delegate'


module Sorbet::Private
  module GemGeneratorTracepoint
    ClassDefinition = Struct.new(:id, :klass, :defs)

    class TracepointSerializer
      SPECIAL_METHOD_NAMES = %w[! ~ +@ ** -@ * / % + - << >> & | ^ < <= => > >= == === != =~ !~ <=> [] []= `]

      BAD_METHODS = [
        # These methods don't match the signatures of their parents, so if we let
        # them monkeypatch, they won't be subtypes anymore. Just don't support the
        # bad monkeypatches.
        ['activesupport', 'Time', :to_s],
        ['activesupport', 'Time', :initialize],

        # These methods cause TracepointSerializer to hang the Ruby process when
        # running Ruby 2.3. See https://github.com/sorbet/sorbet/issues/1145
        ['activesupport', 'ActiveSupport::Deprecation', :new],
        ['activesupport', 'ActiveSupport::Deprecation', :allocate],
      ]

      HEADER = Sorbet::Private::Serialize.header('true', 'gems')

      T::Sig::WithoutRuntime.sig {params(files: T::Hash, delegate_classes: T::Hash).void}
      def initialize(files:, delegate_classes:)
        @files = files
        @delegate_classes = delegate_classes

        @anonymous_map = {}
        @prev_anonymous_id = 0
      end

      T::Sig::WithoutRuntime.sig {params(output_dir: String).void}
      def serialize(output_dir)
        gem_class_defs = preprocess(@files)

        FileUtils.mkdir_p(output_dir) unless gem_class_defs.empty?

        gem_class_defs.each do |gem, klass_ids|
          File.open("#{File.join(output_dir, gem[:gem])}.rbi", 'w') do |f|
            f.write(HEADER)
            f.write("#
# If you would like to make changes to this file, great! Please create the gem's shim here:
#
#   https://github.com/sorbet/sorbet-typed/new/master?filename=lib/#{gem[:gem]}/all/#{gem[:gem]}.rbi
#
")
            f.write("# #{gem[:gem]}-#{gem[:version]}\n\n")
            klass_ids.each do |klass_id, class_def|
              klass = class_def.klass

              f.write("#{Sorbet::Private::RealStdlib.real_is_a?(klass, Class) ? 'class' : 'module'} #{class_name(klass)}")
              f.write(" < #{class_name(klass.superclass)}") if Sorbet::Private::RealStdlib.real_is_a?(klass, Class) && ![Object, nil].include?(klass.superclass)
              f.write("\n")

              rows = class_def.defs.map do |item|
                case item[:type]
                when :method
                  if !valid_method_name?(item[:method])
                    # warn("Invalid method name: #{klass}.#{item[:method]}")
                    next
                  end
                  if BAD_METHODS.include?([gem[:gem], class_name(klass), item[:method]])
                    next
                  end
                  begin
                    method = item[:singleton] ? Sorbet::Private::RealStdlib.real_method(klass, item[:method]) : klass.instance_method(item[:method])

                    "#{generate_method(method, !item[:singleton])}"
                  rescue NameError
                  end
                when :include, :extend
                  name = class_name(item[item[:type]])
                  "  #{item[:type]} #{name}"
                end
              end
              rows = rows.compact.sort
              f.write(rows.join("\n"))
              f.write("\n") if !rows.empty?
              f.write("end\n")
            end
          end
        end
      end

      private

      def preprocess(files)
        gem_class_defs = files_to_gem_class_defs(files)
        filter_unused(gem_class_defs)
      end

      def files_to_gem_class_defs(files)
        # Transform tracer output into hash of gems to class definitions
        files.each_with_object({}) do |(path, defined), gem_class_defs|
          gem = gem_from_location(path)
          if gem.nil?
            warn("Can't find gem for #{path}") unless path.start_with?(Dir.pwd)
            next
          end
          next if gem[:gem] == 'ruby'
          # We're currently ignoring bundler, because we can't easily pin
          # everyone to the same version of bundler in tests and in CI.
          # There is an RBI for bundler in sorbet-typed.
          next if gem[:gem] == 'bundler'
          # We ignore sorbet-runtime because because we write the RBI for it into our payload.
          # For some reason, runtime reflection generates methods with incorrect arities.
          next if gem[:gem] == 'sorbet-runtime'

          gem_class_defs[gem] ||= {}
          defined.each do |item|
            klass = item[:module]
            klass_id = Sorbet::Private::RealStdlib.real_object_id(klass)
            class_def = gem_class_defs[gem][klass_id] ||= ClassDefinition.new(klass_id, klass, [])
            class_def.defs << item unless item[:type] == :module
          end
        end
      end

      def filter_unused(gem_class_defs)
        used = detect_used(gem_class_defs)

        gem_class_defs.each_with_object({}) do |(gem, klass_defs), hsh|
          hsh[gem] = klass_defs.select do |klass_id, klass_def|
            klass = klass_def.klass

            # Unused anon classes
            next if !((Sorbet::Private::RealStdlib.real_is_a?(klass, Module) && !Sorbet::Private::RealStdlib.real_name(klass).nil?) || used?(klass_id, used))

            # Anon delegate classes
            next if Sorbet::Private::RealStdlib.real_is_a?(klass, Class) && klass.superclass == Delegator && !klass.name

            # TODO should this be here?
            # next if [Object, BasicObject, Hash].include?(klass)
            true
          end
        end
      end

      def detect_used(gem_class_defs)
        # subclassed, included, or extended
        used = {}

        gem_class_defs.each do |gem, klass_ids|
          klass_ids.each do |klass_id, class_def|
            klass = class_def.klass

            # only add an anon module if it's used as a superclass of a non-anon module, or is included/extended by a non-anon module
            used_value = Sorbet::Private::RealStdlib.real_is_a?(klass, Module) && !Sorbet::Private::RealStdlib.real_name(klass).nil? ? true : Sorbet::Private::RealStdlib.real_object_id(klass) # if non-anon, set it to true
            (used[Sorbet::Private::RealStdlib.real_object_id(klass.superclass)] ||= Set.new) << used_value if Sorbet::Private::RealStdlib.real_is_a?(klass, Class)
            # otherwise link to next anon class
            class_def.defs.each do |item|
              (used[item[item[:type]].object_id] ||= Set.new) << used_value if [:extend, :include].include?(item[:type])
            end
          end
        end

        used
      end

      def used?(klass, used)
        used_by = used[klass] || []
        used_by.any? { |user| user == true || used?(user, used) }
      end

      def generate_method(method, instance, spaces = 2)
        # method.parameters is an array of:
        # a      [:req, :a]
        # b = 1  [:opt, :b]
        # c:     [:keyreq, :c]
        # d: 1   [:key, :d]
        # *e     [:rest, :e]
        # **f    [:keyrest, :f]
        # &g     [:block, :g]
        prefix = ' ' * spaces
        parameters = method.parameters.map.with_index do |(type, name), index|
          name = "arg#{index}" if name.nil? || name.empty?
          case type
          when :req
            name
          when :opt
            "#{name} = nil"
          when :keyreq
            "#{name}:"
          when :key
            "#{name}: nil"
          when :rest
            case name
            when :* then "*"
            else "*#{name}"
            end
          when :keyrest
            case name
            when :** then "**"
            else "**#{name}"
            end
          when :block
            case name
            when :& then "&"
            else "&#{name}"
            end
          else
            raise "Unknown parameter type: #{type}"
          end
        end
        parameters = parameters.join(', ')
        parameters = "(#{parameters})" unless parameters.empty?
        "#{prefix}def #{instance ? '' : 'self.'}#{method.name}#{parameters}; end"
      end

      def anonymous_id
        @prev_anonymous_id += 1
      end

      def gem_from_location(location)
        match =
          location&.match(/^.*\/(?:gems\/(?:(?:j?ruby-)?[\d.]+(?:@[^\/]+)?(?:\/bundler)?\/)?|ruby\/[\d.]+\/)gems\/([^\/]+)-([^-\/]+)\//i) || # gem
          location&.match(/^.*\/(ruby)\/([\d.]+)\//) || # ruby stdlib
          location&.match(/^.*\/(jruby)-([\d.]+)\//) || # jvm ruby stdlib
          location&.match(/^.*\/(site_ruby)\/([\d.]+)\//) # rubygems
        if match.nil?
          match_via_bundler_specs(location)
        else
          {
            path: match[0],
            gem: match[1],
            version: match[2],
          }
        end
      end

      def class_name(klass)
        klass = @delegate_classes[Sorbet::Private::RealStdlib.real_object_id(klass)] || klass
        name = Sorbet::Private::RealStdlib.real_name(klass) if Sorbet::Private::RealStdlib.real_is_a?(klass, Module)

        # class/module has no name; it must be anonymous
        if name.nil? || name == ""
          middle = Sorbet::Private::RealStdlib.real_is_a?(klass, Class) ? klass.superclass : klass.class
          id = @anonymous_map[Sorbet::Private::RealStdlib.real_object_id(klass)] ||= anonymous_id
          return "Anonymous_#{class_name(middle).gsub('::', '_')}_#{id}"
        end

        # if the name doesn't only contain word characters and ':', or any part doesn't start with a capital, Sorbet doesn't support it
        if name !~ /^[\w:]+$/ || !name.split('::').all? { |part| part =~ /^[A-Z]/ }
          # warn("Invalid class name: #{name}")
          id = @anonymous_map[Sorbet::Private::RealStdlib.real_object_id(klass)] ||= anonymous_id
          return "InvalidName_#{name.gsub(/[^\w]/, '_').gsub(/0x([0-9a-f]+)/, '0x00')}_#{id}"
        end

        name
      end

      def valid_method_name?(symbol)
        string = symbol.to_s
        return true if SPECIAL_METHOD_NAMES.include?(string)
        string =~ /^[[:word:]]+[?!=]?$/
      end

      def match_via_bundler_specs(location)
        @bundler_specs ||= begin
          require 'bundler'
          begin
            Bundler.load.specs.map do |spec|
              spec.load_paths.map do |path|
                [path, [spec.name, spec.version.to_s]]
              end
            end.flatten(1).to_h
          rescue Bundler::BundlerError # bail out on any bundler error
            {}
          end
        rescue LoadError # bundler can't be loaded, abort!
          {}
        end

        path_to_find = Pathname.new(location)
        parent_path, (gem_name, gem_version) = @bundler_specs.detect do |path, _gem|
          path_to_find.fnmatch?(File.join(path, '**'))
        end

        if parent_path.nil? || gem_name.nil? || gem_version.nil?
          # uncomment to generate files for methods outside of gems
          # {
          #   path: location,
          #   gem: location.gsub(/[\/\.]/, '_'),
          #   version: '1.0.0',
          # }
          nil
        else
          {
            path: location,
            gem: gem_name,
            version: gem_version,
          }
        end
      end
    end
  end
end
