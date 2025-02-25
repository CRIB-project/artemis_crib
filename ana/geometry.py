import math
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np
import yaml


class Diagram:
    def __init__(self, params):
        # get parameters
        self.params = params
        diagram_params = params.get("diagram", {})
        self.output_name = diagram_params.get("output_name", "./geo")
        self.x_range = diagram_params.get("x_range", 10)
        self.y_range = diagram_params.get("y_range", 6)
        self.normal_text_size = diagram_params.get("font_size", 6)
        self.font_family = diagram_params.get("font_family", "Liberation Sans")
        self.scale_text_size = diagram_params.get("scale_text_size", 4)

        # size setting
        self.width_cm = diagram_params.get("width_cm", 9)
        self.width_inch = self.width_cm / 2.54
        self.aspect_ratio = self.y_range / self.x_range
        self.height_inch = self.width_inch * self.aspect_ratio

        # font setting
        plt.rcParams["font.family"] = self.font_family
        plt.rcParams["font.size"] = self.normal_text_size
        plt.rcParams["text.color"] = "black"

        # initialize canvas
        self.fig, self.ax = plt.subplots(figsize=(self.width_inch, self.height_inch))
        self.ax.set_xlim((0, self.x_range))
        self.ax.set_ylim((0, self.y_range))
        self.ax.axis("off")
        self.ax.set_aspect("equal")

        # get y center
        self.y_center = self.y_range / 2.0

    def center_dotline_draw(self, width=1):
        self.draw_dotted_line(0.0, self.y_center, self.x_range, self.y_center, width)

    def beam_draw(self):
        beam_params = self.params.get("beam", {})
        beam = Beam(self, **beam_params)
        beam.draw()

    def ppac_draw(self):
        ppac_params = self.params.get("ppac", {})
        ppac = PPAC(self, **ppac_params)
        ppac.draw()
        self.b_center = ppac.b_center
        self.arrow_height = ppac.arrow_height
        self.scale_height = ppac.scale_height

    def chamber_draw(self):
        chamber_params = self.params.get("chamber", {})
        chamber = Chamber(self, **chamber_params)
        chamber.draw()
        self.chamber_xmin = chamber.xmin

    def si_detectors_draw(self):
        si_detector_params = self.params.get("si_detectors", {})
        si_detectors = SiDetectors(self, **si_detector_params)
        si_detectors.draw()

    def save_figure(self):
        self.fig.savefig(
            f"{self.output_name}.png",
            format="png",
            dpi=300,
            bbox_inches="tight",
        )
        self.fig.savefig(
            f"{self.output_name}.eps",
            format="eps",
            dpi=300,
            bbox_inches="tight",
        )

    def draw_dotted_line(self, xmin, ymin, xmax, ymax, linewidth=1):
        self.ax.plot([xmin, xmax], [ymin, ymax], "k--", linewidth=linewidth)

    def draw_beam(self):
        x_start = self.x_range[0]
        x_end = self.x_range[1] / 2.0
        self.draw_single_arrow(x_start, self.y_center, x_end, self.y_center, "red", 1)
        self.draw_text(
            x_end - self.x_range[1] / 9.0,
            self.y_center + self.y_range[1] / 25.0,
            r"$^{26}$Si Beam",
            self.normal_text_size,
            "red",
        )

    def draw_single_arrow(
        self, x_start, y_start, x_end, y_end, color="red", linewidth=1
    ):
        self.ax.annotate(
            "",
            xy=(x_end, y_end),
            xytext=(x_start, y_start),
            arrowprops=dict(edgecolor=color, lw=linewidth, arrowstyle="->"),
        )

    def draw_text(
        self,
        x,
        y,
        text,
        fontsize=None,
        color="black",
        ha="center",
        va="center",
        background_color="white",
    ):
        if fontsize is None:
            fontsize = self.normal_text_size
        self.ax.text(
            x,
            y,
            text,
            fontsize=fontsize,
            color=color,
            ha=ha,
            va=va,
            bbox=dict(facecolor=background_color, edgecolor="none", pad=0),
        )

    def draw_rectangle(
        self,
        rect_x,
        rect_y,
        rect_width,
        rect_height,
        angle=0,
        fill=False,
        color="black",
    ):
        facecolor = color if fill else "none"
        rectangle = patches.Rectangle(
            (rect_x, rect_y),
            rect_width,
            rect_height,
            linewidth=1,
            edgecolor=color,
            facecolor=facecolor,
        )
        # 回転の設定
        t = self.ax.transData
        rotation = patches.transforms.Affine2D().rotate_deg_around(
            rect_x + rect_width / 2, rect_y + rect_height / 2, angle
        )
        rectangle.set_transform(rotation + t)
        self.ax.add_patch(rectangle)

    def draw_double_arrow(
        self,
        x_start,
        y_start,
        x_end,
        y_end,
        color="black",
        linewidth=1,
        omit_middle=False,
    ):
        self.ax.annotate(
            "",
            xy=(x_end, y_end),
            xytext=(x_start, y_start),
            arrowprops=dict(arrowstyle="<->", color=color, lw=linewidth),
        )
        if omit_middle:
            mid_x = (x_start + x_end) / 2
            mid_y = (y_start + y_end) / 2
            delta_x = x_end - x_start
            delta_y = y_end - y_start
            angle = math.degrees(math.atan2(delta_y, delta_x)) + 90
            self.draw_rotated_text(
                mid_x,
                mid_y,
                "≈",
                angle,
                fontsize=self.normal_text_size * 1.5,
                color=color,
                ha="center",
                va="center",
                background_color="none",
            )

    def draw_filled_circle(self, x, y, radius=0.06, color="black"):
        circle = patches.Circle((x, y), radius=radius, color=color)
        self.ax.add_patch(circle)

    def draw_rotated_rectangle(
        self, center_x, center_y, distance, width, height, angle, fill=True
    ):
        rect_x = center_x + distance * np.cos(np.radians(angle))
        rect_y = center_y + distance * np.sin(np.radians(angle))
        self.draw_rectangle(
            rect_x - width / 2, rect_y - height / 2, width, height, angle, fill
        )

    def draw_rotated_text(
        self,
        x,
        y,
        text,
        angle,
        fontsize=None,
        color="black",
        ha="center",
        va="center",
        background_color="white",
    ):
        if fontsize is None:
            fontsize = self.normal_text_size
        self.ax.text(
            x,
            y,
            text,
            fontsize=fontsize,
            color=color,
            ha=ha,
            va=va,
            rotation=angle,
            rotation_mode="anchor",
            bbox=dict(facecolor=background_color, edgecolor="none", pad=0),
        )


