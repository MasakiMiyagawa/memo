# SELinuxに関するメモ書き
## AV (Access Vectorルールについて)
### 参考
- http://hetarena.com/archives/1535
### neverallow ルール
このルールは他の３つとは別格で、ルールのコンパイル時に許さないルールを規定する。
allow/auditallow ルールで許可されていても neverallow に引っかかったらコンパイル時にエラーとなる。
### 現在のAVを確認する
現在設定されている AV ルールを表示するには sesearch を使用

```
root@qemux86:~# sesearch --allow 
```

### neverallowはbase ポリシーじゃないと動作しない？
https://selinuxproject.org/page/AVCRules#auditallow
にしめされている
portconについてもmoduleポリシーでは動作しないことが示されている。※portconはcorenetworkでのみ定義されていてcorenetworkはmodules.conf上BasePolicyである。
https://selinuxproject.org/page/NetworkStatements#portcon
system/authlogin.teにはneverallowステートメントが使用されている。authlogin.teはBase Policyでなければならない。
一方でmodules.confは以下のようになっている。

```
# Layer: system
# Module: authlogin
#
# Common policy for authentication and user login.
# 
authlogin = module
```
これはリファレンスポリシーにおいてModulePolicyでもneverallowが使用していることを示している。
公式ドキュメントの誤記？

### base policyとmodule policyの切り分け方は？
module.confに書かれた内容でbase moduleとしてコンパイルされるのか、policy moduleとしてコンパイルされるのかわかる。

#### BasePolicy
```
# Layer: kernel
# Module: corenetwork
# Required in base
#
# Policy controlling access to network objects
# 
corenetwork = base
```

#### ModulePolicy
```
# Layer: system
# Module: userdomain
#
# Policy for user domains
# 
userdomain = module
```

## permissive ドメインについて
ドメインをpermissiveにする方法は以下の2つがある
### permissiveステートメントを使う
Module Policyでも使用可能
ifステートメントの中では使えない。
permissiveを付与するppを作れば、enforceモードから部分的にpermissiveに切り替えることもできそう。
```
# This is the simple statement that would allow permissive mode
# to be set on the httpd_t domain, however this statement is
# generally built into a loadable policy module so that the
# permissive mode can be easily removed by removing the module.
# 
permissive httpd_t
```


### semanageを使う
permissive付与

```
root@qemux86:~# semanage permissive -a unconfined_t
```

permissive剥奪

```
root@qemux86:~# semanage permissive -d unconfined_t

```

## Android公式 SELinux
- https://source.android.com/security/selinux/
- https://source.android.com/security/selinux/concepts
- https://source.android.com/security/selinux/implement
- https://source.android.com/security/selinux/customize
- https://source.android.com/security/selinux/validate
- https://source.android.com/security/selinux/device-policy




