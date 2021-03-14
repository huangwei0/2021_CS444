#!/bin/bash

# this is the default directory from which files are copied
SRC="xv6-cps"
DEST=""

# this is where i put all my xv6 files
JDIR=~chaneyr/Classes/cs444/xv6
#JDIR=~chaneyr/class_labs/cs444

# our emulator
QEMU=qemu-system-i386

function show_help()
{
    BFILE=$(basename ${0})
    echo "${BFILE} -d dest_dir [-s src_dir] [-h]"
    echo "  -h             Show some, slightly, helpful messages and exit."
    echo ""
    echo "  -d dest_dir -> The name of the directroy into which the"
    echo "                 xv6 source tree will be cloned."
    echo "                 If the dest_dir already exists, you will be"
    echo "                 prompted to delete it."
    echo "                 There is no default value, you must supply a value."
    echo ""
    echo "  -s srs_dir  -> The name of the existing xv6 source tree from"
    echo "                 you are cloning."
    echo "                 The default cloned directory is ${SRC}."
    exit
}

while getopts "hs:d:" OPT
do
    case "${OPT}" in
        h)
            show_help
            ;;
        s)
            SRC=${OPTARG}
            ;;
        d)
            DEST=${OPTARG}
            ;;
        *)
            echo "Unknown command line option ${OPTARG}"
            ;;
    esac
done

if [ -z "${DEST}" ]
then
    echo "You must provide a destination directoy name with the -t <name> option"
    exit
fi

JSRC=${JDIR}/${SRC}
if [ ! -d ${JSRC} ]
then
    echo "Source directory ${SRC} does not exist"
    exit
fi

if [ -d ${DEST} ]
then
    echo "Destination directory exists."
    read -p "  Remove it? (Y/N): " CONFIRM 

    if [[ ${CONFIRM} != [yY] ]]
    then
        echo "  Exiting without cloning"
        exit
    else
        rm -rf ${DEST}
    fi
fi

mkdir ${DEST}
UDEST=${PWD}/${DEST}

cd $JSRC

for FILE in * .gdbinit.tmpl .gitignore
do
    #BFILE=$(basename ${FILE})
    if [ -L ${FILE} -a "${FILE}" == "${QEMU}" ]
    then
        ln -s ${JDIR}/bin/${QEMU} ${UDEST}/${QEMU}
        #echo "ln -s ${JDIR}/${QEMU} ${UDEST}/${QEMU}"
    else
        if [ -r ${FILE} -a -f ${FILE} ]
        then
            cp ${FILE} ${UDEST}/${FILE}
        else
            echo "Cannot copy file ${FILE}"
        fi
        #echo "cp ${FILE} ${UDEST}/${FILE}"
    fi
done

echo "You are ready for xv6 hacking..."
