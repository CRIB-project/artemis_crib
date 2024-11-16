import argparse
import os
from ruamel.yaml import YAML
import csv

# Global constant
ARTHOME = os.getenv("ARTEMIS_WORKDIR")
if not ARTHOME:
    raise EnvironmentError("command [artlogin user] needed")

# [map] device id, name
DEV_DICT = {
    12: "CRIB",  # CRIB use 12
    0: "BIGRIPS",
    11: "SHARAQ",
    20: "20",  # ??
    31: "SHS1DAQ",  # used in SHARAQ FADC
    63: "GET",  # GET?
}


# [map] fp, name
FP_DICT = {
    0: "E7MPV",  # CRIB use fp as MPV ID
    1: "J1ADC",
    2: "J1TDC",
    3: "F3",
    4: "F4",
    5: "F5",
    6: "F6",
    7: "F7",
    8: "F8",
    9: "F9",
    10: "F10",
    11: "F11",
    12: "F12",
    20: "S0",
    21: "S1",
    22: "S2",
    34: "SHFADC",  # used in FADC
    62: "SHB3F",
    63: "B3F",
}

# [map] mod id, (name, channel number)
MODULE_DICT = {
    6: ("ADC", 32),  # for V785, MADC
    7: ("TDC", 128),  # for V1190A
    0: ("PPACQ", 128),  # assume 128ch module
    1: ("PPACT", 128),  # assume 128ch module
    2: ("PLAQ", 128),  # assume 128ch module
    3: ("PLAT", 128),  # assume 128ch module
    21: ("DALIA", 128),  # assume 128ch module
    22: ("DALIT", 128),  # assume 128ch module
    31: ("SHBLD", 128),  # assume 128ch module
    32: ("SHCRDC", 128),  # assume 128ch module
    33: ("SHCRDCA", 128),  # assume 128ch module
    55: ("FADC", 32),  # FADC in SHARAQ exp
    60: ("TIMESTAMP", 1),  # for MPV TS
    63: ("SCALER", 32),  # for SIS3820
}

# after this number, begin on a new line (just format)
UNIT = 16


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i",
        "--input-mapper",
        type=str,
        help="if you don't use default mapper.conf, please specify the path (ex test/mapper.conf)",
        default="mapper.conf",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=str,
        help="if you want to specify the outputfile name, input the output csv file. (default. pyscripts/out/mapcheckr.csv)",
        default="pyscripts/out/mapchecker.csv",
    )
    return parser.parse_args()


# read from mapper.conf => infomation for each mapfile
def get_mapfiles(mapper: str) -> list:
    mapfile_path = os.path.join(ARTHOME, mapper)
    if not os.path.exists(mapfile_path):
        raise FileNotFoundError(f"{mapfile_path} does not exist!")

    mapfile_list = []
    with open(mapfile_path, "r") as file:
        for line in file:
            line = line.split("#")[0].strip()
            if line:
                mapfile_list.append(line)

    return mapfile_list


# read each map file => information each line (channel)
def get_mapfile_data(mapfile: str) -> list:
    mapinfo = mapfile.split()  # [0]: path, [1]: number
    if len(mapinfo) != 2:
        raise ValueError(f"mapper.conf format error: {mapfile}")

    try:
        maplen = int(mapinfo[1])
    except ValueError:
        raise ValueError(f"Invalid number format in mapper.conf: {mapinfo[1]}")

    mapfile_path = os.path.join(ARTHOME, mapinfo[0])
    if not os.path.exists(mapfile_path):
        raise FileNotFoundError(f"{mapfile_path} does not exist!")

    mapinfo_list = []
    with open(mapfile_path, "r") as file:
        for line in file:
            line = line.split("#")[0].strip()
            if not line:
                continue
            parts = []
            for part in line.split(","):
                for item in part.split():
                    try:
                        parts.append(int(item))
                    except ValueError:
                        raise ValueError(f"Invalid format in mapfile: {mapinfo[0]}")

            if parts and len(parts) == 2 + 5 * maplen:
                mapinfo_list.append(parts)
            else:
                raise ValueError(f"Incorrect parameter count in mapfile: {mapinfo[0]}")

    return mapinfo_list


