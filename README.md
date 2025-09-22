#devctl detach vtblk1
#devctl disable vtblk1		# ( diskinfo -v |grep Parent )
#devctl enable vtblk1

devctl disable virtio_pci3
devctl enable virtio_pci3
