cd ../../$1/
git diff > $1.patch
cp $1.patch ../vs2015/patches/
rm $1.patch
