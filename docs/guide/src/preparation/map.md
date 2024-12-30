# Map and Seg Configuration

This section explains the necessary configurations for analyzing actual data.
The structure of the data files assumes the **RIDF** (RIKEN Data File) format, commonly used in DAQ systems at RIKEN.
Here, we focus on two essential configuration files for extracting and interpreting data: the **map file** and the **segment file**.

## The Role of Configuration Files

In `artemis`, input data is stored in an object called the `EventStore`.
To process RIDF files, the class `TRIDFEventStore` is used.
Configuration files map the raw data in `TRIDFEventStore` to the corresponding DAQ modules and detectors, enabling accurate interpretation.

## Segment Files

Segment files are used to read raw ADC or TDC data.
When the map file is not properly configured, segment files help verify whether data exists in each channel.

Segment files are located in the `conf/seg` directory within your working directory.
The directory structure should look like this:

```plaintext
./
├── conf/
│   ├── map/
│   ├── seg/
│   │   ├── modulelist.yaml
│   │   ├── seglist.yaml
```

### `modulelist.yaml`

The `modulelist.yaml` file defines the modules used in the analysis.
Each module corresponds to a DAQ hardware device, such as ADCs or TDCs.

**Example: Configuration for a V1190A module**

```yaml
V1190A:
  id: 24
  ch: 128
  values:
    - tdcL: [300, -5000., 300000.] # Leading edge
    - tdcT: [300, -5000., 300000.] # Trailing edge
```

- V1190A: Module name, used in `seglist.yaml`.
- id: Module ID, defined in `TModuleDecoder` ([V1190 example](https://github.com/artemis-dev/artemis/blob/develop/sources/loop/decoder/TModuleDecoderV1190.h))
- ch: Total number of channels. For example, the V1190A module has 128 channels.
- values: Histogram parameters in the format `[bin_num, x_min, x_max]`. These are used when checking raw data (see [Raw data check](../online_analysis/raw_data_check.md)).

### `seglist.yaml`

The `seglist.yaml` file describes the settings of each module.
In the CRIB DAQ system ([babirl](https://ribf.riken.jp/RIBFDAQ/index.php?DAQ/Download)), modules are identified using four IDs: **dev**, **fp**, **mod**, and **geo**.
These IDs are assigned during DAQ setup and are written into the RIDF data file.

**ID Descriptions**

| ID      | Description                                                                                 | CRIB example                                                                              |
| ------- | ------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| **dev** | Device ID: Distinguishes major groups (e.g., experiments).                                  | CRIB modules use `12`, BIGRIPS and SHARAQ use others.                                     |
| **fp**  | Focal Plane ID: Differentiates sections of the DAQ system or crates.                        | Common DAQ is used for CRIB focal planes, so this ID is used to differentiate MPV crates. |
| **mod** | Module ID: Identifies the purpose of the module (e.g., PPAC, PLA).                          | `6` for ADCs, `7` for TDCs, `63` for scalers.                                             |
| **geo** | Geometry ID: Distinguishes individual modules with the same `[dev, fp, mod]` configuration. | Unique ID for each module.                                                                |

**Example: Configuration for a V1190A module in `seglist.yaml`**

```yaml
J1_V1190:
  segid: [12, 2, 7]
  type: V1190A
  modules:
    - id: 0
    - id: 1
```

- **J1_V1190**: A descriptive name for the module, used when creating histograms or trees.
- **segid**: Represents [dev, fp, mod] values.
- **type**: Specifies the module type, as defined in `modulelist.yaml`.
- **modules**: Lists geometry IDs (geo). Multiple IDs can be specified as an array.

## Map Files

Map files consist of a main configuration file (`mapper.conf`) and individual map files stored in the `conf/map` directory.

```plaintext
./
├── mapper.conf
├── conf/
│   ├── map/
│   │   ├── ppac/
│   │   │   ├── dlppac.map
│   │   ...
│   ├── seg/
```

### `mapper.conf`

This file specifies which map files to load and their configuration.
Each line in the file indicates the path to a map file relative to the working directory and the number of columns of the **segid** in the file.
Example configuration:

```plaintext
# file path for configuration, relative to the working directory
# (path to the map file) (Number of columns)

#====================================
# bld
# cid = 1: rf
conf/map/rf/rf.map 1

# cid = 2: coin
conf/map/coin/coin.map 1

# cid = 3: f1ppac
conf/map/ppac/f1ppac.map 5

# cid = 4: dlppac
conf/map/ppac/dlppac.map 5
```

**Format of the `mapper.conf`**

- **Path**: Specifies the relative path to the map file.
- **Number of Columns**: Indicates the number of segments defined in the map file.

### Individual Map Files

Each map file maps DAQ data segments to detector inputs.
The general format is:

```plaintext
cid, id, [dev, fp, mod, geo, ch], [dev, fp, mod, geo, ch], ...
```

**ID description**

| ID                        | Description                                                                                     |
| ------------------------- | ----------------------------------------------------------------------------------------------- |
| `catid` or `cid`          | Category ID: Groups data for specific analysis.                                                 |
| `detid` or `id`           | Detector ID: Differentiates datasets within a category. It corresponds to the row of map files. |
| `[dev, fp, mod, geo, ch]` | Represents segment IDs and channels (`segid`).                                                  |
| `typeid`                  | Index of the segment IDs. The first set of `segid` is correspond to 0.                          |

Example for `dlppac.map`:

```plaintext
# map for dlppac
#
#  Map: X1 X2 Y1 Y2 A
#
#--------------------------------------------------------------------
# F3PPACb
   4,   0,  12,   2,  7,   0,   4,  12,   2,  7,   0,   5,  12,   2,  7,   0,   6,  12,   2,  7,   0,   7,  12,   2,  7,   0,   9
# F3PPACa
   4,   1,  12,   0,  7,   16,   1,  12,   0,  7,   16,   2,  12,   0,  7,   16,   3,  12,   0,  7,   16,   4,  12,   0,  7,   16,   0
```

In this example:

- `catid = 4`: Indicates the **PPAC** category.
- `detid = 0, 1`: Identifies the specific data set within this category.
- Five `[dev, fp, mod, geo, ch]` combinations: Define the five input channels (X1, X2, Y1, Y2, A).
- `typeid = 0` corresponds X1, `typeid = 1` corresponds X2, and so on.

#### Relation to `mapper.conf`

The number of `[dev, fp, mod, geo, ch]` combinations in a map file determines the column count in `mapper.conf`.
For example:

```plaintext
conf/map/ppac/dlppac.map 5
```

### Verifying the Mapping (Optional)

To ensure correctness, use the Python script `pyscripts/map_checker.py`:

```shell
python pyscripts/map_checker.py
```

For `uv` environments:

```shell
uv run pyscripts/map_checker.py
```

## Summary

- **Segment files**: Define hardware modules and verify raw data inputs (refer to [Raw Data Check](../online_analysis/raw_data_check.md)).
- **Map files**: Map DAQ data to detectors and analysis inputs. Verify with `map_checker.py`.
