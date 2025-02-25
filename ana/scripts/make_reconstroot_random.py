import os
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed


def run_command(cmd):
    return subprocess.run(cmd, shell=True, executable="/usr/bin/zsh")


if __name__ == "__main__":
    ARTEMIS_WORK = os.environ["ARTEMIS_WORKDIR"]

    names = ["high", "bg"]
    # gates = ["si26", "al25", "mg24"]
    gates = ["si26"]

    commands = []

    for i in range(1, 1001):
        for name in names:
            for gate in gates:
                cmd = (
                    "cd "
                    + ARTEMIS_WORK
                    + "; artemis -l 'scripts/evtloop.C(\"procTGTIK_"
                    + gate
                    + '","'
                    + name
                    + '","'
                    + f"{i:04}"
                    + '","'
                    + gate
                    + "\")'"
                )
                print(cmd)
                commands.append(cmd)
    print("\n")
    print(commands)
    print("\n")

    with ThreadPoolExecutor(max_workers=1) as executor:
        futures = {executor.submit(run_command, cmd): cmd for cmd in commands}

        for future in as_completed(futures):
            cmd = futures[future]
            try:
                result = future.result()
                print(f"Command '{cmd}' completed with return code {result.returncode}")
            except Exception as exc:
                print(f"Command '{cmd}' generated an exception: {exc}")
