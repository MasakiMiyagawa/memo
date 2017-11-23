# meta-selinuxについて
## 1. TODO

- object-classの説明
- access-vectorの説明
- core-image-minimal等にselinuxを適用するには？
- SDKの作り方
- 起動しないとラベリングされないってありなの？

## 2. core-image-selinux-minimal/core-image-selinuxのビルド

- meta-selinuxをgit cloneしておく

- bblaysers.confの変更

```
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

## 3. audit2allowを使ってとりあえずすべてのアクセスが通るようにしてみる

1. core-image-selinuxのビルド

2. qemu起動

1回目の起動はrelabelが行われるので必ずpermissiveで起動する模様。

`runqemu qemuarm64 bootparams="selinux=1 enforcing=1" nographic`

2回目の起動(単なる確認本来不要)

loginのドメインがexecできないので、
これをやるとloginできなくなる。qemuの場合プロセスをkillして一旦止める。

`runqemu qemuarm64 bootparams="selinux=1 enforcing=1" nographic`

3回目の起動
permissiveで起動する。audit2allowを使ってとりあえず出まくっているdeniedを
なんとかする。

`runqemu qemuarm64 bootparams="selinux=1 enforcing=0" nographic`

3. audit2allowでどうポリシーを書けばとりあえず通るようになるか調べる

4. audit2allowでppファイルとteファイルを作る

5. ppファイルをロード(恒久的)

6. reboot
