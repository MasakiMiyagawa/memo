# BlueBorne 
## 関連ウェブページ
- IPA 
https://www.ipa.go.jp/security/ciadr/vul/20170914_blueborne.html
- JPCERT
https://www.jpcert.or.jp/at/2017/at170037.html
- Ubuntu
https://www.jpcert.or.jp/at/2017/at170037.html
- Redhat
https://access.redhat.com/security/vulnerabilities/blueborne
- CVE
https://people.canonical.com/~ubuntu-security/cve/2017/CVE-2017-1000250.html
https://people.canonical.com/~ubuntu-security/cve/2017/CVE-2017-1000251.html

## 概要
- CVE-2017-1000250
Bluetooth Stack(BlueZ) 5.46以降を利用して動作するSDP serverにて脆弱性が開示された。
この脆弱性により攻撃者はbluetoothdのプロセスのアドレス空間より、重大な情報を取得することが可能となる。

- CVE-2017-1000251
Linux Kenrel version 3.3-rc1以降のBlueZにおいて、L2CAPコンフィグレーション応答時の脆弱性(スタックオーバーフロー)をついた攻撃が可能であることが発見された。攻撃者は攻撃対象にて不正なコードをカーネルモードで実行させることができため、ルートが奪われるなどの二次被害が発生する可能性がある。

## 対象カーネル
Linux-3.3-rc1以降すべてのバージョン

## 対策
L2CAPのクライアントは任意のコンフィグレーションデータを送ることが可能であるが、このコンフィグレーションデータを受信するバッファをオーバーフローさせることでLinux Kernelのテキストセクションを不正に書き換えることができるようになる。このコンフィグレーションはBluetoothにおける認証プロセスの前に行われる処理なので認証による回避はできない。RHもUbuntuもLinuxKernelのCONFIG_CC_STACKPROTECTORを有効にしている場合を対象外にしている。CONFIG_CC_STACKPROTECTORはgccのオプション-fstack-protectorを有効にしてLinuxKernelをbuildする。-fstack-protectorが有効化されたプログラムは任意の関数の呼び出し前後でスタックの破壊をチェックし、以上が発生していた場合プロセスをexitさせる。BlueBorneによる攻撃を受けたLinuxKernelがCONFIG_CC_STACKPROTECTORを有効化してビルドしていた場合、Bluetoothdのプロセスが強制終了する。
