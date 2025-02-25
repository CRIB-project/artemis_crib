import sys
import os
import subprocess

if __name__ == "__main__":
    ARTEMIS_WORK = os.environ["ARTEMIS_WORKDIR"]
    RUNINFO = sys.argv[1]

    RUNNAME = RUNINFO[:-4]
    RUNNUM = RUNINFO[-4:]

    if int(RUNNUM) < 249:
        cmd = (
            "cd "
            + ARTEMIS_WORK
            + '; artemis -l \'scripts/evtloop.C("chkbeamsingle_before","'
            + RUNNAME
            + '","'
            + RUNNUM
            + '","si26")\''
        )
        print(cmd)
        subprocess.call([cmd], shell=True)
    else:
        cmd = (
            "cd "
            + ARTEMIS_WORK
            + '; artemis -l \'scripts/evtloop.C("chkbeamsingle_after","'
            + RUNNAME
            + '","'
            + RUNNUM
            + '","si26")\''
        )
        print(cmd)
        subprocess.call([cmd], shell=True)
