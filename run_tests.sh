#!/bin/bash

executable_path="./install/bin/"
base_names=("01-main" "02-switch" "03-equity" "11-join" "12-join-main"
    "21-create-many" "22-create-many-recursive" "23-create-many-once"
    "31-switch-many" "32-switch-many-join" "33-switch-many-cascade"
    "51-fibonacci" "61-mutex" "62-mutex" "63-mutex-equity" "64-mutex-join" "71-preemption" "81-deadlock" "91-priority")

# Definitions for base test names and number of parameters
declare -A num_params
declare -A param_descriptions

# Configure the number of parameters and their descriptions
num_params[21-create-many]=1
defaut_params[21-create-many]="10000"
defaut_graph_params[21-create-many]="lin 1 40 1"
param_descriptions[21-create-many]="number of threads"

num_params[22-create-many-recursive]=1
defaut_params[22-create-many-recursive]="10000"
defaut_graph_params[22-create-many-recursive]="lin 1 40 1"
param_descriptions[22-create-many-recursive]="number of threads"

num_params[23-create-many-once]=1
defaut_params[23-create-many-once]="10000"
defaut_graph_params[23-create-many-once]="lin 1 40 1"
param_descriptions[23-create-many-once]="number of threads"

num_params[31-switch-many]=2
defaut_params[31-switch-many]="10 10000"
defaut_graph_params[31-switch-many]="lin 1 40 1 lin 1 40 1"
param_descriptions[31-switch-many]="number of threads;number of switches"

num_params[32-switch-many-join]=2
defaut_params[32-switch-many-join]="10 10000"
defaut_graph_params[32-switch-many-join]="lin 1 40 1 lin 1 40 1"
param_descriptions[32-switch-many-join]="number of threads;number of switches"

num_params[33-switch-many-cascade]=2
defaut_params[33-switch-many-cascade]="10 10000"
defaut_graph_params[33-switch-many-cascade]="lin 1 40 1 lin 1 40 1"
param_descriptions[33-switch-many-cascade]="number of threads;number of switches"

num_params[51-fibonacci]=1
defaut_params[51-fibonacci]="23"
defaut_graph_params[51-fibonacci]="lin 1 15 1"
param_descriptions[51-fibonacci]="Fibonacci number to calculate"

num_params[61-mutex]=1
defaut_params[61-mutex]="20"
defaut_graph_params[61-mutex]="lin 1 20 1"
param_descriptions[61-mutex]="number of threads"

num_params[62-mutex]=1
defaut_params[62-mutex]="20"
defaut_graph_params[62-mutex]="lin 1 20 1"
param_descriptions[62-mutex]="number of threads"

num_params[63-mutex-equity]=1
defaut_params[63-mutex-equity]="10"
defaut_graph_params[63-mutex-equity]="lin 1 10 1"
param_descriptions[63-mutex-equity]="number of threads"

num_params[64-mutex-join]=1
defaut_params[64-mutex-join]="10"
defaut_graph_params[64-mutex-join]="lin 1 10 1"
param_descriptions[64-mutex-join]="number of threads"

num_params[71-preemption]=1
defaut_params[71-preemption]="10"
defaut_graph_params[71-preemption]="lin 1 40 1"
param_descriptions[71-preemption]="number of threads"

mode="normal"

# Parse command line options
while getopts "vg" opt; do
    case "$opt" in
    v) mode="valgrind" ;;
    g) mode="graphs" ;;
    *)
        echo "Usage: $0 [-v (valgrind) | -g (graphs)]"
        exit 1
        ;;
    esac
done

get_params_output=""

# Function to get regular parameters
get_params() {
    local test_name=$1
    IFS=';' read -r -a descriptions <<< "${param_descriptions[$test_name]}"
    local params=()

    for desc in "${descriptions[@]}"; do
        echo "Enter $desc for $test_name:"
        read param
        params+=("$param")
    done

    get_params_output="${params[*]}"
    # Echo the parameters separated by spaces to capture them as a single string
}

