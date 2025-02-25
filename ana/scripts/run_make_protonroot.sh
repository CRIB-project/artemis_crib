#!/bin/bash

python "$ARTEMIS_WORKDIR"/scripts/make_protonroot.py

python "$ARTEMIS_WORKDIR"/scripts/merge_protonroot.py