class Beam:
    def __init__(self, diagram, **params):
        self.diagram = diagram
        x_factor = diagram.x_range
        y_factor = diagram.y_range

        # get parameters
        self.x_start = params.get("x_start", 0.0) * x_factor
        self.y_start = params.get("y_start", 0.5) * y_factor
        self.x_end = params.get("x_end", 0.5) * x_factor
        self.y_end = params.get("y_end", 0.5) * y_factor
        self.color = params.get("color", "red")
        self.width = params.get("width", 1)
        self.text_offset_x = params.get("text_offset_x", 0.39) * x_factor
        self.text_offset_y = params.get("text_offset_y", 0.54) * y_factor
        self.label = params.get("label", "Beam")

    def draw(self):
        self.diagram.draw_single_arrow(
            self.x_start, self.y_start, self.x_end, self.y_end, self.color, self.width
        )
        self.diagram.draw_text(
            self.text_offset_x,
            self.text_offset_y,
            rf"{self.label}",
            self.diagram.normal_text_size,
            self.color,
        )


class PPAC:
    def __init__(self, diagram, **params):
        self.diagram = diagram
        x_factor = diagram.x_range
        y_factor = diagram.y_range

        # get parameters
        self.xmin_a = params.get("xmin_a", 0.05) * x_factor
        self.y_center = params.get("y_center", diagram.y_center)
        self.height = params.get("height", 0.3) * y_factor
        self.width = params.get("width", 0.04) * x_factor
        self.distance = params.get("distance", 0.1) * x_factor
        self.text_offset = params.get("text_offset", 0.7) * y_factor
        self.scale_height = params.get("scale_height", 0.04) * y_factor
        self.arrow_height = params.get("arrow_height", 0.09) * y_factor
        self.label_a = params.get("label_a", "PPACa")
        self.label_b = params.get("label_b", "PPACb")
        self.distance_label = params.get("distance_label", "322 mm")
        self.linewidth = params.get("linewidth", 0.5)
        self.dotted_linewidth = params.get("dotted_linewidth", 0.5)
        self.arrow_linewidth = params.get("arrow_linewidth", 1)

        self.xmin_b = self.xmin_a + self.distance
        self.a_center = self.xmin_a + self.width / 2.0
        self.b_center = self.xmin_b + self.width / 2.0

    def draw(self):
        self.diagram.draw_rectangle(
            self.xmin_a, self.y_center - self.height / 2.0, self.width, self.height
        )
        self.diagram.draw_rectangle(
            self.xmin_b, self.y_center - self.height / 2.0, self.width, self.height
        )
        self.diagram.draw_text(self.a_center, self.text_offset, self.label_a)
        self.diagram.draw_text(self.b_center, self.text_offset, self.label_b)
        self.diagram.draw_double_arrow(
            self.a_center,
            self.arrow_height,
            self.b_center,
            self.arrow_height,
            linewidth=self.arrow_linewidth,
            omit_middle=True,
        )
        self.diagram.draw_text(
            (self.a_center + self.b_center) / 2.0,
            self.scale_height,
            self.distance_label,
            self.diagram.scale_text_size,
        )
        self.diagram.draw_dotted_line(
            self.a_center,
            0.0,
            self.a_center,
            self.y_center,
            self.dotted_linewidth,
        )
        self.diagram.draw_dotted_line(
            self.b_center,
            0.0,
            self.b_center,
            self.y_center,
            self.dotted_linewidth,
        )


