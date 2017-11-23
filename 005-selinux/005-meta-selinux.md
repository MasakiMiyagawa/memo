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

### 3-1. core-image-selinuxのビルド

### 3-2. qemu起動

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

### 3-3. audit2allowでどうポリシーを書けばとりあえず通るようになるか調べる

Permissiveモードで起動してrootでログインするととりあえずこれだけ監査ログがでる

```
root@qemuarm64:~# grep audit /var/log/kern.log 
Nov 23 03:27:51 qemuarm64 kernel: [    1.948259] audit: initializing netlink subsys (disabled)
Nov 23 03:27:51 qemuarm64 kernel: [    1.954168] audit: type=2000 audit(1.884:1): state=initialized audit_enabled=0 res=1
Nov 23 03:27:51 qemuarm64 kernel: [    3.586280] audit: type=1403 audit(1511407644.674:2): policy loaded auid=4294967295 ses=4294967295
Nov 23 03:27:51 qemuarm64 kernel: [   25.594336] audit: type=1400 audit(1511407666.466:3): avc:  denied  { dac_read_search } for  pid=300 comm="sm-notify" capability=2  scontext=system_u:system_r:rpcd_t:s0 tcontext=system_u:system_r:rpcd_t:s0 tclass=capability permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   26.802952] audit: type=1400 audit(1511407667.677:4): avc:  denied  { write } for  pid=313 comm="distccd" name="distcc.pid" dev="tmpfs" ino=8044 scontext=system_u:system_r:distccd_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   26.805135] audit: type=1400 audit(1511407667.679:5): avc:  denied  { open } for  pid=313 comm="distccd" path="/run/distcc.pid" dev="tmpfs" ino=8044 scontext=system_u:system_r:distccd_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   26.811319] audit: type=1400 audit(1511407667.686:6): avc:  denied  { getattr } for  pid=313 comm="distccd" path="/run/distcc.pid" dev="tmpfs" ino=8044 scontext=system_u:system_r:distccd_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   27.201058] audit: type=1400 audit(1511407668.279:7): avc:  denied  { search } for  pid=321 comm="modprobe" name="events" dev="tracefs" ino=93 scontext=system_u:system_r:kmod_t:s0 tcontext=system_u:object_r:tracefs_t:s0 tclass=dir permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   30.635010] audit: type=1400 audit(1511407671.711:8): avc:  denied  { read } for  pid=364 comm="avahi-daemon" name="resolv.conf" dev="tmpfs" ino=7795 scontext=system_u:system_r:avahi_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   30.642064] audit: type=1400 audit(1511407671.716:9): avc:  denied  { open } for  pid=364 comm="avahi-daemon" path="/run/resolv.conf" dev="tmpfs" ino=7795 scontext=system_u:system_r:avahi_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   30.643457] audit: type=1400 audit(1511407671.719:10): avc:  denied  { getattr } for  pid=363 comm="avahi-daemon" path="/run/resolv.conf" dev="tmpfs" ino=7795 scontext=system_u:system_r:avahi_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:54 qemuarm64 kernel: [   33.368323] audit: type=1400 audit(1511407674.446:11): avc:  denied  { execute_no_trans } for  pid=410 comm="start_getty" path="/sbin/agetty" dev="vda" ino=10031 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:getty_exec_t:s0 tclass=file permissive=1
Nov 23 03:27:58 qemuarm64 kernel: [   37.560823] audit: type=1400 audit(1511407678.636:12): avc:  denied  { execute } for  pid=511 comm="login" name="bash.bash" dev="vda" ino=7708 scontext=system_u:system_r:local_login_t:s0-s0:c0.c1023 tcontext=system_u:object_r:bin_t:s0 tclass=file permissive=1
```

audit2allowでとりあえずのセキュリティコンテキストをどう設定すれば
このログが消えるか知ることができる。
ただし、これはdomain側の許可範囲(TE)を広げる設定のみ。
オブジェクトのFCを変更するのが正しい気もする。

```
root@qemuarm64:~# audit2allow -i /var/log/kern.log 

#============= avahi_t ==============
allow avahi_t initrc_var_run_t:file { getattr open read };

#============= distccd_t ==============
allow distccd_t initrc_var_run_t:file { getattr open write };

#============= getty_t ==============
allow getty_t getty_exec_t:file execute_no_trans;

#============= kmod_t ==============
allow kmod_t tracefs_t:dir search;

#============= local_login_t ==============

#!!!! This avc can be allowed using the boolean 'allow_polyinstantiation'
allow local_login_t bin_t:file execute;

#============= rpcd_t ==============
allow rpcd_t self:capability dac_read_search;
```

### 3-4. audit2allowでppファイルとteファイルを作る

audit2allowでppファイルとteファイルを作ることができる。

```
root@qemuarm64:~# cat /var/log/kern.log | audit2allow -M test
To make this policy package active, execute:  
semodule -i test.pp  
```

TEファイルは以下のような感じ。

```
root@qemuarm64:~# cat test.te 

module test 1.0;

require {
	type kmod_t;
	type distccd_t;
	type tracefs_t;
	type local_login_t;
	type avahi_t;
	type getty_t;
	type initrc_var_run_t;
	type bin_t;
	type rpcd_t;
	type getty_exec_t;
	class capability dac_read_search;
	class file { execute execute_no_trans getattr open read write };
	class dir search;
}

#============= avahi_t ==============
allow avahi_t initrc_var_run_t:file { getattr open read };

#============= distccd_t ==============
allow distccd_t initrc_var_run_t:file { getattr open write };

#============= getty_t ==============
allow getty_t getty_exec_t:file execute_no_trans;

#============= kmod_t ==============
allow kmod_t tracefs_t:dir search;

#============= local_login_t ==============

#!!!! This avc can be allowed using the boolean 'allow_polyinstantiation'
allow local_login_t bin_t:file execute;

#============= rpcd_t ==============
allow rpcd_t self:capability dac_read_search;
```

### 3-5. ppファイルをロード(恒久的)

ポリシーをロードする

`semodule -i test.pp`

### 3-6. reboot

今度はenforceでもログインできる

`runqemu qemuarm64 bootparams="selinux=1 enforcing=1" nographic`
