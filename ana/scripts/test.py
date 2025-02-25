import os
import subprocess

if __name__ == '__main__':
    ARTEMIS_WORK = os.environ['ARTEMIS_WORKDIR']

    cmd = "cd " + ARTEMIS_WORK + "; " + ARTEMIS_WORK + "/build/run_artemis " + ARTEMIS_WORK + "/steering/procproton.yaml high 0000 si26 tel1"
    print(cmd)
    subprocess.call([cmd], shell=True, executable="/usr/bin/zsh")


