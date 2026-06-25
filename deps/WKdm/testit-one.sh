#!/usr/bin/env bash

if [[ ${#@} != 2 ]]; then

    printf "USAGE: ${0} <trace name>\n"
    printf "            <timing iterations>\n"
    exit

fi

traceName=${1}
iterations=${2}

algorithm=WKdm
bytesPerBuffer=4096

baseDirectory=${PWD}/..
traceDirectory=${baseDirectory}/traces
#traceDirectory=/home/sfkaplan/Archive/traces/page-replacement-project
tracePath=${traceDirectory}/${traceName}/${traceName}.images.xz
tester=${baseDirectory}/${algorithm}/testit
resultsDirectory=${baseDirectory}/results/${traceName}
timingsPath=${resultsDirectory}/${algorithm}-timings.xz
mkdir -p ${resultsDirectory}

unxz -c ${tracePath} \
    | ${tester} ${bytesPerBuffer} ${iterations} - \
    | xz \
    > ${timingsPath}
results=( ${PIPESTATUS[@]} )
if [[ ${results[0]} != 0 ]]; then
    printf "ERROR: Decompression of ${tracePath} failed\n"
    exit 1
elif [[ ${results[1]} != 0 ]]; then
    printf "ERROR: Tester failed\n"
    exit 1
elif [[ ${results[2]} != 0 ]]; then
    printf "ERROR: Compression of ${timingsPath} failed\n"
    exit 1
else
    printf "DONE\n"
fi
