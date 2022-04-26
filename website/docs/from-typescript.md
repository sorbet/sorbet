---
id: from-typescript
title: TypeScript ↔ Sorbet Reference
sidebar_label: TypeScript ↔ Sorbet
---

Many people learning Sorbet for the first time come from a TypeScript
background. We've built this quick reference table mapping some common
TypeScript features to the corresponding feature in Sorbet. (This is not a
document about automatically interoperating between Sorbet and TypeScript; it is
merely provided for educational purposes.)

Note that Sorbet is not affiliated with TypeScript in any official way, and the
information in this table may become out of date as TypeScript evolves
independently of Sorbet. Please use the "Edit" button at the top of the page to
help correct inaccuracies.

Also note that Sorbet features do not always map to an exactly equivalent
TypeScript feature. Be sure to read the relevant Sorbet docs to learn more about
the features Sorbet provides.

<table>
  <thead>
    <tr>
      <th>
        TypeScript
      </th>
      <th>
        Sorbet
      </th>
      <th>
        Notes
      </th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>
        <code>string</code>
      </td>
      <td>
        <code>String</code>
      </td>
      <td>
        Ruby strings are represented by <a href="/docs/class-types">Class
        Types</a>. All values in Ruby are instances of a particular class,
        including strings.
      </td>
    </tr>
    <tr>
      <td>
        <code>number</code>
      </td>
      <td>
        <code>Integer</code>, <code>Float</code>, <code>Numeric</code>
      </td>
      <td>
        Ruby has <a href="/docs/class-types">separate classes</a> for different
        numeric types. <code>number</code> in JavaScript is most closely
        <code>Float</code> in Ruby, but unlike in JavaScript, integers and
        floating-point numbers are not transparently compatible with each
        other. To convert between different types, Ruby provides various <code>to_i</code>
        and <code>to_f</code> methods. See the Ruby docs for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>boolean</code>
      </td>
      <td>
        <code>T::Boolean</code> (also: <code>TrueClass</code>,
        <code>FalseClass</code>)
      </td>
      <td>
        JavaScript has boolean true and false as a primitive boolean type. Ruby
        has true as an instance of the Ruby <code>TrueClass</code> class and
        false as an instance of the <code>FalseClass</code> class (these are
        normal <a href="/docs/class-types">Class Types</a>). Sorbet then
        uses a <a href="/docs/union-types">Union Type</a> to define a <a
        href="/docs/type-aliases">Type Alias</a> called <code>T::Boolean</code>
        inside the <code>sorbet-runtime</code> gem.
      </td>
    </tr>
    <tr>
      <td>
        <code>string[]</code>
      </td>
      <td>
        <code>T::Array[String]</code>
      </td>
      <td>
        There is more information about why Sorbet uses this syntax in <a
        href="/docs/stdlib-generics">Arrays, Hashes, and Generics in the Standard
        Library</a>.
      </td>
    </tr>
    <tr>
      <td>
        <code>any</code>
      </td>
      <td>
        <code>T.untyped</code>
      </td>
      <td>
        See <a href="/docs/untyped">T.untyped</a> and <a href="../gradual">Gradual
        Type Checking</a> for more information.
      </td>
    </tr>
    <tr>
      <td>
        <code>noImplicitAny</code>
      </td>
      <td>
        <code># typed: strict</code>
      </td>
      <td>
        TypeScript uses the <code>noImplicitAny</code> command line flag to
        prevent accidentally inferring <code>any</code> at function boundaries.
        Sorbet instead uses <a href="/docs/static">Strictness Levels</a>, which are
        controlled by comments that live at the top of individual files.
      </td>
    </tr>
    <tr>
      <td>
        <pre><code>function f(
  x: number
): string {...}</code></pre>
      </td>
      <td>
        <pre><code>sig do
  params(x: Integer)
  .returns(String)
