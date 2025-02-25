import os
import subprocess

if __name__ == "__main__":
    ARTEMIS_WORK = os.environ["ARTEMIS_WORKDIR"]

    names = ["high", "bg"]
    # gates = ["si26", "al25", "mg24"]
    gates = ["si26"]

    procs = []
    for name in names:
        for gate in gates:
            cmd = (
                "cd "
                + ARTEMIS_WORK
                + "; artemis -l 'scripts/evtloop.C(\"procTGTIK_"
                + gate
                + '","'
                + name
                + '","0000","'
                + gate
                + "\")'"
            )
            print(cmd)
            procs.append(subprocess.Popen([cmd], shell=True, executable="/usr/bin/zsh"))

    for proc in procs:
        proc.communicate()