get_graph_params_output=""
# Function to get graph parameters with min, max, and step
get_graph_params() {
    local test_name=$1
    IFS=';' read -r -a descriptions <<< "${param_descriptions[$test_name]}"
    local graph_params=()

    for desc in "${descriptions[@]}"; do
        echo "Enter scale type for $desc of $test_name (lin)ear, or (log)arithmic"
        read scale_type
        echo "Enter min value for $desc of $test_name:"
        read min_val
        echo "Enter max value for $desc of $test_name:"
        read max_val
        echo "Enter step for $desc of $test_name:"
        read step
        graph_params+=("$scale_type $min_val $max_val $step")
    done
    # Return the graph parameters as a single string with each set of values quoted
    local i=0
    local output=""
    for param in "${graph_params[@]}"; do
        output+="$param"
        ((i++))
        [ $i -ne ${#graph_params[@]} ] && output+=" "
    done
    get_graph_params_output="$output"
}

run_test() {
    local base_name=$1 local parameters=$2
    echo "Running test $executable_path$base_name with mode $mode..."
    case $mode in
    normal)
        $executable_path$base_name $parameters
        echo "-----------------------"
        echo "Running test $executable_path${base_name}-pthread with mode $mode..."
        $executable_path${base_name}-pthread $parameters
        echo "-----------------------"
        ;;
    valgrind)
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes "$executable_path$base_name" $parameters
        echo "-----------------------"
        ;;
    graphs)
        python3 plot_graph.py "$executable_path$base_name" $parameters
        echo "-----------------------"
        ;;
    esac
}

for base_name in "${base_names[@]}"; do
    if [ "$mode" == "graphs" ] && ! [[ -n ${num_params[$base_name]} ]]; then
        continue
    fi

    echo $base_name $mode ${num_params[$base_name]}
    base_default_params=""
    if [[ -n ${num_params[$base_name]} ]]; then
        if [ "$mode" == "graphs" ]; then
            base_default_params="${defaut_graph_params[$base_name]}"
            params="$base_default_params"
        else
            base_default_params="${defaut_params[$base_name]}"
            params="$base_default_params"
        fi
    fi

    next="false"
    while true; do
        echo "Do you want to (e)xecute this test, skip to (n)ext test or (q)uit?"
        read -r choice
        case "$choice" in
        e) 
            break
            ;;
        n)
            next="true"
            break
            ;;
        q) 
            exit
            ;;
        *)
            echo "Invalid option. Try again."
            continue
            ;;
        esac
    done

    if [[ "$next" = "true" ]]; then
        continue
    fi

    chparam="true"
    rerun="true"
    while true; do
        if [[ -n ${num_params[$base_name]} ]] && [[ "$chparam" = "true" ]]; then
            while true; do
                echo "Here's the actual parameters : $params"
                echo "Do you want to (e)xecute, (c)hange parameters, or (q)uit?"
                read -r choice
                case "$choice" in
                e) break ;;
                c)
                    if [ "$mode" == "graphs" ]; then
                        get_graph_params "$base_name"
                        params=$get_graph_params_output
                    else
                        get_params "$base_name"
                        params=$get_params_output
                    fi
                    ;;
                q)
                    echo "Exiting test suite."
                    exit
                    ;;
                *)
                    echo "Invalid option. Try again."
                    continue
                    ;;
                esac
            done
        fi

        if [[ "$rerun" = "true" ]]; then
            run_test "$base_name" "$params"
        fi
        rerun="false"
        chparam="false"

        echo "Do you want to (r)erun, (c)hange parameters, (n)ext test or (q)uit?"
        read -r choice
        case "$choice" in
        r) 
            rerun="true"
            continue 
            ;;
        c)
            chparam="true"
            rerun="true"
            ;;
        n) break ;;
        q)
            echo "Exiting test suite."
            exit
            ;;
        *)
            echo "Invalid option. Try again."
            continue
            ;;
        esac
    done
done

echo "All tests completed."