class Chamber:
    def __init__(self, diagram, **params):
        self.diagram = diagram
        x_factor = diagram.x_range
        y_factor = diagram.y_range

        self.b_center = getattr(diagram, "b_center", None)
        self.y_center = params.get("y_center", diagram.y_center)
        scale_height = getattr(diagram, "scale_height", 0.04 * y_factor)
        self.scale_height = (
            params.get("scale_height", scale_height / y_factor) * y_factor
        )
        arrow_height = getattr(diagram, "arrow_height", 0.09 * y_factor)
        self.arrow_height = (
            params.get("arrow_height", arrow_height / y_factor) * y_factor
        )

        self.xmin = params.get("xmin", 0.31) * x_factor
        self.ymin = params.get("ymin", 0.02) * y_factor
        self.width = x_factor - self.xmin
        self.height = y_factor - self.ymin * 2
        self.linewidth = params.get("linewidth", 0.5)
        self.arrow_linewidth = params.get("arrow_linewidth", 1)
        self.label_b_to_chamber = params.get("label_b_to_chamber", "length")

        window_params = params.get("window_params", {})
        self.exist_window = bool(window_params)
        self.window_width = window_params.get("width", 0.005) * x_factor
        self.window_height = window_params.get("height", 0.06) * y_factor
        self.window_color = window_params.get("color", "black")
        self.window_label = window_params.get("label", "window")
        self.window_fontsize = window_params.get("fontsize", 5)
        self.window_text_x = window_params.get("text_x", 0.34) * x_factor
        self.window_text_y = window_params.get("text_y", 0.444) * y_factor

    def draw(self):
        if self.b_center is not None:
            self.diagram.draw_double_arrow(
                self.b_center,
                self.arrow_height,
                self.xmin,
                self.arrow_height,
                linewidth=self.arrow_linewidth,
                omit_middle=True,
            )
            self.diagram.draw_text(
                (self.b_center + self.xmin) / 2.0,
                self.scale_height,
                self.label_b_to_chamber,
                self.diagram.scale_text_size,
            )

        self.diagram.draw_rectangle(self.xmin, self.ymin, self.width, self.height)

        # draw window
        if self.exist_window:
            self.diagram.draw_rectangle(
                self.xmin - self.window_width / 2.0,
                self.y_center - self.window_height / 2.0,
                self.window_width,
                self.window_height,
                0,
                True,
                color=self.window_color,
            )
            self.diagram.draw_text(
                self.window_text_x,
                self.window_text_y,
                self.window_label,
                fontsize=self.window_fontsize,
                color=self.window_color,
            )


