import ROOT
import copy
import sys

CURRENTDIR = "/home/okawa/art_analysis/develop/develop/ana2"
MAX_LEVEL = 40

ROOT.gErrorIgnoreLevel = ROOT.kError
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)
ROOT.gStyle.SetMarkerStyle(20)
ROOT.gStyle.SetMarkerSize(1.5)

ZERO = ROOT.TF1("zero", "pol0", 0.0, 10.0)
ZERO.SetParameter(0, 0.0)
ZERO.SetLineColor(ROOT.kRed)

CANVAS = ROOT.TCanvas("canvas", "canvas", 800, 800)
CANVAS.SetGrid()


# object name should be "tel1/5"
def get_root_objects(filename):
    rootfile = ROOT.TFile(filename, "read")
    if not rootfile.IsOpen():
        print(f"failed to open {filename}")
        sys.exit()
    objects = []
    for i in range(5):
        obj = rootfile.Get(f"tel{i + 1}")
        if not obj:
            print(f"ROOT Object name tel{i + 1} is not found")
            sys.exit()
        objects.append(obj)
    copyed_objects = copy.deepcopy(objects)
    rootfile.Close()
    return copyed_objects


def draw_objects(objects, savedir, option, xmin=0.0, xmax=0.0, ymin=0.0, ymax=0.0):
    for i in range(5):
        CANVAS.Clear()
        if objects[i].InheritsFrom("TGraph"):
            objects[i].GetXaxis().SetLimits(xmin, xmax)
            objects[i].GetYaxis().SetRangeUser(ymin, ymax)
        objects[i].Draw(option)
        CANVAS.SaveAs(f"{CURRENTDIR}/{savedir}/tel{i + 1}.png")


def shift_and_scale_th2f(hist, xshift, xscale):
    if not hist.InheritsFrom("TH2F"):
        print("should be inherit from TH2F")
        return None

    binsX = hist.GetNbinsX()
    binsY = hist.GetNbinsY()
    xMin = hist.GetXaxis().GetXmin() * xscale + xshift
    xMax = hist.GetXaxis().GetXmax() * xscale + xshift
    shifted_hist = ROOT.TH2F(
        hist.GetName(),
        hist.GetTitle(),
        binsX,
        xMin,
        xMax,
        binsY,
        hist.GetYaxis().GetXmin(),
        hist.GetYaxis().GetXmax(),
    )

    for i in range(1, binsX + 1):
        for j in range(1, binsY + 1):
            content = hist.GetBinContent(i, j)
            shifted_hist.SetBinContent(i, j, content)

    return shifted_hist


if __name__ == "__main__":
    # 実験データ
    exp_hists = get_root_objects(f"{CURRENTDIR}/hist/exp/energy_timing.root")
    draw_objects(exp_hists, f"figure/exp/raw", "colz")

    # windowでの散乱
    window_cal_graphs = get_root_objects(f"{CURRENTDIR}/graph/window.root")
    draw_objects(window_cal_graphs, f"figure/window/theo", "ap", 0.0, 30.0, 0.0, 25.0)

    window_sim_hists = get_root_objects(f"{CURRENTDIR}/hist/sim/window.root")
    draw_objects(window_sim_hists, f"figure/window/sim", "colz")

    # (a, px)のシミュレーション
    apx_cal_2dgraphs = []
    for ex in range(MAX_LEVEL + 1):
        apx_cal_graphs = get_root_objects(f"{CURRENTDIR}/graph/ap{ex}.root")
        draw_objects(
            apx_cal_graphs, f"figure/sim/apx/ap{ex}", "ap", 0.0, 30.0, 0.0, 25.0
        )
        apx_cal_2dgraphs.append(apx_cal_graphs)

    apx_sim_2dgraphs = []
    for ex in range(MAX_LEVEL + 1):
        apx_sim_graphs = get_root_objects(f"{CURRENTDIR}/graph/real/ap{ex}.root")
        draw_objects(
            apx_sim_graphs, f"figure/exp/graphs/apx/ap{ex}", "ap", 0.0, 30.0, 0.0, 25.0
        )
        apx_sim_2dgraphs.append(apx_sim_graphs)

    apx_sim_2dhists = []
    for ex in range(MAX_LEVEL + 1):
        apx_sim_hists = get_root_objects(f"{CURRENTDIR}/hist/sim/ap{ex}.root")
        draw_objects(apx_sim_hists, f"figure/exp/apx/ap{ex}", "colz")
        apx_sim_2dhists.append(apx_sim_hists)

    # TOF resolutionのチェック
    colors = [
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

    for i in range(5):
        CANVAS.Clear()
        for ex in range(MAX_LEVEL + 1):
            apx_sim_2dgraphs[ex][i].SetMarkerColor(colors[ex])
            apx_sim_2dgraphs[ex][i].SetMarkerStyle(20)
            apx_sim_2dgraphs[ex][i].SetMarkerSize(0.3)
            if ex == 0:
                apx_sim_2dgraphs[ex][i].GetXaxis().SetLimits(-10.0, 40.0)
                apx_sim_2dgraphs[ex][i].GetYaxis().SetRangeUser(0.0, 25.0)
                apx_sim_2dgraphs[ex][i].Draw("ap")
            else:
                apx_sim_2dgraphs[ex][i].Draw("p same")
        # CANVAS.BuildLegend()
        CANVAS.SaveAs(f"{CURRENTDIR}/figure/exp/sim/graphs/tel{i + 1}.png")

    # 比較
    timing_shifts = [100.0, 207.0, 207.0, 202.0, 207.0]
    timing_scales = [0.5, 1.0, 1.0, 1.0, 1.0]
    for i in range(5):
        CANVAS.Clear()
        do_with_data = False
        if do_with_data:
            exp_hist = shift_and_scale_th2f(
                exp_hists[i], timing_shifts[i], timing_scales[i]
            )
            exp_hist.Draw("colz")
            # window_cal_graphs[i].GetXaxis().SetLimits(0.0, 30.0)
            # window_cal_graphs[i].GetYaxis().SetRangeUser(0.0, 25.0)
            window_cal_graphs[i].Draw("p same")
        else:
            window_cal_graphs[i].GetXaxis().SetLimits(0.0, 30.0)
            window_cal_graphs[i].GetYaxis().SetRangeUser(0.0, 25.0)
            window_cal_graphs[i].Draw("ap")
        for ex in range(MAX_LEVEL + 1):
            apx_cal_2dgraphs[ex][i].Draw("p same")

        CANVAS.SaveAs(f"{CURRENTDIR}/figure/exp/sim/tel{i + 1}.png")