def add_map_info(map_dict: dict, mapinfo: list) -> dict:
    if (len(mapinfo) - 2) % 5 != 0:
        raise ValueError(f"mapfile parameter number is wrong!\nwrong line: {mapinfo}")

    for i in range((len(mapinfo) - 2) // 5):
        device_id = mapinfo[5 * i + 2]
        if not device_id in DEV_DICT:
            raise ValueError(
                f"Please add device ID {device_id} to DEV_DICT!\nline: {mapinfo}"
            )

        key_tuple = tuple(mapinfo[5 * i + 2 : 5 * i + 6])
        child_key = mapinfo[5 * i + 6]
        child_value = f"{mapinfo[0]}-{mapinfo[1]}[{i}]"

        if key_tuple in map_dict:
            if child_key in map_dict[key_tuple]:
                raise ValueError(
                    f"Error: duplicate map entries for {key_tuple + (child_key,)}"
                )
            map_dict[key_tuple][child_key] = child_value
        else:
            map_dict[key_tuple] = {child_key: child_value}

    return map_dict


def add_tref_info(map_dict: dict) -> dict:
    tref_path = os.path.join(ARTHOME, "steering/tref.yaml")
    if not os.path.exists(tref_path):
        raise FileNotFoundError(f"{tref_path} does not exist!")

    yaml = YAML()
    with open(tref_path, "r", encoding="utf-8") as file:
        try:
            tref_yaml = yaml.load(file)
        except Exception as e:
            raise Exception(f"Error reading tref.yaml: {e}")

    for processor in tref_yaml.get("Processor", []):
        refconfig = processor.get("parameter", {}).get("RefConfig")
        if not refconfig:
            raise ValueError("tref.yaml format seems wrong!    ")

        device_id, fp, det, geo, ch = refconfig
        if device_id not in DEV_DICT:
            raise ValueError(
                f"Please add device ID {device_id} to DEV_DICT!\nline: {refconfig}"
            )

        key_tuple = (device_id, fp, det, geo)
        if key_tuple in map_dict:
            if ch in map_dict[key_tuple]:
                print(f"Warning: duplicate map entry for {key_tuple + (ch,)}")
            else:
                map_dict[key_tuple][ch] = "TREF"
        else:
            raise ValueError(
                f"Could not find the module for this tref!\nline: {refconfig}"
            )

    return map_dict


def show_all_mapinfo(map_dict: dict, filename: str) -> None:
    print("\033[1m\033[4mOutput format [CatID]-[fID][ch]\033[0m\n")

    output_path = os.path.join(ARTHOME, filename)
    with open(output_path, mode="w", newline="") as file:
        writer = csv.writer(file, lineterminator="\n")
        for key_tuple, mod_dict in sorted(map_dict.items()):
            dev, fp, det, geo = key_tuple
            if det not in MODULE_DICT:
                raise ValueError(
                    f"Please add the module {det} to MODULE_DICT in this script."
                )

            header = (
                f"{key_tuple} = {DEV_DICT[dev]}, {FP_DICT[fp]}, "
                f"{MODULE_DICT[det][0]}, geo={geo}"
            )
            print(header)
            writer.writerow([header] + [""] * (UNIT - 1))

            writer_list = [""] * UNIT
            for i in range(MODULE_DICT[det][1]):
                writer_list[i % UNIT] = mod_dict.get(i, "--")
                print(f"{writer_list[i % UNIT]:<11}", end="")

                if (i + 1) % UNIT == 0 or (i + 1) == MODULE_DICT[det][1]:
                    print()
                    writer.writerow(writer_list)
                    writer_list = [""] * UNIT
            print()
            writer.writerow([""] * UNIT)

    print(f"\033[1m\033[4mInfo:\033[0m\nsaved {filename}")


if __name__ == "__main__":
    args = parse_arguments()

    # prepare map information as a dictionary
    map_dict = {}
    for mapfile in get_mapfiles(args.input_mapper):
        for mapinfo in get_mapfile_data(mapfile):
            map_dict = add_map_info(map_dict, mapinfo)

    map_dict = add_tref_info(map_dict)

    # show the result in stdout and save file at args.output
    show_all_mapinfo(map_dict, args.output)
