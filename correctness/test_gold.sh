trace_name=ollama
trace=./images/REFERENCE_ollama_qwen25_coder_32b.page-images.xz

binaries=(lz4 WKdm zlib WK64 zstd lzo)
invalidations_c=(none clflush)
invalidations_dc=(none clflush)

for bin in "${binaries[@]}"; do
for invc in "${invalidations_c[@]}"; do
for invdc in "${invalidations_dc[@]}"; do
    echo -n "Starting ${bin} ${invc} ${invdc}..."
    refdata="./correctness/GOLD_${trace_name}_${bin}.csv"
    cat "./correctness/REFERENCE_ollama_qwen25_coder_32b.page-images" | "./bin/${bin}" "REFERENCE_ollama" $invc $invdc "csv" | awk -F',' -v cols="page_number,compressed_size,uncompressed_size" '
BEGIN {
    # make array of target column names
    split(cols, target_names, ",")
}
NR==1 {
    # first file header: get the indices of the target columns
    for (i=1; i<=NF; i++) header_indices1[$i] = i
    for (j in target_names) target_indices1[j] = header_indices1[target_names[j]]    
    next
}
NR==FNR {
    # first file rows: save values in the target columns, keyed by row# and col name
    for (j in target_names) values1[FNR, j] = $target_indices1[j]
    next
}
FNR==1 {
    # second file header: get the indices of the target columns
    for (i=1; i<=NF; i++) header_indices2[$i] = i
    for (j in target_names) target_indices2[j] = header_indices2[target_names[j]]    
    next
}
{
    # second file rows: compare to the saved values; emit differences if any
    for (j in target_names) {
        col_name = target_names[j]
        if (values1[FNR, j] != $target_indices2[j]) {
            printf("[%s: %s vs %s]\n", col_name, values1[FNR, j], $target_indices2[j])
        }
    }
}
END {
}
' - "./correctness/GOLD_ollama_${bin}.csv"
    echo "done."
done
done
done