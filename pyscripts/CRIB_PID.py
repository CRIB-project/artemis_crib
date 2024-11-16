import os
import sys
import pyTSrim as sr
import pyMass as mass
import matplotlib.pyplot as plt
from ruamel.yaml import YAML
import numpy as np

# Global constant
ART_HOME = os.getenv("ARTEMIS_WORKDIR")
if not ART_HOME:
    raise EnvironmentError("command [artlogin user] needed")

TSRIM_HOME = os.getenv("TSRIM_DATA_HOME")
if not TSRIM_HOME:
    raise EnvironmentError("Please set the TSrim environment variable!")

# configure matplotlib
plt.rcParams["font.family"] = "Liberation Sans"
plt.rcParams["font.size"] = 16
# plt.rcParams["axes.labelweight"] = "bold"
plt.rcParams["axes.labelsize"] = 20
plt.rcParams["axes.titlesize"] = 20
plt.rcParams["legend.fontsize"] = 14
plt.rcParams["legend.title_fontsize"] = 14

# Constants related to CRIB PID calculation
LIGHTSPEED = 299792458.0  # m/s
KJ2MEV = 6.2415064e12  # J -> MeV
UNITCHARGE = 1.60217663e-19  # C (A s)
LENGTHF0F2 = 7.8  # m, length between F0 and F2
LENGTHF0F3 = 12.97  # m, length between F0 and F3
SEC2NSEC = 1.0e9  # s -> ms
MM2M = 1.0e-3  # mm -> m, also used for um -> mm


