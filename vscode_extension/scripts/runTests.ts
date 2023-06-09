// Modified from https://code.visualstudio.com/api/working-with-extensions/testing-extension
import {resolve, join} from 'path';
import {existsSync} from 'fs';
import {runTests} from 'vscode-test';

function usage() {
  console.error(
    'usage: run_tests [path/to/extension] [path/to/extension/tests]',
  );
}

async function main(extensionDir: string, extensionTests: string) {
  try {
    // If not specified, then vscode-test will download VS Code.
    const vscodeExecutablePath: string | undefined = process.env.VSCODE_PATH;
    if (vscodeExecutablePath) {
      console.log(`Using VS Code at ${vscodeExecutablePath}`);
    }

    // The folder containing the Extension Manifest package.json
    // Passed to `--extensionDevelopmentPath`
    const extensionDevelopmentPath = resolve(extensionDir);
    const packageJsonPath = join(extensionDevelopmentPath, 'package.json');
    if (!existsSync(packageJsonPath)) {
      console.error(
        `Unable to locate \`${packageJsonPath}\`. Did you pass the correct extension directory?`,
      );
      usage();
      process.exit(2);
    } else {
      console.log(`Testing extension at \`${extensionDevelopmentPath}\``);
    }

    // The path to the extension test runner script
    // Passed to --extensionTestsPath
    const extensionTestsPath = extensionTests
      ? resolve(extensionTests)
      : resolve(extensionDevelopmentPath, 'out', 'src', 'test', 'index.js');
    if (!existsSync(extensionTestsPath)) {
      console.error(
        `Unable to locate test entry point \`${extensionTestsPath}\``,
      );
      process.exit(3);
    }

    const launchArgs = [
      '--disable-extensions',
      '--disable-telemetry',
      '--disable-updates',
    ];
    if (process.env.VSCODE_USER_DATA_DIR) {
      console.log(`Using user-data-dir ${process.env.VSCODE_USER_DATA_DIR}`);
      launchArgs.push('--user-data-dir', process.env.VSCODE_USER_DATA_DIR);

      if (!existsSync(process.env.VSCODE_USER_DATA_DIR)) {
        console.error(
          `Error: ${process.env.VSCODE_USER_DATA_DIR} does not exist.`,
        );
        process.exit(4);
      }
    }

    // Run the integration tests on the already installed VS Code.
    await runTests({
      extensionDevelopmentPath,
      extensionTestsPath,
      launchArgs,
      vscodeExecutablePath,
      // https://github.com/microsoft/vscode-test/issues/221
      version: '1.78.2'
    });
  } catch (err) {
    console.error('Failed to run tests');
    console.error(err);
    process.exit(5);
  }
}

if (process.argv.length < 3) {
  usage();
  process.exit(1);
} else {
  main(process.argv[2], process.argv[3]);
}
