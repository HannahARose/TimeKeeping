sh filequeue.sh | \
    ./build/bin/Phaser stdin stdout | \
    ./build/bin/Timer -e --si_freq=995532.6897579981 --si_error_freq=250.0046e6 stdin ./data/out5.csv