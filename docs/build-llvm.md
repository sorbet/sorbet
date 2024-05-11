# Building an LLVM release

Useful links:

- <https://www.llvm.org/docs/ReleaseProcess.html>
- <https://github.com/llvm/llvm-project/issues/53892>
- <https://github.com/llvm/llvm-project/issues/64703>

Commands I've run in the past:

```bash
pay ssh
sudo apt-get install -y autoconf build-essential cmake parallel mc chrpath ninja

# This builds LLVM with the GCC toolchain that's on the box
# (It seems like it actually bootstraps it.)
llvm/utils/release/test-release.sh \
  -no-test-suite \
  -no-compare-files \
  -release 15.0.7 \
  -final \
  -triple x86_64-linux-gnu-ubuntu-20.04 \
  -j 16 \
  -use-ninja \
  -lldb
```

The release tarball is at `final/*.tar.xz`.

The build took over 2 hours to create on a devbox (I just know that the first
attempt failed at the 2 hour mark--I didn't get the full runtime for the time it
passed).

It also ran the tests at the end (I thought that the `-no-test-suite` flag would
have turned that off), which added another half hour to the runtime. The test
results looked like this:

```
Testing Time: 1936.77s
  Skipped          :    46
  Unsupported      :  4791
  Passed           : 99647
  Expectedly Failed:   303
  Timed Out        :    18
  Failed           :    28
FAILED: CMakeFiles/check-all
```

Having a 99.95% pass rate was good enough for me.

## Tinkering with these releases

The easiest way to trick bazel into using this tarball is to put the tarball in
a folder, serve that folder with `python -m http.server`, and then update the
`llvm_mirror_prefixes` variable in the `WORKSPACE` so that it hits the localhost
mirror first.

## Publishing these releases

To make these archives available to Sorbet when building in Buildkite CI and in
Stripe's CI environment, we store the archives in S3.

I always use the AWS console for this. There are instructions for how to log in
at <http://go/types/elastic-ci>. From there, you'll have to

- Make a new folder with the LLVM version number
- Upload any archives you built
- Set the "Everyone (public access)"
  (http://acs.amazonaws.com/groups/global/AllUsers) ACL to **Read**, so that
  Buildkite can access it.
  - To do this, you'll have to temporarily change the [Block Public Access
    settings for this
    account](https://us-west-2.console.aws.amazon.com/s3/settings?region=us-west-2&bucketType=general)
    for the Sorbet S3 account.
  - Be sure to flip this back to off when you're done.

It may be useful to use an `aws s3 cp`
invocation for this. If you do that, feel free to update this doc with what
command you used.
