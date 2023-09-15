#!/bin/bash

code_ptrs=("ret"  "funcptrstackvar"  "funcptrstackparam"  "funcptrheap"            \
"funcptrbss"  "funcptrdata"  "structfuncptrstack"  "structfuncptrheap"              \
"structfuncptrdata"  "structfuncptrbss"  "longjmpstackvar"  "longjmpstackparam"     \
"longjmpheap"  "longjmpdata"  "longjmpbss" "bof"  "iof"  "leak" )                   

# funcs=("memcpy"  "strcpy"  "strncpy"  "sprintf"  "snprintf"  "strcat"  \
# "strncat"  "sscanf"  "homebrew")
funcs=("memcpy")

locations=("stack" "heap" "bss" "data")

attacks=("shellcode"  "returnintolibc"  "rop"  "dataonly")
techniques=("direct" "indirect")
count_only=0
output=''

# echo ${code_ptr[4]}

print_attack(){
    local attack=$1
    local tech=$2
    local loc=$3
    local ptr=$4
    local func=$5
    local status=$6
    
    result=''
    if [[ $status -eq 1 ]]; then
        result="OK"
    else
        result="FAIL"
    fi

    if [[ $status -eq 2 ]]; then
        result="Impossible"
    fi

    echo "Technique: $tech"
    echo "Attack code: $attack"
    printf "%-50s%s\n" "Target Pointer: $ptr" "Result: $result"
    echo "Location: $loc"
    echo "Function: $func"
    echo "$bold_line"
    echo ""
}

# Default values
techniques=("direct" "indirect")
output_file=""

# Parse command line arguments
while getopts "t:fro:" option; do
    case $option in
        t)
            techniques=("$OPTARG")
            ;;
        f)
            unset funcs
            ;;
        o)
            output_file="${OPTARG[0]}"
            ;;
        *)
            echo "Usage: script.sh [-t techniques] [-f] [-r] [-o output_file]"
            exit 1
            ;;
    esac
done


# Print header
width=64
bold_line=$(printf "%${width}s" | tr ' ' '=')
echo "$bold_line"
echo "RIPE: The Runtime Intrusion Prevention Evaluator for RISCV"
echo "$bold_line"

mkdir out
touch out/out.text

total_ok=0
total_fail=0
total_np=0
defend_ok=0

start_time=$(date +%s.%N)

# Loop through the attacks, techniques, locations, code_ptrs, and funcs arrays
for attack in "${attacks[@]}"; do
    for tech in "${techniques[@]}"; do
        for loc in "${locations[@]}"; do
            for ptr in "${code_ptrs[@]}"; do
                for func in "${funcs[@]}"; do
                    rm -f out/out.text

                    cmdargs="./ripe_attack_generator -t $tech -i $attack -c $ptr -l $loc -f $func -dasics"
                    cmdline="$cmdargs > out/out.text 2>&1"

                
					if [[ $count_only -eq 0 ]]; then
						eval "$cmdline"
                        return_code=$?
                        if [ $return_code -eq 66 ]; then
                            defend_ok=$((defend_ok + 1))
                        fi
						sleep 0.1
					else
						touch out/out.text
					fi

					# Evaluate attack status
					status=0
					if grep -q "Impossible" out/out.text; then
						status=2
						total_np=$((total_np + 1))
					fi

					if grep -q "success" out/out.text; then
						status=1
						total_ok=$((total_ok + 1))
                        echo $cmdargs >> success.txt
					fi

					if [[ $status -eq 0 ]]; then
						total_fail=$((total_fail + 1))
					fi


					# print attack
					print_attack $attack $tech $loc $ptr $func $status
					echo "$cmdargs $status"
                done
            done
        done
    done
done

# Calculate summary values
total_attacks=$((total_ok + total_fail + total_np))
end_time=$(echo "$(date +%s.%N) - $start_time" | bc)
avg_time=$(echo "scale=2; $end_time / $total_attacks" | bc)

# Print summary
echo "SUMMARY"
echo "Total OK: $total_ok"
echo "Total protect: $defend_ok"
echo "Total FAIL: $total_fail"
echo "Total Not-Posiible: $total_np"
echo "Total Attacks Executed: $total_attacks"
# echo "Total time elapsed: $(printf "%dm %ds" $((end_time / 60)) $((end_time % 60)))"
# echo "Average time per attack: $(printf "%.2f" $avg_time)s"