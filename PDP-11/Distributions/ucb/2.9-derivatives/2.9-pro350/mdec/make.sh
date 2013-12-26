as produm.s
strip a.out
dd if=a.out of=produm bs=16 skip=1
as microdum.s
strip a.out
dd if=a.out of=microdum bs=16 skip=1
as r5uboot.s
strip a.out
dd if=a.out of=t0 bs=16 skip=1
cat produm t0>t1
dd if=t1 of=r5uboot conv=sync
as rduboot.s
strip a.out
dd if=a.out of=t0 bs=16 skip=1
cat produm t0>t1
dd if=t1 of=rduboot conv=sync
ln rduboot x
cat x x x x x x x x x x x x x x x x x>rdboot
rm x
as rauboot.s
strip a.out
dd if=a.out of=t0 bs=16 skip=1
cat microdum t0>t1
dd if=t1 of=rauboot conv=sync
rm produm microdum a.out t0 t1
