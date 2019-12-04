## JRuby

> The Ruby Programming Language on the JVM

This document serves as a getting-started guide for those wishing to use
[Sorbet](https://sorbet.org) with [JRuby](https://www.jruby.org/).

The use of Sorbet on the JVM is **NOT SUPPORTED** by the Sorbet
development team however this document services as a getting started guide
for those who wish to trot on a unsupported path.

### Performance

TBD

### Gotchas

#### RBI for Java Code

The `srb` tool can auto-generate RBI files for your gems however it cannot
do so for your Java code :(

> The technical reason is because the Java code is never officially `required`
> so that it never ends up being triggered through TracePoint

To move past the woes of the type-checker, I have found the following hand
curated pattern for Java RBI files good:

```
tree ./sorbet
|------ config
|------ rbi
    |-------- java
    |   |---------- java_lang.rbi # this is the standard library
    |   |---------- org_eclipse_jgit.rbi
    |   |---------- org_apache_logging_log4j.rbi
    |-------------  org_slf4j.rbi
```

Basically, I take the _maven coordinates_ for a given java packet (replaced with
underscores) and hand-create an RBI file for it.

You can put as much as you need depending on the `#typed` sigils you are aiming
for.

```ruby
# typed: strong

module Java

  module OrgEclipseJgitApiErrors

    class JGitInternalException < Exception

    end

  end

end
```

#### Installing sorbet-static

You might have added `sorbet` to your Gemfile but noticed that sorbet-static
was not included and that `srb` failed!

This is because [at the moment](https://github.com/sorbet/sorbet/pull/2254) the
Sorbet team does not publish a `sorbet-static` gem for the Java platform.

The workaround is to fetch the Gem manually for your arch and install it.

For instance on OSX you would do:
```
# download the most recent one
gem fetch sorbet-static --platform universal-darwin
# install it from the local file
gem install --local sorbet-static-*-universal-darwin-*.gem
```

#### Debug Flag

When running the `srb` command to generate RBI files for any gems, you may depend
on -- make sure to have the `--debug` flag enabled.

```
jruby --debug srb rbi gems
```

Sorbet relies on `TracePoint` to discover modules & classes, which is disabled
by default in JRuby unless the flag is enabled.

For good measure you can set the following

```
jruby -J-Djruby.objectspace.enabled=true -J-Djruby.compile.mode=OFF \
       -J-Djruby.debug.fullTrace=true --debug srb rbi gems
```

#### Naming Convention For Java code

You can reference a Java class in JRuby in a couple of different ways.

```
Java: org.foo.department.Widget
Ruby: Java::OrgFooDepartment::Widget
```

You can even leverage `java_import` and then access the class non
fully qualified.

Unfortunately, Sorbet will not be able to resolve many of these ways and the
_only_ course of action is to take fully-qualify the code and transform the name
by removing the dots and converting to CamelCase.

If you don't, Sorbet will think that Java class use such as `org.foo.department.Widget`
are a series of method calls.
