import os
import glob
import subprocess

if __name__ == "__main__":
    ARTEMIS_WORK = os.environ["ARTEMIS_WORKDIR"]

    # high si26
    output = ARTEMIS_WORK + "/output/coin/si26/high_si26_all.root"

    inputs = ""
    input_array = glob.glob(ARTEMIS_WORK + "/output/coin/si26/high/*.root")
    for i in range(len(input_array)):
        inputs += input_array[i] + " "

    cmd = (
        "cd "
        + ARTEMIS_WORK
        + "; artemis -l 'scripts/processline.C(\"mergefile "
        + output
        + " "
        + inputs
        + "\")'"
    )

    print(cmd)
    subprocess.call([cmd], shell=True)

    # bg si26
    output = ARTEMIS_WORK + "/output/coin/si26/bg_si26_all.root"

    inputs = ""
    input_array = glob.glob(ARTEMIS_WORK + "/output/coin/si26/bg/*.root")
    for i in range(len(input_array)):
        inputs += input_array[i] + " "

    cmd = (
        "cd "
        + ARTEMIS_WORK
        + "; artemis -l 'scripts/processline.C(\"mergefile "
        + output
        + " "
        + inputs
        + "\")'"
    )

    print(cmd)
    subprocess.call([cmd], shell=True)
