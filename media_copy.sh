# grep packages Assets/app.log | grep -i success | awk '{print $1}' | sort | uniq | sed s/\\\\/\\//g > tocopy
rm -rf media_loaded
mkdir -p media_loaded
for i in `cat tocopy`; do 
	dst=`echo $i | sed s/packages/media_loaded/g`
	echo $dst 
	mkdir -p `dirname $dst`
	cp $i $dst
done


