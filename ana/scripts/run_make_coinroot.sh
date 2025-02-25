#!/bin/bash

MAX_CONCURRENT_JOBS=8

running_jobs_count() {
  pgrep -c "artemis"
}

for FILE in "$ART_DATA_DIR"/high*.ridf; do
  while (($(running_jobs_count) >= MAX_CONCURRENT_JOBS)); do
    sleep 1
  done

  DATAFILE=${FILE##*/}
  FILENAME=${DATAFILE%.*}

  if [ "${FILENAME}" = "high0208" ]; then
    continue
  fi
  if [ "${FILENAME}" = "high0214" ]; then
    continue
  fi
  if [ "${FILENAME}" = "high0215" ]; then
    continue
  fi
  if [ "${FILENAME}" = "high0216" ]; then
    continue
  fi
  if [ "${FILENAME}" = "high0217" ]; then
    continue
  fi
  if [ "${FILENAME}" = "high0218" ]; then
    continue
  fi

  python "$ARTEMIS_WORKDIR"/scripts/make_coinroot.py "${FILENAME}" &
done
wait

for FILE in "$ART_DATA_DIR"/bg*.ridf; do
  while (($(running_jobs_count) >= MAX_CONCURRENT_JOBS)); do
    sleep 1
  done

  DATAFILE=${FILE##*/}
  FILENAME=${DATAFILE%.*}
  python "$ARTEMIS_WORKDIR"/scripts/make_coinroot.py "${FILENAME}" &
done
wait

python "$ARTEMIS_WORKDIR"/scripts/merge_coinroot.py
