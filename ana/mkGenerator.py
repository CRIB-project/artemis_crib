import ROOT
import numpy as np
import os
import sys
import copy

CURRENT_DIR = os.path.dirname(__file__)
MAX_LEVEL = 40
NTEL = 5
BG_FACTOR = 4.42
SIM_SCALE = 1.5e-5
SCALE_FACTORS = [
    36.982545,
    54.633287,
    62.085444,
    48.770858,
    52.704735,
    50.506098,
    42.675392,
    37.507057,
    38.011826,
    21.401443,
    36.845087,
    32.023705,
    32.741086,
    18.226524,
    28.519727,
    30.878202,
    31.323751,
    26.627127,
    25.394701,
    23.673539,
    14.323031,
    24.097924,
    12.652305,
    15.346899,
    10.520887,
    19.483616,
    15.459227,
    16.390521,
    14.979996,
    12.825722,
    12.631248,
    6.778514,
    12.05896,
    5.102659,
    5.492337,
    9.130406,
    7.219848,
    3.102373,
    7.063787,
    6.688967,
    2.982683,
]
DO_A2P = False
A2P_SCALE_FACTOR = 131.664094

COLORS = [
    ROOT.kRed,
    ROOT.kGreen,
    ROOT.kBlue,
    ROOT.kYellow,
    ROOT.kMagenta,
    ROOT.kCyan,
    ROOT.kOrange,
    ROOT.kSpring,
    ROOT.kTeal,
    ROOT.kAzure,
    ROOT.kViolet,
    ROOT.kPink,
    ROOT.kRed,
    ROOT.kGreen,
    ROOT.kBlue,
    ROOT.kYellow,
    ROOT.kMagenta,
    ROOT.kCyan,
    ROOT.kOrange,
    ROOT.kSpring,
    ROOT.kTeal,
    ROOT.kAzure,
    ROOT.kViolet,
    ROOT.kPink,
    ROOT.kRed,
    ROOT.kGreen,
    ROOT.kBlue,
    ROOT.kYellow,
    ROOT.kMagenta,
    ROOT.kCyan,
    ROOT.kOrange,
    ROOT.kSpring,
    ROOT.kTeal,
    ROOT.kAzure,
    ROOT.kViolet,
    ROOT.kPink,
    ROOT.kRed,
    ROOT.kGreen,
    ROOT.kBlue,
    ROOT.kYellow,
    ROOT.kMagenta,
    ROOT.kCyan,
    ROOT.kOrange,
    ROOT.kSpring,
    ROOT.kTeal,
    ROOT.kAzure,
    ROOT.kViolet,
    ROOT.kPink,
]
# HIST1D_RANGE = [21000.0, 32000.0, 36000.0, 49000.0, 60000.0]
HIST1D_RANGE = [84000.0, 128000.0, 144000.0, 196000.0, 240000.0]

CANVAS = ROOT.TCanvas("canvas", "canvas", 800, 800)
CANVAS.SetGrid()
ROOT.gErrorIgnoreLevel = ROOT.kError
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)


def get_root_object(filename):
    rootfile = ROOT.TFile(filename, "read")
    if not rootfile.IsOpen():
        print(f"failed to open {filename}")
        sys.exit()
    objects = []
    for i in range(NTEL):
        obj = rootfile.Get(f"tel{i + 1}")
        if not obj:
            print(f"ROOT Object name tel{i + 1} is not found")
            sys.exit()
        objects.append(obj)
    copyed_object = copy.deepcopy(objects)
    rootfile.Close()
    return copyed_object


def get_graphs_from_file(filename, directoryname):
    graphs = []

    file = ROOT.TFile.Open(filename)
    if not file or file.IsZombie():
        print(f"Failed to open file: {filename}")
        return graphs

    dir = file.Get(directoryname)
    if not dir:
        print(f"Directory not found: {directoryname}")
        file.Close()
        return graphs

    for key in dir.GetListOfKeys():
        obj = key.ReadObj()
        if isinstance(obj, ROOT.TGraph):
            graphs.append(obj)

    copyed_graphs = copy.deepcopy(graphs)
    file.Close()
    return copyed_graphs


def graph_to_hist(graph, name, color):
    n = graph.GetN()
    x = graph.GetX()
    y = graph.GetY()
    hist = ROOT.TH1F(name, name, n, x[0], x[n - 1])
    for i in range(n):
        hist.SetBinContent(i + 1, y[i])
        hist.SetBinError(i + 1, 0)
    hist.SetFillColor(color)
    hist.SetFillColorAlpha(color, 0.1)
    hist.SetLineColor(color)
    return hist


