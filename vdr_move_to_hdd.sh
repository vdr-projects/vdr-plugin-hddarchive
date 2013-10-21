#!/bin/bash

ARCHIVEHDD=/media/archive-hdd

HDDMOUNTED=0

VID_FULLPATH="`cd \"$VIDEO\" 2>/dev/null && pwd || echo \"$VIDEO\"`/"

SRC_FULLPATH="`cd \"$1\" 2>/dev/null && pwd || echo \"${1}\"`/"

VIDPATH="`cd \"$1\" && cd \"..\" 2>/dev/null && pwd || echo \"${1}\"`/"

PART1=${VIDPATH%*/}
PART1=${PART1##*/}

PART2=${SRC_FULLPATH%*/}
PART2=${PART2##*/}

MOVIEFOLDER=$PART1"/"$PART2"/"

# Test, if recording has already moved?
if [ -f ${SRC_FULLPATH}/hdd.vdr ]; then
    svdrpsend mesg "Recording has already been moved to Archive-HDD!"
    exit 1
fi


# Test, if Archive-HDD can be mounted
if [ ! -f ${ARCHIVEHDD}/hdd.vdr ]; then
    mount ${ARCHIVEHDD}
    HDDMOUNTED=1
fi
if [ ! -f ${ARCHIVEHDD}/hdd.vdr ]; then
    svdrpsend mesg "Archive-HDD could not be mounted!"
    exit 1
fi

# Test if there is enough disk space on Archive-HDD
SIZE_SRC="`du $SRC_FULLPATH | cut -f 1`"
SIZE_DEST="`df -Pk /media/archive-hdd | tail -n 1 | tr -s ' ' | cut -d' ' -f 4`"

if [ $SIZE_DEST -lt $SIZE_SRC ]; then
    svdrpsend mesg "Not enough space on Archive-HDD!"
    umount ${ARCHIVEHDD}
    exit 1
fi

MODMOVIEFOLDER=$(echo "$MOVIEFOLDER" | awk '
BEGIN{
	FS="/";
}
{
	MODMOVIEFOLDER="";
	for (i=1; i<=NF; i++) {
		if ((i==NF-2) && (index($i, "%")==1)) { # cutted movie
			MODMOVIEFOLDER=MODMOVIEFOLDER "" substr ($i, 2);
		} else {
			MODMOVIEFOLDER=MODMOVIEFOLDER "" $i;
		}
		if (i<NF) MODMOVIEFOLDER=MODMOVIEFOLDER "/";
	}
	printf "%s", MODMOVIEFOLDER;
}' ARCHIVEHDD="$ARCHIVEHDD")

mkdir -p ${ARCHIVEHDD}/${MODMOVIEFOLDER}
for i in ${SRC_FULLPATH}/0??.vdr; do
    if [ -e "${i}" ]; then
	B=$(basename $i)
	svdrpsend mesg "Moving $B..."
	mv ${i}  ${ARCHIVEHDD}/${MODMOVIEFOLDER}
    fi
done
for i in ${SRC_FULLPATH}/0????.ts; do
    if [ -e "${i}" ]; then
	B=$(basename $i)
	svdrpsend mesg "Moving $B..."
        mv ${i}  ${ARCHIVEHDD}/${MODMOVIEFOLDER}
    fi
done
cp ${SRC_FULLPATH}/index.vdr ${ARCHIVEHDD}/${MODMOVIEFOLDER}
cp ${SRC_FULLPATH}/index ${ARCHIVEHDD}/${MODMOVIEFOLDER}
cp ${SRC_FULLPATH}/info.vdr  ${ARCHIVEHDD}/${MODMOVIEFOLDER}
cp ${SRC_FULLPATH}/info  ${ARCHIVEHDD}/${MODMOVIEFOLDER}
rm ${SRC_FULLPATH}/resume.vdr
rm ${SRC_FULLPATH}/resume
rm ${SRC_FULLPATH}/marks.vdr
rm ${SRC_FULLPATH}/marks
cp ${ARCHIVEHDD}/hdd.vdr ${SRC_FULLPATH}/


if [ "${HDDMOUNTED}" == "1" ]; then
    umount ${ARCHIVEHDD}
fi

svdrpsend mesg "Successfully moved Recording to Archive-HDD."
