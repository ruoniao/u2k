qemu-system-x86_64 -kernel ./linux-5.10.61/arch/x86_64/boot/bzImage -drive file=./img/bullseye.img,format=raw,if=virtio -m 4G -nographic -append "root=/dev/vda console=ttyS0"
