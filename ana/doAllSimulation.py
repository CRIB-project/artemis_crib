import subprocess
from concurrent.futures import ProcessPoolExecutor, as_completed
import os

ARTEMIS_WORKDIR = os.environ["ARTEMIS_WORKDIR"]
MAX_LEVEL = 40


def run_script_window(arg=""):
    cmd = (
        "cd "
        + ARTEMIS_WORKDIR
        + "; artemis -l 'ana2/simEventLoop.C(\"simWindow"
        + arg
        + "\")'"
    )
    subprocess.run(cmd, shell=True)
    return f"  [finished] {cmd=}"


def run_script_apx(arg="", foutname="", excited=0.0, cs_file=""):
    cmd = (
        "cd "
        + ARTEMIS_WORKDIR
        + "; artemis -l 'ana2/simEventLoop.C(\"simApxReaction"
        + arg
        + '","'
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
    apx_cal_foutnames = []
    apx_sim_foutnames = []
    cs_filenames = []
    for i in range(MAX_LEVEL + 1):
        apx_cal_foutnames.append(f"output/sim/ap/p{i}_cal.root")
        apx_sim_foutnames.append(f"output/sim/ap/p{i}.root")
        cs_filenames.append(f"model/ap.L{i:02}")

    excited_energies = [
        0.0,
        1.38355,
        1.95391,
        2.4227,
        3.1059,
        3.4476,
        4.0805,
        4.343001,
        4.642001,
        4.759001,
        4.95410142,
        5.0470012,
        5.293002,
        5.527001,
        5.583001,
        5.716001,
        5.740001,
        5.826001,
        5.968,
        6.191,
        6.328,
        6.505,
        6.577,
        6.828,
        6.956,
        7.021,
        7.148,
        7.272,
        7.361,
        7.456,
        7.523,
        7.641,
        7.755,
        7.95,
        7.998,
        8.106,
        8.234,
        8.297,
        8.379,
        8.432,
        8.51,
    ]

    with ProcessPoolExecutor(max_workers=os.cpu_count() - 4) as executor:
        # with ProcessPoolExecutor(max_workers=1) as executor:
        futures = []
        futures.append(executor.submit(run_script_window, ""))
        futures.append(executor.submit(run_script_window, "_cal"))
        for i in range(MAX_LEVEL + 1):
            futures.append(
                executor.submit(
                    run_script_apx,
                    "",
                    apx_sim_foutnames[i],
                    excited_energies[i],
                    cs_filenames[i],
                )
            )
            futures.append(
                executor.submit(
                    run_script_apx,
                    "_cal",
                    apx_cal_foutnames[i],
                    excited_energies[i],
                    cs_filenames[i],
                )
            )

        for future in as_completed(futures):
            result = future.result()
            print(result)
