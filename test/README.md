# Test for Sorbet

We try to make it easy to add tests, and thus subfolders of this directory are magic.
 - any file `<fileName>.rb` that is added to `test/testdata` becomes a test.
    * we will check that Sorbet only emits errors on lines that contain
        `# error: <text>` and that `<text>` is contained in that error messages.
        In case there are multiple errors on this line, use `MULTI` as `<text>`.
    * you can optionally create `<fileName>.rb.<phase>.exp` files that will contain
        pretty printed internal state after `<phase>`.

 - any folder `<folderName>` that is added to `test/cli` becomes a test.
    * this folder should contain an executable `<folderName>.sh`. It will be run
        its output will be compared against `<folderName>.out`.

 - any folder `<folderName>` that is added to `test/lsp` will become a test.
    * this folder should contain a file named `<folderName>.rec` that contains a recorded LSP session.
    * lines that start with "Read: " will be sent to sorbet as input.
    * lines that start with "Write: " will be expected from sorbet as output.


