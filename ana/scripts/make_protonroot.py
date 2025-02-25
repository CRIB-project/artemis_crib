import os
import subprocess

if __name__ == "__main__":
    ARTEMIS_WORK = os.environ["ARTEMIS_WORKDIR"]

    tels = ["tel1", "tel2", "tel3", "tel4", "tel5"]

    # si26
    procs = []
    for tel in tels:
        cmd = (
            "cd "
            + ARTEMIS_WORK
            + '; artemis -l \'scripts/evtloop.C("procproton","high","0000","si26","'
            + tel
            + "\")'"
        )
        print(cmd)
        procs.append(subprocess.Popen([cmd], shell=True))

    for tel in tels:
        cmd = (
            "cd "
            + ARTEMIS_WORK
            + '; artemis -l \'scripts/evtloop.C("procproton","bg","0000","si26","'
            + tel
            + "\")'"
        )
        print(cmd)
        procs.append(subprocess.Popen([cmd], shell=True))

    for proc in procs:
        proc.communicate()
