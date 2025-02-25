#!/bin/bash

if [ -z "${ARTEMIS_WORKDIR}" ]; then
  echo "ARTEMIS_WORKDIR is not set."
  exit 1
fi

if cd "${ARTEMIS_WORKDIR}"; then
  echo "Main process"
  ### only first time is okay
  ## assume solid angle histogram is already created
  ## acd; a; add steering/simSolidAngle.yaml

  ## run simulation using artemis
  #poetry run python ana2/doAllSimulation.py

  ## from ROOT file, make TGraph or TH2F objects
  #artemis -l -q ana2/mkObjects.C

  ## from TGraph or TH2F objects, save the figure
  #poetry run python ana2/chkObjects.py
  ### end

  ### make geometry figure
  ##poetry run python ana2/geometry.py

  ##########################################################
  ## make random generator
  #poetry run python ana2/mkGenerator.py
  #uv run ana2/mkGenerator.py

  ## make yield root files from okawa directory
  ## ./run_***

  ## from ROOT file, make yield text files
  #artemis -l -q ana2/mkyield_text.C
  artemis -l -q ana2/mksim_text.C
  ##########################################################

  ## make syserror root file
  #poetry run python ana2/reconstSimulation.py

  ## make syserror hist file
  #artemis -l -q ana2/mkSyserr.C

  ## generate cross section text file
  ## from mkYield.ipynb, and plot_crosssection.ipynb we can make it.
else
  echo "Failed to change directory to ${ARTEMIS_WORKDIR}"
  exit 1
fi

if ! cd - >&/dev/null; then
  echo "Failed to return to the previous directory"
  exit 1
fi
