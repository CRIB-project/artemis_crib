import ROOT
import argparse
import os
import sys
from sympy import symbols
from sympy.geometry import Point, Polygon
from sympy.geometry.util import convex_hull
import itertools
from ruamel.yaml import YAML
from ruamel.yaml.scalarstring import PreservedScalarString as PSS
import random


def get_argperser_setting():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        "-i",
        "--input-tcutg-file",
        type=str,
        help="input the ROOT file name contain TCutG object, no need gate/ (ex test.root)",
        required=True,
    )
    argparser.add_argument(
        "-o",
        "--output-hist-yaml",
        type=str,
        help="input the output hist yaml file. (ex. mwdc/mwdc.yaml)",
        required=True,
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


def is_valid(p):
    if p.is_convex():
        return True
    if len(p.vertices) != len(set(p.vertices)):
        return False
    for s1, s2 in itertools.combinations(p.sides, 2):
        cp = s1.intersection(s2)
        if len(cp) == 0:
            continue
        if cp[0] in s1.points and cp[0] in s2.points:
            continue
        else:
            return False
    return True


def get_point_inside(polygon, p1, p2):
    mid_point = p1.midpoint(p2)
    offset_x = 1.0e-3 * random.random()
    offset_y = 1.0e-3 * random.random()
    p_plus = Point(mid_point.x + offset_x, mid_point.y + offset_y)
    p_minus = Point(mid_point.x - offset_x, mid_point.y - offset_y)
    if polygon.encloses_point(p_plus):
        p = p_plus
    elif polygon.encloses_point(p_minus):
        p = p_minus
    else:
        sys.exit("Cannot find internal point, please try again")
    return p


def get_convex_array(polygon):
    poly_array = []
    if polygon.is_convex():
        poly_array.append(polygon)
    else:
        points = list(polygon.vertices)
        triangles = []

        while len(points) > 3:
            for i in range(len(points)):
                p1, p2, p3 = (
                    points[i],
                    points[(i + 1) % len(points)],
                    points[(i + 2) % len(points)],
                )
                triangle = Polygon(p1, p2, p3)
                p_in = get_point_inside(triangle, p1, p2)

                if all(
                    not triangle.encloses_point(p)
                    for p in points
                    if p not in [p1, p2, p3]
                ) and polygon.encloses_point(p_in):
                    triangles.append(triangle)
                    del points[(i + 1) % len(points)]
                    break

        triangles.append(Polygon(*points))

        while triangles:
            triangle = triangles.pop(0)
            for other in triangles:
                triangle = convex_hull(triangle, other)
                triangles.remove(other)
                break

            poly_array.append(triangle)

    return poly_array


def get_inequalities(points):
    inequalities = []
    poly = Polygon(*[Point(p) for p in points])
    if not is_valid(poly):
        sys.exit("Invalid TCutG object (it has intersection)")

    polys = get_convex_array(poly)
    if any(not pol.is_convex for pol in polys):
        sys.exit("failed to make convex object")

    for pol in polys:
        pref1, pref2 = pol.vertices[0], pol.vertices[1]
        p_inside = get_point_inside(pol, pref1, pref2)
        convex_inequalities = []
        for side in pol.sides:
            p1, p2 = side.p1, side.p2
            if p1.x == p2.x:
                inequality = x <= p1.x if p_inside.x < p1.x else x >= p1.x
            elif p1.y == p2.y:
                inequality = y <= p1.y if p_inside.y < p1.y else y >= p1.y
            else:
                slope = (p2.y - p1.y) / (p2.x - p1.x)
                slope = round(float(slope), 5)
                intercept = p1.y - slope * p1.x
                intercept = round(float(intercept), 5)
                inequality = (
                    y >= slope * x + intercept
                    if p_inside.y > slope * p_inside.x + intercept
                    else y <= slope * x + intercept
                )
            convex_inequalities.append(inequality)
        inequalities.append(convex_inequalities)

    return inequalities


if __name__ == "__main__":
    try:
        art_work_dir = os.environ["ARTEMIS_WORKDIR"]
    except:
        sys.exit("command [artlogin user] needed")

    perser = get_argperser_setting()
    args = perser.parse_args()
    inputname = art_work_dir + "/gate/" + args.input_tcutg_file
    outputname = art_work_dir + "/hist/" + args.output_hist_yaml

    if not os.path.isfile(inputname):
        sys.exit("Input File " + args.input_tcutg_file + " does not exists")

    if not os.path.isfile(outputname):
        sys.exit("Output File " + args.output_hist_yaml + " does not exists")

    infile = ROOT.TFile(inputname)
    cut = infile.Get(os.path.basename(inputname).split(".", 1)[0])

    if not cut:
        infile.Close()
        sys.exit(
            "TCutG "
            + cut
            + "does not exists\nNOTE: filename and TCutG object name should be same"
        )

    infile.Close()

    yaml = get_yaml_setting()
    with open(outputname, "r", encoding="utf-8") as fin:
        content = yaml.load(fin)

    if "alias" not in content:
        print(
            "alias node does not exist in hist yaml, "
            + os.path.basename(outputname)
            + "."
        )
        sys.exit()

    print("Get TCutG object")
    print("\tName: " + cut.GetName())
    print("\tXVar: " + cut.GetVarX())
    print("\tYVar: " + cut.GetVarY())

    points = []
    for i in range(cut.GetN()):
        points.append((cut.GetPointX(i), cut.GetPointY(i)))
    points.pop()

    x, y = symbols(cut.GetVarX() + " " + cut.GetVarY())
    result_inequalities = get_inequalities(points)

    print("Write " + outputname)
    index = 0
    for inequalities in result_inequalities:
        yaml_value = ""
        for inequality in inequalities:
            yaml_value += str(inequality) + " && "

        yaml_value = yaml_value[:-4]
        yaml_value += ";"

        print("\tadd " + cut.GetName() + "_sub" + str(index) + " alias cut line")
        content["alias"][cut.GetName() + "_sub" + str(index)] = PSS(yaml_value)
        index += 1

    total_value = ""
    for i in range(index):
        total_value += cut.GetName() + "_sub" + str(i) + " || "

    total_value = total_value[:-4]
    total_value += ";"

    print("\tadd " + cut.GetName() + " alias cut line")
    content["alias"][cut.GetName()] = PSS(total_value)

    with open(outputname, "w", encoding="utf-8") as fout:
        yaml.dump(content, fout)
