# frozen_string_literal: true
# typed: true

module T::Props
  # Helper to validate generated code, to assauge security concerns around
  # `class_eval`. Not called by default; the expectation is this will be used
  # in tests.
  module GeneratedCodeValidation

    class ValidationError < RuntimeError; end

    def self.validate_deserialize(source)
      parsed = parse_to_ast(source)

      assert_equal(:def, parsed.type)
      name, args, body = parsed.children
      assert_equal(:__t_props_generated_deserialize, name)

      assert_equal(s(:args, s(:arg, :hash)), args)

      assert_equal(:begin, body.type)
      init, *prop_clauses, ret = body.children

      assert_equal(:lvasgn, init.type)
      init_name, init_val = init.children
      assert_equal(:found, init_name)
      assert_equal(:int, init_val.type)
      assert_equal(s(:lvar, :found), ret)

      prop_clauses.each_with_index do |clause, i|
        if i % 2 == 0
          validate_deserialize_hash_read(clause)
        else
          validate_deserialize_ivar_set(clause)
        end
      end
    end

    def self.validate_serialize(source)
      parsed = parse_to_ast(source)

      assert_equal(:def, parsed.type)
      name, args, body = parsed.children
      assert_equal(:__t_props_generated_serialize, name)

      assert_equal(s(:args, s(:arg, :strict)), args)

      assert_equal(:begin, body.type)
      init, *prop_clauses, ret = body.children

      assert_equal(s(:lvasgn, :h, s(:hash)), init)
      assert_equal(s(:lvar, :h), ret)

      prop_clauses.each do |clause|
        validate_serialize_clause(clause)
      end
    end

    private_class_method def self.parse_to_ast(source)
      # This is an optional dependency for sorbet-runtime in general,
      # but is required if this method is to be used.
      require 'parser/current'
      Parser::CurrentRuby.parse(source)
    end

    private_class_method def self.validate_serialize_clause(clause)
      assert_equal(:if, clause.type)
      condition, if_body, else_body = clause.children

      assert_equal(:send, condition.type)
      receiver, method = condition.children
      assert_equal(:ivar, receiver.type)
      assert_equal(:nil?, method)

      unless if_body.nil?
        assert_equal(:if, if_body.type)
        if_strict_condition, if_strict_body, if_strict_else = if_body.children
        assert_equal(s(:lvar, :strict), if_strict_condition)
        assert_equal(:send, if_strict_body.type)
        on_strict_receiver, on_strict_method, on_strict_arg = if_strict_body.children
        assert_equal(nil, on_strict_receiver)
        assert_equal(:required_prop_missing_from_serialize, on_strict_method)
        assert_equal(:sym, on_strict_arg.type)
        assert_equal(nil, if_strict_else)
      end

      assert_equal(:begin, else_body.type)
      assign_val, set_h = else_body.children
      assert_equal(:lvasgn, assign_val.type)
      assigned_ivar, assignment = assign_val.children
      assert_equal(:val, assigned_ivar)
      assert_equal(:ivar, assignment.type)

      assert_equal(:send, set_h.type)
      set_h_receiver, set_h_method, set_h_key, set_h_val = set_h.children
      assert_equal(s(:lvar, :h), set_h_receiver)
      assert_equal(:[]=, set_h_method)
      assert_equal(:str, set_h_key.type)

      validate_lack_of_side_effects(set_h_val, whitelisted_methods_for_serialize)
    end

    private_class_method def self.validate_deserialize_hash_read(clause)
      assert_equal(:lvasgn, clause.type)
      name, val = clause.children
      assert_equal(:val, name)
      assert_equal(:send, val.type)
      receiver, method, arg = val.children
      assert_equal(s(:lvar, :hash), receiver)
      assert_equal(:[], method)
      assert_equal(:str, arg.type)
    end

    private_class_method def self.validate_deserialize_ivar_set(clause)
      assert_equal(:ivasgn, clause.type)
      ivar_name, deser_val = clause.children
      unless ivar_name.is_a?(Symbol)
        raise ValidationError.new("Unexpected ivar: #{ivar_name}")
      end

      assert_equal(:if, deser_val.type)
      condition, if_body, else_body = deser_val.children
      assert_equal(s(:send, s(:lvar, :val), :nil?), condition)

      assert_equal(:begin, if_body.type)
      update_found, handle_nil = if_body.children
      assert_equal(:if, update_found.type)
      found_condition, found_if_body, found_else_body = update_found.children
      assert_equal(:send, found_condition.type)
      receiver, method, arg = found_condition.children
      assert_equal(s(:lvar, :hash), receiver)
      assert_equal(:key?, method)
      assert_equal(:str, arg.type)
      assert_equal(nil, found_if_body)
      assert_equal(s(:op_asgn, s(:lvasgn, :found), :-, s(:int, 1)), found_else_body)

      validate_deserialize_handle_nil(handle_nil)
      validate_lack_of_side_effects(else_body, whitelisted_methods_for_deserialize)
    end

    private_class_method def self.validate_deserialize_handle_nil(node)
      case node.type
      when :hash, :array, :str, :sym, :int, :float, :nil
        # Primitives are ok
      when :send
        receiver, method, arg = node.children
        if receiver.nil?
          assert_equal(:required_prop_missing_from_deserialize, method)
          assert_equal(:sym, arg.type)
        elsif receiver == s(:send, s(:send, s(:self), :class), :decorator)
          assert_equal(:raise_nil_deserialize_error, method)
          assert_equal(:str, arg.type)
        elsif method == :default
          assert_equal(:send, receiver.type)
          inner_receiver, inner_method, inner_arg = receiver.children
          assert_equal(
            s(:send, s(:send, s(:send, s(:self), :class), :decorator), :props_with_defaults),
            inner_receiver,
          )
          assert_equal(:fetch, inner_method)
          assert_equal(:sym, inner_arg.type)
        else
          raise ValidationError.new("Unexpected receiver in nil handler: #{node.inspect}")
        end
      else
        raise ValidationError.new("Unexpected nil handler: #{node.inspect}")
      end
    end

    private_class_method def self.validate_lack_of_side_effects(node, whitelisted_methods_by_receiver_type)
      case node.type
      when :const
        # This is ok, because we'll have validated what method has been called
        # if applicable
      when :hash, :array, :str, :sym, :int, :float, :nil, :self
        # Primitives & self are ok
      when :lvar, :arg
        unless node.children.all? {|c| c.is_a?(Symbol)}
          raise ValidationError.new("Unexpected child for #{node.type}: #{node.inspect}")
        end
      when :args, :mlhs, :block, :begin, :if
        node.children.each {|c| validate_lack_of_side_effects(c, whitelisted_methods_by_receiver_type) if c}
      when :send
        receiver, method, *args = node.children
        if receiver
          if receiver.type == :send
            key = receiver
          else
            key = receiver.type
            validate_lack_of_side_effects(receiver, whitelisted_methods_by_receiver_type)
          end

          if !whitelisted_methods_by_receiver_type[key]&.include?(method)
            raise ValidationError.new("Unexpected method #{method} called on #{receiver.inspect}")
          end
        end
        args.each do |arg|
          validate_lack_of_side_effects(arg, whitelisted_methods_by_receiver_type)
        end
      else
        raise ValidationError.new("Unexpected node type #{node.type}: #{node.inspect}")
      end
    end

    private_class_method def self.assert_equal(expected, actual)
      if expected != actual
        raise ValidationError.new("Expected #{expected}, got #{actual}")
      end
    end

    private_class_method def self.whitelisted_methods_for_serialize
      @whitelisted_methods_for_serialize ||= {
        :lvar => %i{serialize dup map transform_values transform_keys each_with_object []=},
        :const => %i{checked_serialize deep_clone_object},
      }
    end

    private_class_method def self.whitelisted_methods_for_deserialize
      @whitelisted_methods_for_deserialize ||= {
        :lvar => %i{dup map transform_values transform_keys each_with_object []=},
        :const => %i{deserialize from_hash deep_clone_object},
      }
    end

    private_class_method def self.s(type, *children)
      Parser::AST::Node.new(type, children)
    end
  end
end
