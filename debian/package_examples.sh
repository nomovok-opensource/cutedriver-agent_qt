#!/bin/sh
# Package examples into tar.gz
cd $1/usr/share/doc/qttas-dev/examples
ls 
for i in *; do tar cvzf $i.tar.gz $i;done
ls -d */ | xargs rm -r
