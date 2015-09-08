cp $1.patch ../../$1/
cd ../../$1/
git apply -v $1.patch
rm $1.patch
