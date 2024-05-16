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

# The same, but for Linux on aarch64
llvm/utils/release/test-release.sh \
  -no-test-suite \
  -no-compare-files \
  -release 15.0.7 \
  -final \
  -triple aarch64-linux-gnu \
  -j 16 \
  -use-ninja \
  -lldb

#
# I've never had to run this on macOS, but if/when we do that it would be good
# to update this with the command used.
#
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

We try to test with localhost servers like this before publishing the release
somewhere, because it's way more annoying to yank a bad release from a trusted
source later than it is to spin up a localhost testing server like this.

You will also have to put the release archive name and its checksum in the
`bazel-toolchain` (`toolchains_llvm`) repo, in the file called
`toolchain/internal/llvm_distributions.bzl`. At the moment, there is no way to
provide extra, per-project LLVM releases tarballs—they must all be declared
upstream. We don't submit our LLVM release builds' checksums upstream because we
don't want to be on the hook for maintaining these releases for the whole world.

## Publishing these releases

We have a [GitHub fork of LLVM](https://github.com/sorbet/llvm-project) that we
use only for publishing extra release tarballs. At this time there are no custom
changes to LLVM in this fork in support of building Sorbet.

To publish a new release:

1.  Clone the LLVM project, and add the Sorbet fork as a remote:

    ```
    mkdir -p ~/stripe/github
    cd !$
    git clone git@github.com:llvm/llvm-project.git
    cd llvm-project
    git remote add sorbet git@github.com:sorbet/llvm-project.git
    git fetch sorbet
    ```

    This repo is huge and will take over 4 minutes to clone even on a fast
    connection.

    If you already have this repo locally (maybe this is not your first time
    upgrading LLVM), make sure to pull:

    ```
    cd ~/stripe/github/llvm-project
    git checkout main
    git pull
    git push sorbet main
    ```

    This should update the `main` branch and also fetch all new `release/*`
    branches and all new release tags.

1.  Push the LLVM release branch and tag to the Sorbet fork. (The Sorbet fork by
    default has no branches, only those that we have explicitly pushed to it.)

    ```
    git checkout --track origin/release/15.x
    git push sorbet release/15.x     # release branch
    git push sorbet llvm-org-15.0.7  # release tag
    ```

1.  Create a release in Sorbet's fork with the upstream tag.
    You can use this command to open the release page in a browser. You'll want
    to edit the `LLVM` variable so it populates the right version in all the
    places.

    ```
    (LLVM=15.0.7; open "https://github.com/sorbet/llvm-project/releases/new?tag=llvmorg-$LLVM&title=LLVM%20$LLVM&body=LLVM%20$LLVM%20release%20builds%20created%20by%20the%20Sorbet%20team,%20for%20use%20when%20building%20Sorbet")
    ```

    Make sure that the release notes look good, and attach any binaries that you
    need to to this release.

    When everything looks good, click the green button to create the release.

## [OUTDATED] Publishing releases to S3

> [!NOTE]
>
> We used to use a special S3 bucket owned by Stripe to host custom LLVM builds.
> At some point, Stripe's build infrastructure changed, and these buckets are no
> longer reachable from Stripe's CI environment. These notes are still here for
> posterity's sake.

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

It may be useful to use an `aws s3 cp` invocation for this. If you do that, feel
free to update this doc with what command you used.
