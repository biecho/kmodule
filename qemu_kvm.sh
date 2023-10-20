# Make sure to compile the kernel v5.10 to get the bzImage
# And create a bullseye with ./create-image.sh --arch x86_64 --distribution bullseye --feature full
qemu-system-x86_64 \
        -m 2G \
        -smp 2 \
        -kernel $KERNEL/arch/x86/boot/bzImage \
        -append "console=ttyS0 root=/dev/sda earlyprintk=serial net.ifnames=0" \
        -drive file=$IMAGE/bullseye.img,format=raw \
        -net user,host=10.0.2.10,hostfwd=tcp:127.0.0.1:10021-:22 \
        -net nic,model=e1000 \
        -enable-kvm \
        -nographic \
        -pidfile vm.pid \
        -virtfs local,path=$HOME,mount_tag=host0,security_model=none,id=host0 \
        2>&1 | tee vm.log
