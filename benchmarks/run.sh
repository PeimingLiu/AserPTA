for f in *.bc
do
    echo "running on $f"
	/home/peiming/Documents/paper-ae/AserPTA/cmake-build-release/bin/wpa $f -stats
    echo "\n\n\n"
done
