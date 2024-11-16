import os
import numpy as np
import pyTSrim as sr
import pyMass as mass
import ROOT


class EffectiveThickness:
    """
    Calculate the effective thickness of the gas target.

    Parameters:
        beam_z (int)       : Z number of beam ion.
        beam_a (int)       : A number of beam ion.
        target_name (str)  : target name used in TSrim library
        mol_weight (float) : Molecular weight of the gas (g/mol ~ amu)
        target_z (int)     : Z number of target ion to calculate Ecm
        target_a (int)     : A number of target ion to calculate Ecm
        pressure (float)   : target gas pressure (Torr)
                             if not, use 760.0 Torr (defined TSrim::P1)
        temperature (float): target gas temperature (K)
                             if not, use 273.15 K (defined TSrin::E0)

        e_min (float)      : minimum Ecm to calculate effective thickness
        e_max (float)      : maximum Ecm to calculate effective thickness
        delta (float)      : energy width of effective thickness
        unit_type (str)    : unit of effective thickness (mm, g/cm2, /cm2)

    Usage:
        > ins = EffectiveThickness()
        > ins.set_beam_parameters(beam_z, beam_a)
        > ins.set_target_parameters(target, mol_weight, target_z, target_a, pressure, temperature)
        > obj = ins.get_thickness(e_min, e_max, delta, unit_type)

    """

    TORR2PA = 133.322  # Torr -> Pa
    AVOGADRO = 6.0221367e23  # Avogadro's number (/mol)
    BOLTZMANN = 1.380658e-23  # Boltzmann constant (J/K)
    ESTEP = 0.01  # MeV

    def __init__(self):
        tsrim_home = os.getenv("TSRIM_DATA_HOME")
        if not tsrim_home:
            raise EnvironmentError("Please set the TSrim environment variable!")
        self.tsrim_home = tsrim_home

        self.beam_z = None
        self.beam_a = None
        self.target_name = None
        self.mol_weight = None
        self.target_z = None
        self.target_a = None
        self.pressure = None
        self.temperature = None
        self.datapath = None
        self.do_beam_init = False
        self.do_target_init = False

    def set_beam_parameters(self, z, a) -> None:
        if self.do_beam_init:
            self.show_help()
            raise ValueError("Beam information is already registered")
        if not isinstance(z, int):
            raise TypeError("beam_z must be an int")
        if not isinstance(a, int):
            raise TypeError("beam_a must be an int")

        self.beam_z = z
        self.beam_a = a
        self.do_beam_init = True

    def set_target_parameters(
        self, target, mol_weight, z, a, pressure=None, temperature=None
    ) -> None:
        if self.do_target_init:
            self.show_help()
            raise ValueError("Beam and target information is already registered")
        if not isinstance(target, str):
            raise TypeError("target must be a str")
        if not isinstance(mol_weight, float):
            raise TypeError("mol-weight must be a float")
        if not isinstance(z, int):
            raise TypeError("target_z must be an int")
        if not isinstance(a, int):
            raise TypeError("target_a must be an int")
        if not isinstance(pressure, float):
            raise TypeError("pressure must be a float")
        if not isinstance(temperature, float):
            raise TypeError("temperature must be a float")

        self.target_name = target
        self.mol_weight = mol_weight
        self.target_z = z
        self.target_a = a
        self.pressure = pressure or sr.P1
        self.temperature = temperature or sr.T0

        self.__set_datapath()

    def __set_datapath(self) -> None:
        filename = f"range_fit_pol16_{self.target_name}.txt"
        data_path = os.path.join(self.tsrim_home, self.target_name, filename)
        if not os.path.exists(data_path):
            raise FileNotFoundError(f"{data_path} does not exist!")

        self.datapath = data_path
        self.do_target_init = True

    def show_help(self) -> None:
        message = (
            "Usage:\n"
            "> ins = EffectiveThickness()\n"
            "> ins.set_beam_parameters(beam_z, beam_a)\n"
            "> ins.set_target_parameters(target, target_z, target_a, pressure, temperature)\n"
            "> obj = ins.get_thickness(e_min, e_max, delta, unit_type)"
        )
        print(message)

    def show_status(self) -> None:
        do_init = self.do_beam_init and self.do_target_init
        status = {
            "do_init": do_init,
            "beam_ion": (
                f"{self.beam_a}{mass.GetEl(self.beam_z)}{self.beam_z}"
                if self.do_beam_init
                else None
            ),
            "target": self.target_name,
            "molecular weight": self.mol_weight,
            "target_ion": (
                f"{self.target_a}{mass.GetEl(self.target_z)}{self.target_z}"
                if self.do_target_init
                else None
            ),
            "pressure": self.pressure,
            "temperature": self.temperature,
            "gas_density": self.gas_density() if self.do_target_init else None,
            "datafile": self.datapath,
        }
        for key, value in status.items():
            print(f"{key}: {value}")

        if not do_init:
            self.show_help()

    def get_thickness_gr(
        self, e_min: float, e_max: float, delta: float, unit_type="/cm2"
    ) -> ROOT.TGraph:
        """
        return ROOT TGraph
        """
        if not self.do_beam_init or not self.do_target_init:
            raise ValueError("not initialized the instance")

        if e_max - e_min < self.ESTEP:
            raise ValueError(f"e_max - e_min should be larger than {self.ESTEP}")

        density = self.gas_density()
        graph_title = f"dE = {delta} MeV;Ecm (MeV);"
        if unit_type == "mm":
            # default mm
            thickness_factor = 1.0
        elif unit_type == "g/cm2":
            # (mm->cm) * g/cm3 => g/cm2
            thickness_factor = 1.0e-1 * density
        elif unit_type == "/cm2":
            # (mm->cm) * g/cm3 * /mol / g/mol => /cm2
            thickness_factor = 1.0e-1 * density * self.AVOGADRO / self.mol_weight
        else:
            raise ValueError(
                f"{unit_type} unit is not supported.\nuse mm, g/cm2 or /cm2"
            )

        graph_title += f"effective thickness ({unit_type})"
        print(f"calculate effective thickness unit: {unit_type}")
        gr = ROOT.TGraph()
        gr.SetTitle(graph_title)

        # TSrim initialization
        srim = sr.TSrim()
        srim.AddElement("srim", 16, self.datapath, self.beam_z, self.beam_a)

        for i, e in enumerate(np.arange(e_min, e_max, self.ESTEP)):
            e_low = e - delta / 2.0
            e_high = e + delta / 2.0
            if e_low < 0.0:
                e_low = 0.0

            # default thickness is mm
            thickness = srim.Range(
                self.beam_z,
                self.beam_a,
                self.ecm_to_elab(e_high),
                self.target_name,
                self.pressure,
                self.temperature,
            ) - srim.Range(
                self.beam_z,
                self.beam_a,
                self.ecm_to_elab(e_low),
                self.target_name,
                self.pressure,
                self.temperature,
            )

            gr.SetPoint(i, e, thickness * thickness_factor)

        return gr

    def gas_density(self) -> float:
        """
        ideal gas
        return gas density (g/cm3)
        """
        p = self.pressure * self.TORR2PA
        # unit g/cm3
        return (
            (self.mol_weight * p)
            / (self.BOLTZMANN * self.temperature * self.AVOGADRO)
            * 1.0e-6
        )

    def ecm_to_elab(self, ecm: float) -> float:
        """
        kinematics calculation

        using Lorentz invariant (c = 1 unit)
        Ecm^2 = (E1lab + E2lab)^2 - (p1lab + p2lab)^2
        (Ekincm + m1 + m2)^2 = (E1lab + m2)^2 - p1lab^2
        (Ekincm + m1 + m2)^2 = (E1lab + m2)^2 - (E1lab^2 - m1^2)
        ...
        E1kinlab = E1lab - m1 = ...

        from this relationship, the Ecm2Elab is calculated
        """
        beam_mass = mass.Mass(self.beam_z, self.beam_a) * mass.amu
        target_mass = mass.Mass(self.target_z, self.target_a) * mass.amu

        tmp = ecm * ecm + 2.0 * (
            ecm * (beam_mass + target_mass) + beam_mass * target_mass
        )
        return tmp / (2.0 * target_mass) - beam_mass


if __name__ == "__main__":
    ef = EffectiveThickness()
    ef.set_beam_parameters(14, 26)
    ef.set_target_parameters("he", mass.Mass(2, 4), 2, 4, 250.0, 300.0)
    ef.show_status()

    do_test = True
    if do_test:
        canvas = ROOT.TCanvas("c1", "c1", 800, 800)
        canvas.Clear()
        g = ef.get_thickness_gr(0.5, 8.0, 0.5, "/cm2")
        g.Draw("apl")
        canvas.Update()
        canvas.SaveAs("c1.png")
