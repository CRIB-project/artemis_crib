import subprocess
from concurrent.futures import ProcessPoolExecutor, as_completed
import os
import constant_si26ap

ARTEMIS_WORKDIR = os.environ["ARTEMIS_WORKDIR"]
MAX_LEVEL = 40


def run_script(foutname="", excited=0.0, cs_file=""):
    cmd = (
        "cd "
        + ARTEMIS_WORKDIR
        + '; artemis -l \'ana2/simEventLoop.C("simTGTIK","'
        + foutname
        + '","'
        + str(excited)
        + '","'
        + cs_file
        + "\")'"
    )
    subprocess.run(cmd, shell=True)
    return f"  [finished] {cmd=}"


if __name__ == "__main__":
    foutnames = []
    cs_filenames = []
    for i in range(MAX_LEVEL + 1):
        foutnames.append(f"ana2/syserror/p{i}.root")
        cs_filenames.append(f"model/ap.L{i:02}")

    excited_energies = constant_si26ap.excited_energies

    with ProcessPoolExecutor(max_workers=os.cpu_count() - 4) as executor:
        # with ProcessPoolExecutor(max_workers=1) as executor:
        futures = []
        for i in range(MAX_LEVEL + 1):
            futures.append(
                executor.submit(
                    run_script,
                    foutnames[i],
                    excited_energies[i],
                    cs_filenames[i],
                )
            )

        for future in as_completed(futures):
            result = future.result()
            print(result)
