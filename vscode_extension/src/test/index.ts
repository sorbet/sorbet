import * as path from "path";
import * as Mocha from "mocha";
import * as glob from "glob";

// When the js is generated, this will be in out/src/tests
// which is why it isn't ../../ as we might expect
const PKG = require("../../../package.json");

const TEST_SUITE_NAME = `${PKG.publisher}.${PKG.name}`;

function setupMocha(): Mocha {
  // Magic environment variable from Bazel dictating where the junit should go.
  if (process.env.XML_OUTPUT_FILE) {
    return new Mocha({
      reporter: "mocha-junit-reporter",
      reporterOptions: {
        mochaFile: process.env.XML_OUTPUT_FILE,
        testsuitesTitle: TEST_SUITE_NAME,
      },
      ui: "tdd",
    });
  } else {
    const mocha = new Mocha({
      ui: "tdd",
    });
    mocha.useColors(true);
    return mocha;
  }
}

export function run(): Promise<void> {
  // Create the mocha test
  const mocha = setupMocha();
  const testsRoot = path.resolve(__dirname);

  return new Promise((c, e) => {
    glob("**/**.test.js", { cwd: testsRoot }, (error, files) => {
      if (error) {
        return e(error);
      }

      // Add files to the test suite
      files.forEach((f) => mocha.addFile(path.resolve(testsRoot, f)));

      try {
        // Run the mocha test
        mocha.run((failures) => {
          if (failures > 0) {
            e(new Error(`${failures} tests failed.`));
          } else {
            c();
          }
        });
      } catch (err) {
        e(err);
      }
    });
  });
}