end
def f(x); ...; end</code></pre>
      </td>
      <td>
        See <a href="/docs/sigs">Method Signatures</a>.
      </td>
    </tr>
    <tr>
      <td>
        <pre><code>let myName: string = "Alice"</code></pre>
      </td>
      <td>
        <pre><code>my_name = T.let("Alice", String)</code></pre>
      </td>
      <td>
        Unlike in TypeScript, this will do <strong>both</strong> a static and
        <a href="/docs/runtime">runtime type check</a>. See <a
        href="/docs/type-annotations">Type Annotations</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>number | string</code>
      </td>
      <td>
        <code>T.any(Integer, String)</code>
      </td>
      <td>
        Sorbet uses <code>T.any(...)</code> to declare <a
        href="/docs/union-types">Union types</a>. Do not confuse this with
        TypeScript's <code>any</code>, which corresponds to Sorbet's
        <code>T.untyped</code>.
      </td>
    </tr>
    <tr>
      <td>
        <code>x?: string</code>,<br>
        <code>x: string | null</code>
      </td>
      <td>
        <code>T.nilable(String)</code>
      </td>
      <td>
        Ruby does has only <code>nil</code>, while JavaScript has both
        <code>null</code> and <code>undefined</code>. See <a
        href="/docs/nilable">Nilable Types</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>if (typeof x === "string")</code>,<br>
        <code>if (x instanceof Date)</code>
      </td>
      <td>
        <pre><code>if x.is_a?(String)
# or
case x
when String then # ...
end</code></pre>
      </td>
      <td>
        All Ruby objects are instances of a class, so the comparison for
        JavaScript's <code>typeof</code> operator is replaced by
        <code>is_a?</code> in Ruby. See <a
        href="/docs/flow-sensitive">Flow-Sensitive Typing</a>.
      </td>
    </tr>
    <tr>
      <td>
        <code>if (x == null)</code>
      </td>
      <td>
        <code>if x.nil?</code>
      </td>
      <td>
        See <a href="/docs/flow-sensitive">Flow-Sensitive Typing</a>.
      </td>
    </tr>
    <tr>
      <td>
        <pre><code>type AorB = A | B
</code></pre>
      </td>
      <td>
        <pre><code>AorB = T.type_alias {T.any(A, B)}</code></pre>
      </td>
      <td>
        See <a href="/docs/type-aliases">Type Aliases</a>.
      </td>
    </tr>
    <tr>
      <td>
        <pre><code>{
  x: number,
  y?: string,
}</code></pre>
      </td>
      <td>
        <pre><code>class Thing < T::Struct
  prop :x, Integer
  prop :y, T.nilable(String)
end</code></pre>
      </td>
      <td>
        Sorbet uses <a href="/docs/tstruct">T::Struct</a> subclasses where
        TypeScript uses what it calls object types.
      </td>
    </tr>
    <tr>
      <td>
        <code>let x = expr as Type</code>
      </td>
      <td>
        <code>x = T.cast(expr, Type)</code>
      </td>
      <td>
        Unlike in TypeScript, this will do <strong>both</strong> a static and
        <a href="/docs/runtime">runtime type check</a>. See <a
        href="/docs/type-annotations">Type Annotations</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>let x = expr as any</code>
      </td>
      <td>
        <code>x = T.unsafe(expr)</code>
      </td>
      <td>
        See <a href="/docs/type-annotations">Type Annotations</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <a href="https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#literal-types">Literal Types</a>,
        like <code>"foo"</code>
      </td>
      <td>
        none
      </td>
      <td>
        Sorbet has no exact equivalent for TypeScript's literal types. Consider
        using <a href="/docs/tenum">Typed Enums</a> instead.
      </td>
    </tr>
    <tr>
      <td>
        <code>x!.foo</code>
      </td>
      <td>
        <code>T.must(x).foo</code>
      </td>
      <td>
        See <a href="/docs/type-assertions">Type Assertions</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>unknown</code>
      </td>
      <td>
        <code>BasicObject</code>
      </td>
      <td>
        See <a href="/docs/class-types">Class Types</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>never</code>
      </td>
      <td>
        <code>T.noreturn</code>
      </td>
      <td>
        See <a href="/docs/noreturn">T.noreturn</a> and <a
        href="/docs/exhaustiveness">Exhaustiveness Checking</a> for more.
      </td>
    </tr>
  </tbody>
</table>

This table is unfortunately not exhaustive. If you were looking for something in
this table but couldn't find it, please either use the "Edit" button above to
add it, or reach out to a member of the Sorbet team asking whether it can be
updated.

<!--
  NOTE!
  The below script will delete the on-page nav to make room for the table.
  Using headings will not show up in a sidebar.
-->

<script src="/js/from-typescript.js"></script>