class SiDetectors:
    def __init__(self, diagram, **params):
        self.diagram = diagram
        x_factor = diagram.x_range
        y_factor = diagram.y_range

        self.b_center = getattr(diagram, "b_center", None)
        self.y_center = params.get("y_center", diagram.y_center)
        self.chamber_xmin = getattr(diagram, "chamber_xmin", None)
        scale_height = getattr(diagram, "scale_height", 0.04 * y_factor)
        self.scale_height = (
            params.get("scale_height", scale_height / y_factor) * y_factor
        )
        arrow_height = getattr(diagram, "arrow_height", 0.09 * y_factor)
        self.arrow_height = (
            params.get("arrow_height", arrow_height / y_factor) * y_factor
        )

        self.draw_points = params.get("draw_points", False)
        self.radius = params.get("radius", 0.06)
        point_offsets = params.get("point_offsets", [0.5])
        self.point_offsets = np.array(point_offsets) * x_factor
        self.linewidth = params.get("linewidth", 0.5)
        self.arrow_linewidth = params.get("arrow_linewidth", 1)
        self.distance_label = params.get("distance_label", "length")
        self.points_label = params.get("points_labels", [])

        if len(self.point_offsets) - 1 != len(self.points_label):
            raise ValueError("size of point_offsets and points_label is different")

        # detector input
        self.det_height = params.get("det_height", 0.06) * y_factor
        self.det_gap = params.get("det_gap", 0.01) * x_factor
        det_centers = params.get("det_centers", [0])
        det_widths = params.get("det_widths", [[0.01]])
        det_angles = params.get("det_angles", [0.0])
        det_distances = params.get("det_distances", [0.25])
        self.det_scale_texts = params.get("det_scale_texts", ["length"])

        if (
            len(det_centers)
            != len(det_widths)
            != len(det_angles)
            != len(det_distances)
            != len(self.det_scale_texts)
        ):
            raise ValueError("size is not the same")

        self.det_centers = np.array(det_centers)
        scaled_widths = [[value * x_factor for value in row] for row in det_widths]
        self.det_widths = np.array(scaled_widths, dtype="object")
        self.det_angles = np.array(det_angles)
        self.det_distances = np.array(det_distances) * x_factor
        self.det_arrow_linewidth = params.get("det_arrow_linewidth", 1)
        self.n_det = len(self.det_centers)

    def draw(self):
        if self.draw_points:
            for x_point in self.point_offsets:
                self.diagram.draw_filled_circle(x_point, self.y_center, self.radius)

        if self.chamber_xmin is not None:
            arrow_x_min = self.chamber_xmin
        elif self.b_center is not None:
            arrow_x_min = self.b_center

        if self.chamber_xmin is not None or self.b_center is not None:
            self.diagram.draw_double_arrow(
                arrow_x_min,
                self.arrow_height,
                self.point_offsets[0],
                self.arrow_height,
                linewidth=self.arrow_linewidth,
                omit_middle=True,
            )
            self.diagram.draw_text(
                (arrow_x_min + self.point_offsets[0]) / 2.0,
                self.scale_height,
                self.distance_label,
                self.diagram.scale_text_size,
            )
            for x_point in self.point_offsets:
                self.diagram.draw_dotted_line(
                    x_point, 0.0, x_point, self.y_center, self.linewidth
                )

        for i in range(len(self.points_label)):
            self.diagram.draw_double_arrow(
                self.point_offsets[i],
                self.arrow_height,
                self.point_offsets[i + 1],
                self.arrow_height,
                linewidth=self.arrow_linewidth,
            )
            self.diagram.draw_text(
                (self.point_offsets[i] + self.point_offsets[i + 1]) / 2.0,
                self.scale_height,
                self.points_label[i],
                self.diagram.scale_text_size,
            )

        # draw detectors
        for i in range(self.n_det):
            distance = self.det_distances[i]
            angle = self.det_angles[i]
            x_center = self.point_offsets[self.det_centers[i]]
            det_center_x = x_center + distance * np.cos(np.radians(angle))
            det_center_y = self.y_center + distance * np.sin(np.radians(angle))

            self.diagram.draw_double_arrow(
                x_center,
                self.y_center,
                det_center_x,
                det_center_y,
                "black",
                self.det_arrow_linewidth,
            )
            scale_text_x = (x_center + det_center_x) / 2.0
            scale_text_y = (self.y_center + det_center_y) / 2.0
            self.diagram.draw_text(
                scale_text_x,
                scale_text_y,
                self.det_scale_texts[i],
                self.diagram.scale_text_size,
            )

            for j in range(len(self.det_widths[i])):
                self.diagram.draw_rotated_rectangle(
                    x_center,
                    self.y_center,
                    distance + self.det_widths[i][j] / 2.0,
                    self.det_widths[i][j],
                    self.det_height,
                    angle,
                )
                distance += self.det_widths[i][j] + self.det_gap


if __name__ == "__main__":
    with open("ana2/geo_parameters.yaml", "r") as f:
        params = yaml.safe_load(f)

    diagram = Diagram(params)
    diagram.center_dotline_draw(width=1)
    diagram.beam_draw()
    diagram.ppac_draw()
    diagram.chamber_draw()
    diagram.si_detectors_draw()
    diagram.save_figure()
