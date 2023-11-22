# USAGE: this <prefix>

prefix=$1

root=$PWD
rootLen=${#root}

function checkFolder() {
    local curDir=$PWD
    for item in *; do
        if [ -f "$item" ]; then
            local text="${curDir}/${item}"
            echo "${prefix}${text:${rootLen}}"
        fi
    done
    for item in *; do
        if [ -d "$item" ]; then
            cd "$item"
            checkFolder
            cd ..
        fi
    done
}

cd QRMatrix
res=$?
if [ $res -ne 0 ]; then
    exit $res
fi

checkFolder