class CRIBPID:
    """
    PID calculation at F2 and F3 focal plane.
    It is almost the same with S. Hayakawa-san's application
    https://www.cns.s.u-tokyo.ac.jp/crib/cgi-bin/pid/CRIB_PID_calc.html
    """

    def __init__(self, config_path: str):
        if not config_path:
            raise ValueError(
                "configuration path argument is required and cannot be None"
            )
        if not os.path.exists(config_path):
            raise FileNotFoundError(f"{config_path} does not exist!")

        self.config_path = config_path
        self.__load_yaml()
        self.__set_datapath()

        self.f2_datas = None
        self.f3_datas = None

    def __load_yaml(self) -> None:
        yaml = YAML()
        try:
            with open(self.config_path, "r", encoding="utf-8") as file:
                pid_yaml = yaml.load(file)
        except (YAML.YAMLError, FileNotFoundError) as e:
            raise ValueError(f"Error parsing YAML file '{self.config_path}': {e}")

        self.__set_parameters(pid_yaml)

    def __set_parameters(self, pid_yaml) -> None:
        """
        Set parameters based on YAML configuration
        """
        # set ions information
        self.ion_list = pid_yaml["input_ions"]

        f1_yaml = pid_yaml["f1_parameters"]
        self.brho = f1_yaml["brho"]
        self.degrader_material = f1_yaml["degrader_material"]
        self.degrader_thickness = f1_yaml["degrader_thickness"] * MM2M
        self.rf_period = f1_yaml["rf_period"]

        # F2 parameters
        f2_yaml = pid_yaml["f2_parameters"]
        self.ppac_thickness = f2_yaml["PPAC_thickness"] * MM2M
        self.ssd_thickness = f2_yaml["SSD_thickness"] * MM2M
        self.f2_rf_offset = f2_yaml["rf_offset"]
        self.f2_rf_range = f2_yaml["rf_range"]
        self.f2_energy_range = f2_yaml["energy_range"]

        # F3 parameters
        f3_yaml = pid_yaml["f3_parameters"]
        self.a_thickness = f3_yaml["a_thickness"] * MM2M
        self.b_thickness = f3_yaml["b_thickness"] * MM2M
        self.ab_distance = f3_yaml["distance"]
        self.trigger_flag = f3_yaml["trigger"]
        self.f3_rf_offset = f3_yaml["rf_offset"]
        self.f3_tof_offset = f3_yaml["tof_offset"]
        self.f3_rf_range = f3_yaml["rf_range"]
        self.f3_tof_range = f3_yaml["tof_range"]

    def __set_datapath(self) -> None:
        """
        Set data paths for TSrim initialization
        """
        materials = ["mylar", "si"]
        if self.degrader_material not in materials:
            materials.append(self.degrader_material)

        self.datapaths = []
        for mat in materials:
            filename = f"range_fit_pol16_{mat}.txt"
            data_path = os.path.join(TSRIM_HOME, mat, filename)
            if not os.path.exists(data_path):
                raise FileNotFoundError(f"TSrim Error: {data_path} does not exist!")
            self.datapaths.append(data_path)

    def calculation(self) -> None:
        """
        Main PID calculation!
        set data points
        """
        f2_datas = []
        f3_datas = []

        # TSrim initialization
        srim = sr.TSrim()
        for path in self.datapaths:
            srim.AddElement("srim", 16, path)

        self.__print_headers()
        for ion in self.ion_list:
            z, a, q, color = ion
            m = mass.Mass(z, a) * mass.amu

            # calculate the kinetic energy from Brho
            Brhoqc = self.brho * q * UNITCHARGE * LIGHTSPEED * KJ2MEV
            beam_energy = m * (np.sqrt(1.0 + (Brhoqc / m) ** 2) - 1.0)
            E_after_degrader = srim.EnergyNew(
                z, a, beam_energy, self.degrader_material, self.degrader_thickness
            )

            # calculate energy loss
            E_after_f2ppac = srim.EnergyNew(
                z, a, E_after_degrader, "mylar", self.ppac_thickness
            )
            E_in_f2ssd = E_after_f2ppac - srim.EnergyNew(
                z, a, E_after_f2ppac, "si", self.ssd_thickness
            )

            E_after_ppaca = srim.EnergyNew(
                z, a, E_after_degrader, "mylar", self.a_thickness
            )
            E_after_ppacb = srim.EnergyNew(
                z, a, E_after_ppaca, "mylar", self.b_thickness
            )

            # calculate tof (ns)
            tof_f0f2 = SEC2NSEC * LENGTHF0F2 / self.__MeV_to_ms(beam_energy, m)
            tof_f0f3 = SEC2NSEC * LENGTHF0F3 / self.__MeV_to_ms(beam_energy, m)
            tof_ppacs = (
                SEC2NSEC * self.ab_distance * MM2M / self.__MeV_to_ms(E_after_ppaca, m)
            )

            self.__print_results(
                a,
                z,
                q,
                beam_energy,
                E_after_degrader,
                E_after_f2ppac,
                E_in_f2ssd,
                tof_f0f2,
                tof_f0f3,
                tof_ppacs,
                E_after_ppacb,
            )

            # CRIB use RF as start: f2/f3, stop: f0, so tof is negative value
            f2_datas.append([z, a, q, -tof_f0f2, E_in_f2ssd, color])
            f3_datas.append([z, a, q, -tof_f0f3, tof_ppacs, color])

        self.__print_conditions()

        self.f2_datas = f2_datas
        self.f3_datas = f3_datas

    def __MeV_to_ms(self, energy: float, mass: float) -> float:
        """
        energy: MeV, mass: MeV/c**2
        MeV -> m/s
        """
        return LIGHTSPEED * np.sqrt(1.0 - (1.0 / (energy / mass + 1.0)) ** 2)

    def __print_headers(self) -> None:
        print("[Result]")
        header_titles = [
            "",
            "aft F0tar",
            "aft F1deg",
            "bfr f2SSD",
            "f2SSD",
            "F0-F2",
            "F0-F3",
            "bet a/b",
            "bfr F3SSD",
        ]
        header_names = [
            "Ion name:",
            "E (MeV)",
            "E (MeV)",
            "E (MeV)",
            "E (MeV)",
            "TOF (ns)",
            "TOF (ns)",
            "TOF (ns)",
            "E (MeV)",
        ]
        # Join the headers and names into aligned strings
        header_line = "".join([title.center(10) for title in header_titles])
        name_line = "".join([name.center(10) for name in header_names])

        # Print them out
        print(header_line)
        print(name_line)

    def __print_results(
        self,
        a,
        z,
        q,
        beam_energy,
        E_after_degrader,
        E_after_f2ppac,
        E_in_f2ssd,
        tof_f0f2,
        tof_f0f3,
        tof_ppacs,
        E_after_ppacb,
    ):
        """
        Prints the results of the calculations.
        """
        print(
            f"{a}{mass.GetEl(z)}{q}+ :".center(10)
            + f"{beam_energy:.2f}".center(10)
            + f"{E_after_degrader:.2f}".center(10)
            + f"{E_after_f2ppac:.2f}".center(10)
            + f"{E_in_f2ssd:.2f}".center(10)
            + f"{tof_f0f2:.2f}".center(10)
            + f"{tof_f0f3:.2f}".center(10)
            + f"{tof_ppacs:.2f}".center(10)
            + f"{E_after_ppacb:.2f}".center(10)
        )

    def __print_conditions(self) -> None:
        """
        Prints the experimental conditions.
        """
        print(
            "[condition]\n"
            f"  Brho = {self.brho:.5f} Tm\n"
            f"  RF period = {self.rf_period:.1f} ns\n"
            f"  F1: Degrader material: {self.degrader_material}\n"
            f"      Degrader thickness = {self.degrader_thickness / MM2M} um\n"
            f"  F2: PPAC thickness = {self.ppac_thickness / MM2M:.1f} um (equiv. mylar)\n"
            f"      SSD thickness = {self.ssd_thickness / MM2M:.1f} um\n"
            f"  F3: PPACa/MWDCa thickness = {self.a_thickness / MM2M:.1f} um (equiv. mylar)\n"
            f"      Distance between a and b = {self.ab_distance:.1f} mm\n"
        )

    def show_f2_pid(self, figure_path: str = f"{ART_HOME}/pyscripts/out/f2_pid.png"):
        if self.f2_datas == None:
            raise ValueError("need calculation first")

        self.__generate_pid_plot(
            data=self.f2_datas,
            rf_offset=self.f2_rf_offset,
            rf_range=self.f2_rf_range,
            y_range=self.f2_energy_range,
            titles=[
                "F2 PID (E-TOF)",
                r"$T_{F2PPAC-RF}$ (ns)",
                r"$E_{SSD\ after\ F2PPAC}$ (MeV)",
            ],
            path=figure_path,
        )

    def show_f3_pid(self, figure_path: str = f"{ART_HOME}/pyscripts/out/f3_pid.png"):
        if self.f3_datas == None:
            raise ValueError("need calculation first")

        self.__generate_pid_plot(
            data=self.f3_datas,
            rf_offset=self.f3_rf_offset,
            rf_range=self.f3_rf_range,
            y_range=self.f3_tof_range,
            titles=["F3 PID (TOF-TOF)", r"$T_{F3PPAC-RF}$ (ns)", r"$T_{a/b}$ (ns)"],
            path=figure_path,
            y_offset=self.f3_tof_offset,
        )

    def __generate_pid_plot(
        self, data, rf_offset, rf_range, y_range, titles, path, y_offset=0
    ):
        """
        Generates a PID plot.
        """
        x, y, labels, colors = [], [], [], []

        for datum in data:
            z, a, q, x_data, y_data, color = datum
            x_data += rf_offset
            y_data += y_offset

            x_data = self.__adjust_rf_range(x_data, rf_range)
            while x_data < rf_range[1]:
                x.append(x_data)
                x_data += self.rf_period
                y.append(y_data)
                colors.append(color)
                labels.append(rf"$^{{{a}}}${mass.GetEl(z)}$^{{{q}+}}$")

        self.__plot_and_save(x, y, labels, colors, titles, rf_range, y_range, path)

    def __adjust_rf_range(self, x_data, rf_range):
        """
        Adjusts RF range for the plot.
        """
        if x_data < rf_range[0]:
            while x_data < rf_range[0]:
                x_data += self.rf_period
        else:
            while x_data > rf_range[0]:
                x_data -= self.rf_period
            x_data += self.rf_period
        return x_data

    def __plot_and_save(self, x, y, labels, colors, titles, rf_range, y_range, path):
        """
        Plots and saves the PID plot.
        """
        color_map = {0: "red", 1: "blue", 2: "black"}

        plt.figure(figsize=(8, 8))
        for i in range(len(x)):
            plt.scatter(x[i], y[i], color=color_map[colors[i]], s=100)
            plt.text(x[i] + 0.1, y[i], labels[i], fontsize=15)

        plt.title(titles[0])
        plt.xlabel(titles[1])
        plt.ylabel(titles[2])
        plt.xlim(rf_range)
        plt.ylim(y_range)
        plt.grid(which="both", linestyle="--", linewidth="0.2", color="gray")
        plt.minorticks_on()
        plt.grid(which="minor", linestyle=":", linewidth="0.1", color="gray")
        plt.savefig(path)
        plt.show()
        print(f"Saved {path}")
        plt.clf()

    def check_vars(self) -> None:
        """
        For debug, show all class members
        """
        print("[debug]")
        for key, value in self.__dict__.items():
            print(f"  {key}: {value}")

    def usage(self) -> None:
        print(
            "[usage]\n"
            "  obj = CRIBPID('path to the config yaml, ex ../prm/pid/hoge.yaml')\n"
            "  obj.calculation()\n"
            "  obj.show_f2_pid() # if you want to save figure"
        )


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python CRIB_PID.py <path to config yaml>")
        sys.exit(1)

    obj = CRIBPID(sys.argv[1])
    obj.calculation()
    obj.show_f2_pid()
    obj.show_f3_pid()
