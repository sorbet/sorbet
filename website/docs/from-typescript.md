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
        other. To convert between different types, Ruby provides various
        <code>to_i</code> and <code>to_f</code> methods. See the Ruby docs for
        more.
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
        See <a href="/docs/untyped">T.untyped</a> and <a href="/docs/gradual">Gradual
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
        Ruby has only <code>nil</code>, while JavaScript has both
        <code>null</code> and <code>undefined</code>. See <a
        href="/docs/nilable-types">Nilable Types</a> for more.
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
        <code>x?.foo</code>
      </td>
      <td>
        <code>x&.foo</code>
      </td>
      <td>
        This is built into Ruby (Sorbet is not required to use it). The feature
        is called "optional chaining" in TypeScript or "the safe navigation
        operator" in Ruby.
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
        <code>x ?? y</code>
      </td>
      <td>
        <code>x.nil? ? y : x</code>
      </td>
      <td>
        TypeScript calls this feature "nullish coalescing". A close
        approximation in Ruby is <code>x || y</code>. Only <code>nil</code> and
        <code>false</code> are falsy in Ruby (meanwhile <code>0</code>,
        <code>''</code>, <code>NaN</code>, etc. are falsy in TypeScript). Thus,
        <code>x || y</code> in Ruby behaves exactly like <code>x ?? y</code> in
        TypeScript <strong>unless</strong> <code>x</code> can be
        <code>false</code>. If <code>x</code> is false, <code>x || y</code>
        evaluates to <code>y</code>, while <code>x ?? y</code> evaluates to
        <code>false</code>. The closest approximation is to use a ternary
        operator.
      </td>
    </tr>
    <tr>
      <td>
        <code>unknown</code>
      </td>
      <td>
        <code>T.anything</code>
      </td>
      <td>
        See <a href="/docs/anything">T.anything</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>never</code>
      </td>
      <td>
        <code>T.noreturn</code> (in type annotation),
        <code>T.absurd</code> (as type assertion)
      </td>
      <td>
        See <a href="/docs/noreturn">T.noreturn</a> and <a
        href="/docs/exhaustiveness">Exhaustiveness (T.absurd)</a> for more.
      </td>
    </tr>
    <tr>
      <td>
        <code>pet is Fish</code> (<a href="https://www.typescriptlang.org/docs/handbook/2/narrowing.html#using-type-predicates">Type Predicates</a>)
      </td>
      <td>
        none
      </td>
      <td>
        Sorbet does not have anything like TypeScript's type predicates feature.
        Instead, consider using something like <a href="https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0A%23%20Follows%20the%20example%20defined%20here%3A%0A%23%20https%3A%2F%2Fwww.typescriptlang.org%2Fdocs%2Fhandbook%2F2%2Fnarrowing.html%23using-type-predicates%0A%0A%0Aclass%20Fish%0A%20%20def%20swim%3B%20end%0Aend%0Aclass%20Bird%0A%20%20def%20fly%3B%20end%0Aend%0APet%20%3D%20T.type_alias%20%7BT.any%28Fish%2C%20Bird%29%7D%0A%0A%23%20--%20TypeScript%20--%0A%23%20function%20isFish%28pet%3A%20Fish%20%7C%20Bird%29%3A%20pet%20is%20Fish%20%7B%0A%23%20%20%20return%20%28pet%20as%20Fish%29.swim%20!%3D%3D%20undefined%3B%0A%23%20%7D%0A%0Asig%20%7Bparams%28pet%3A%20Pet%29.returns%28T.nilable%28Fish%29%29%7D%0Adef%20as_fish%28pet%29%0A%20%20if%20pet.is_a%3F%28Fish%29%0A%20%20%20%20pet%0A%20%20else%0A%20%20%20%20nil%0A%20%20end%0Aend%0A%0Asig%20%7Bparams%28zoo%3A%20T%3A%3AArray%5BPet%5D%29.void%7D%0Adef%20example%28zoo%29%0A%20%20%23%20--%20TypeScript%20--%0A%20%20%23%20const%20underWater%3A%20Fish%5B%5D%20%3D%20zoo.filter%28isFish%29%3B%0A%0A%20%20under_water%20%3D%20zoo.map%20%7B%7Cpet%7C%20as_fish%28pet%29%7D.compact%0A%20%20T.let%28under_water%2C%20T%3A%3AArray%5BFish%5D%29%0Aend">this approach.</a>
      </td>
    </tr>
    <tr>
      <td>
        <a href="https://www.typescriptlang.org/docs/handbook/2/narrowing.html#discriminated-unions">Discriminated Unions</a>
      </td>
      <td>
        <a href="/docs/sealed">Sealed Classes and Modules</a>
      </td>
      <td>
        &nbsp;
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
