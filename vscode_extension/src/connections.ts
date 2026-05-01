import { ChildProcess } from "child_process";
import { Log } from "./log";

/**
 * Send a signal to the process group on POSIX, or to the process itself on
 * Windows. Using the process group ensures the signal reaches the actual
 * Sorbet binary even when it is launched through a wrapper script such as
 * `bundle exec srb`, which keeps itself alive as the direct child and runs
 * Sorbet as a subprocess.
 */
function signalProcessGroup(p: ChildProcess, signal: NodeJS.Signals): void {
  if (!p.pid) {
    return;
  }
  try {
    if (process.platform === "win32") {
      p.kill(signal);
    } else {
      process.kill(-p.pid, signal);
    }
  } catch (error) {
    // ESRCH: process group already gone — that's fine.
    if ((error as NodeJS.ErrnoException).code !== "ESRCH") {
      throw error;
    }
  }
}

/**
 * Attempts to stop the given child process. Tries a SIGTERM, then a SIGKILL.
 *
 * On POSIX the signal is sent to the entire process group so that it reaches
 * the actual Sorbet binary even when launched through a wrapper script.
 */
export async function stopProcess(p: ChildProcess, log: Log): Promise<void> {
  return new Promise<void>((resolve) => {
    let hasExited = false;
    log.debug("Stopping process", p.pid);
    function onExit() {
      if (!hasExited) {
        hasExited = true;
        resolve();
      }
    }
    p.on("exit", onExit);
    p.on("error", onExit);
    signalProcessGroup(p, "SIGTERM");
    setTimeout(() => {
      if (!hasExited) {
        log.debug(
          "Process did not respond to SIGTERM with 1s. Sending a SIGKILL.",
          p.pid,
        );
        signalProcessGroup(p, "SIGKILL");
        setTimeout(resolve, 100);
      }
    }, 1000);
  });
}
