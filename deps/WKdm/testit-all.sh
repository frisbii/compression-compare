#! /usr/bin/env bash

#SBATCH --ntasks=4
#SBATCH --cpus-per-task=4
#SBATCH --nodes=8
#SBATCH --tasks-per-node=16

if [[ ${#@} < 2 ]]; then
    printf "USAGE: ${0} <where to run (local|hpc)> <trace name list...>\n"
    exit 1
fi

# Extract the command line arguments.
whereToRun=${1}
shift
traceNameList=($@)

if [[ ${whereToRun} != "local" ]] && [[ ${whereToRun} != "hpc" ]]; then
   printf "Invalid choice of where to run\n"
   exit 1
fi

# Parameters and constants.
iterations=5

# Basic values used throughout.
taskName=testit
script=${taskName}-one.sh
baseDirectory=${PWD}/..
baseResultsDirectory=${baseDirectory}/results

# Create the results directory.
mkdir --parents --mode=700 ${baseResultsDirectory}
if [ $? != 0 ]; then
    printf "ERROR: mkdir ${baseResultsDirectory} failed\n"
    exit
fi

# Loop through each trace.
for traceName in ${traceNameList[@]}; do

    printf "${traceName}:\n"

    resultsDirectory=${baseResultsDirectory}/${traceName}
    mkdir --parents --mode=700 ${resultsDirectory}
    if [ $? != 0 ]; then
	printf "ERROR: Failed to make results directory ${resultsDirectory}\n"
	exit 1
    fi
        
    resultsPrefix=${resultsDirectory}/${algorithm}

    # Run it.
    if [[ ${whereToRun} == "local" ]]; then
	${script} ${traceName} ${iterations} ${dictionaryType} ${dictionarySize} \
		  > ${resultsPrefix}.out \
		  2> ${resultsPrefix}.err
    else
	srun -l \
	     -o ${resultsPrefix}.out \
	     -e ${resultsPrefix}.err \
	     ${script} ${traceName} ${iterations} ${dictionaryType} ${dictionarySize}
    fi

    printf "done.\n"

done