if __name__ == "__main__":
    # prepare the histograms
    sim_hist2d_array = [[] for _ in range(NTEL)]

    for i in range(MAX_LEVEL + 1):
        filename = f"{CURRENT_DIR}/hist/sim/ap{i}.root"
        hists = get_root_object(filename)
        for j, hist in enumerate(hists):
            sim_hist2d_array[j].append(hist)

    exp_hist1d_array = []
    run_names = ["high", "bg"]
    for name in run_names:
        filename = f"{CURRENT_DIR}/hist/exp/{name}_proton.root"
        hists = get_root_object(filename)
        exp_hist1d_array.append(hists)

    # check 2D figures
    for i, apx_hists in enumerate(sim_hist2d_array):
        CANVAS.Clear()
        for j, hist in enumerate(apx_hists):
            hist.SetMarkerColor(COLORS[j])
            hist.SetTitle(f"(a,p{j})")
            hist.GetXaxis().SetRangeUser(-10, 50)
            if j == 0:
                hist.Draw("scat")
            else:
                hist.Draw("same scat")
        CANVAS.BuildLegend()
        CANVAS.SaveAs(f"{CURRENT_DIR}/figure/exp/sim/hist2d/tel{i + 1}.png")

    # check 1D figures
    sim_hist1d_array = [[None for _ in row] for row in sim_hist2d_array]
    sim_hist1d_all = []
    sim_a2p_array = get_root_object(f"{CURRENT_DIR}/hist/a2p/ground.root")
    total_yields = [0.0 for _ in range(NTEL)]
    a2p_yields = [0.0 for _ in range(NTEL)]
    for i, apx_hists in enumerate(sim_hist2d_array):
        CANVAS.Clear()
        add_hist = ROOT.TH1F()
        for j, hist in enumerate(apx_hists):
            hist1d = hist.ProjectionY()
            hist1d.SetTitle(f"(a,p{j})")
            hist1d.SetLineColor(COLORS[j])
            hist1d.SetFillColorAlpha(COLORS[j], 0.1)
            hist1d.Scale(SCALE_FACTORS[j])
            hist1d.SetMaximum(HIST1D_RANGE[i])
            sim_hist1d_array[i][j] = copy.deepcopy(hist1d)
            bin_min = hist1d.FindBin(5.0)
            bin_max = hist1d.GetNbinsX()
            total_yields[i] += hist1d.Integral(bin_min, bin_max)
            if j == 0:
                sim_hist1d_array[i][j].Draw("hist")
                add_hist = sim_hist1d_array[i][j].Clone()
            else:
                sim_hist1d_array[i][j].Draw("hist same")
                add_hist.Add(sim_hist1d_array[i][j])

        sim_a2p_array[i].SetLineColor(ROOT.kBlack)
        sim_a2p_array[i].SetFillColorAlpha(ROOT.kBlack, 0.1)
        sim_a2p_array[i].Scale(A2P_SCALE_FACTOR)
        sim_a2p_array[i].Draw("hist same")
        bin_min = hist1d.FindBin(5.0)
        bin_max = hist1d.GetNbinsX()
        a2p_yields[i] = sim_a2p_array[i].Integral(bin_min, bin_max)
        total_yields[i] += a2p_yields[i]
        # print(
        #    f"tel{i + 1}: ratio: {a2p_yield/total_yield[i]}, a2p {a2p_yield}, total {total_yield[i]}"
        # )
        # add_hist.Add(sim_a2p_array[i])
        sim_hist1d_all.append(copy.deepcopy(add_hist))
        CANVAS.BuildLegend()
        CANVAS.SaveAs(f"{CURRENT_DIR}/figure/exp/sim/hist1d/tel{i + 1}.png")

    print("ratio of the (a, 2p) reaction over 5 MeV")
    print(f"ratio: {np.sum(a2p_yields)/np.sum(total_yields)}")

    # compare with experimental data
    proton_maxes = [40.0, 60.0, 35.0, 50.0, 60.0]
    for i in range(NTEL):
        CANVAS.Clear()
        phys_hist = copy.deepcopy(exp_hist1d_array[0][i])
        bg_hist = copy.deepcopy(exp_hist1d_array[1][i])
        bg_hist.Scale(BG_FACTOR)
        phys_hist.Add(bg_hist, -1.0)

        sim_hist1d_all[i].SetTitle(f"tel{i + 1}")
        sim_hist1d_all[i].Scale(SIM_SCALE)
        sim_hist1d_all[i].SetLineColor(ROOT.kBlue)
        sim_hist1d_all[i].SetFillColorAlpha(ROOT.kBlue, 0)
        sim_hist1d_all[i].SetMaximum(proton_maxes[i])

        sim_hist1d_all[i].Draw("hist")
        phys_hist.SetLineColor(ROOT.kRed)
        phys_hist.Draw("same")

        CANVAS.SaveAs(f"{CURRENT_DIR}/figure/exp/sim/comparison/tel{i + 1}.png")

    # make random generator
    output_root_file = ROOT.TFile(f"{CURRENT_DIR}/random_generator.root", "recreate")
    yield_maxes = [90000.0, 130000.0, 150000.0, 200000.0, 250000.0]
    for i in range(NTEL):
        CANVAS.Clear()
        output_root_file.cd()
        tel_dir = output_root_file.mkdir(f"tel{i + 1}")
        tel_dir.cd()
        graphs = []
        for j in range(MAX_LEVEL + 1):
            hist = copy.deepcopy(sim_hist1d_array[i][j])
            hist.Rebin()
            gr = ROOT.TGraph()
            gr.SetName(f"tel{i + 1}(a,p{j})")
            gr.SetTitle(f"tel{i + 1}(a,p{j})")
            gr.SetLineColor(COLORS[j])
            gr.SetMarkerColor(COLORS[j])
            gr.SetMaximum(yield_maxes[i] * 2.0)
            for k in range(hist.GetNbinsX()):
                gr.SetPoint(k, hist.GetBinCenter(k), hist.GetBinContent(k))
            gr.Write()
            graphs.append(gr)

        if DO_A2P:
            hist = copy.deepcopy(sim_a2p_array[i])
            hist.Rebin()
            gr = ROOT.TGraph()
            gr.SetName(f"tel{i + 1}(a,2p)")
            gr.SetTitle(f"tel{i + 1}(a,2p)")
            for k in range(hist.GetNbinsX()):
                gr.SetPoint(k, hist.GetBinCenter(k), hist.GetBinContent(k))
            gr.Write()
            graphs.append(gr)

        for j, graph in enumerate(graphs):
            if j == 0:
                graph.Draw("al")
            else:
                graph.Draw("l")
        CANVAS.SaveAs(f"{CURRENT_DIR}/figure/exp/sim/generator/tel{i + 1}.png")

    output_root_file.Close()

    # check the ratio
    for i in range(NTEL):
        CANVAS.Clear()
        graphs = get_graphs_from_file(
            f"{CURRENT_DIR}/random_generator.root", f"tel{i + 1}"
        )
        graphs_for_stack = []
        for j in range(len(graphs)):
            gr = ROOT.TGraph()
            graphs_for_stack.append(gr)

        for j, ene in enumerate(np.arange(5.0, 30.0, 0.1)):
            total = 0.0
            for graph in graphs:
                # val = graph.Eval(ene, 0, "S")
                val = graph.Eval(ene)
                if val < 0.0:
                    val = 0.0
                total += val

            for k, graph in enumerate(graphs):
                # raw_val = graph.Eval(ene, 0, "S")
                raw_val = graph.Eval(ene)
                ratio = 0.0
                if raw_val < 0.0:
                    raw_val = 0.0
                elif raw_val > 0.001:
                    ratio = raw_val / total

                if k == 0 and total < 0.001:
                    ratio = 1.0

                graphs_for_stack[k].SetPoint(j, ene, ratio)

        hstack = ROOT.THStack()
        for j, gr in enumerate(graphs_for_stack):
            if j == len(graphs_for_stack) - 1:
                hist = graph_to_hist(gr, f"tel{i + 1}(a,p{j})", ROOT.kBlack)
                hstack.Add(hist)
            else:
                hist = graph_to_hist(gr, f"tel{i + 1}(a,p{j})", COLORS[j])
                hstack.Add(hist)

        hstack.Draw("hist")
        CANVAS.SaveAs(f"{CURRENT_DIR}/figure/exp/sim/ratio/tel{i + 1}.png")
