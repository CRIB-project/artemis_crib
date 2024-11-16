import sys
import os
import argparse
import glob
from ruamel.yaml import YAML


def get_argperser_setting():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        "-a", "--all", action="store_true", help="modify all hist yaml files"
    )
    argparser.add_argument(
        "-i",
        "--input-yaml",
        type=str,
        default="-",
        help="specify one of the hist yaml to modify (relative path is okay)",
    )
    return argparser


def get_yaml_setting():
    yaml = YAML()
    yaml.default_flow_style = False
    # yaml.allow_duplicate_keys = True
    yaml.allow_unicode = True
    yaml.preserve_quotes = True
    # yaml.width = True
    # yaml.indent = True
    return yaml


def title_generator(yaml, filepath):
    print("processing: " + filepath)
    with open(filepath, "r", encoding="utf-8") as fin:
        data = yaml.load(fin)

    for i in range(len(data["group"])):
        for j in range(len(data["group"][i]["contents"])):
            name = str(data["group"][i]["contents"][j]["x"]).split("'")[1]

            if "y" in data["group"][i]["contents"][j]:
                name += ":" + str(data["group"][i]["contents"][j]["y"]).split("'")[1]

            if "cut" in data["group"][i]["contents"][j]:
                name += "{" + str(data["group"][i]["contents"][j]["cut"]) + "}"

            data["group"][i]["contents"][j]["name"] = name
            data["group"][i]["contents"][j]["title"] = name

    with open(filepath, "w", encoding="utf-8") as fout:
        yaml.dump(data, fout)


if __name__ == "__main__":
    try:
        art_work_dir = os.environ["ARTEMIS_WORKDIR"]
    except:
        sys.exit("command [artlogin user] needed")

    yaml = get_yaml_setting()
    perser = get_argperser_setting()
    args = perser.parse_args()

    if args.all:
        print("change all hist yaml? [y/n]", end=" ")
        answer = input()
        if answer == "y":
            for f in glob.glob(art_work_dir + "/hist/**/*.yaml", recursive=True):
                title_generator(yaml, f)
        else:
            print("cancelled")
            sys.exit()

    elif not args.input_yaml == "-":
        title_generator(yaml, args.input_yaml)

    else:
        perser.print_help()
