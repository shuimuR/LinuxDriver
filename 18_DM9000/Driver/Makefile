#KERN_DIR = /home/shuimu/more/linux-2.6.22.6 
KERN_DIR = /home/shuimu/more/Android_Linux_Source/linux-2.6.22.6 
all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += dm9dev9000c.o
