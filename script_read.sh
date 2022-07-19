bench_dir="../ReverseSynthesis/release"
top="top_primitive.v"
read_sh="script.read"

for dir in $(ls -d $bench_dir/*); do
    echo "[INFO] Testing with ${dir} ..."
    echo -e "read ${dir}/${top} \nstrash \n&get \nlsv_print_nodes" > ${read_sh}
    ./abc -F "${read_sh}" 
    echo -e "[END]\n"
done