# meta-selinuxについて
## 1. 知りたいこと

- object-classの説明
- access-vectorの説明
- core-image-minimal等にselinuxを適用するには？

## 2. core-image-selinux-minimal/core-image-selinuxのビルド

- meta-selinuxをgit cloneしておく

- bblaysers.confの変更

```html:sample
	BBLAYERS ?= " \
	  /home/miyagawa/poky/meta \
	  /home/miyagawa/poky/meta-poky \
	  /home/miyagawa/poky/meta-yocto-bsp \
	  /home/miyagawa/poky/meta-openembedded/meta-oe \
	  /home/miyagawa/poky/meta-openembedded/meta-networking \
	  /home/miyagawa/poky/meta-openembedded/meta-python \
	  /home/miyagawa/poky/meta-virtualization \
	  /home/miyagawa/poky/meta-selinux \
	  "
```

- local.confの変更

PREFERRED_PROVIDER_virtual/refpolicy ?= "refpolicy-targeted"
PREFERRED_VERSION_refpolicy-targeted = "git"
DISTRO_FEATURES_append = " acl xattr pam selinux"

- core-image-selinux-minimalでビルドした場合

The 'core-image-selinux-minimal' does not automatically relabel the system.
So you must boot using the parameters "selinux=1 enforcing=0", and then
manually perform the setup.  Running 'fixfiles -f -F relabel' is available
in this configuration.

`runqemu qemuarm64 bootparams="selinux=1 enforcing=0"` 


