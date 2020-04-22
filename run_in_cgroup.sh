#!/bin/bash

print_help(){
cat << 'DOC'
usage:
    run_in_cgroup --cggroup=CGROUP <COMMAND>
    run_in_cgroup --help
note:    
    CGROUP must start will slash '/' , and no special character
example:
    run_in_cgroup --cggroup=/mycgroup.slice ping 127.0.0.1
DOC
}

## parse parameter
for i in "$@"
do
case $i in
    --cgroup=*)
        cgroup=${i#*=}
        shift
        ;;
    --help)
        print_help
        exit 0
        shift
        ;;
    -*)
        shift
        ;;
    *)
        break
        ;;
esac
done

[[ -z "$cgroup" ]] && print_help && exit 1
[[ -z "$@" ]] && print_help && exit 1

# test suid bit
if [ -u "$(which cgattach)" ]; then 
    cgattach $$ $cgroup && attached=1
else
    sudo cgattach $$ $cgroup && attached=1
fi

# test attach success or not
[[ -z "$attached" ]] && print_help && exit 1

exec "$@"
