import os
import subprocess

if __name__ == '__main__':
    ARTEMIS_WORK = os.environ['ARTEMIS_WORKDIR']

    #gates = ["si26", "al25", "mg24"]
    gates = ["si26"]

    procs = []
    for j in range(len(gates)):
        cmd = "cd " + ARTEMIS_WORK + "; artemis -l '" + ARTEMIS_WORK + "/scripts/evtloop.C(\"simSolidAngle_\", \"\", \"\", \"" + gates[j] + "\", \"\")'"
        print(cmd)
        procs.append(subprocess.Popen([cmd], shell=True, executable="/usr/bin/zsh"))

    for proc in procs:
        proc.communicate()


