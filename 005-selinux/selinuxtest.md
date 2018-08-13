# 課題
SELinuxのセキュリティポリシファイル作成時、設計上拒否されるべきアクセスベクタが誤設定により許可されてしまうケースが考えられる。設計→ポリシファイルの作成時に混入する
誤設定を是正する手法を考える必要がある。
## どのような種別の誤設定があるか
SELinuxはホワイトリスト方式に基づくアクセス制御を行う。セキュリティポリシファイルには許可すべきアクセスベクタを記載する。

### A.許可すべきアクセスベクタの設定の失念
#### 概要
設計上許可されるべきアクセスベクタのセキュリティポリシファイルへの記載を失念するパターン。
#### リスク
セキュリティポリシファイルに対応するプロセスの挙動に不具合が出る。
上記はテストで発見が可能。
セキュリティの脆弱性となるような不具合ではない。


### B.拒否すべきアクセスベクタを許可設定する
#### 概要
以下のようなケースにより不要な許可設定がセキュリティポリシファイルへ残置される。
+ 設計の変更により、不要になったアクセスベクタ許可設定を消し忘れる。
+ 誤記により無関係なアクセスベクタを許可してしまう。
#### リスク
セキュリティポリシファイルに対応するプロセスの挙動に不具合は出ない。
セキュリティの脆弱性となる許可アクセスベクタが生まれてしまう

# B：「無用な許可設定がされる」に対する分析
## なぜなぜ
* B1 新規設計の段階で無用な許可設定を追加してしまう。
* B1-1 アクセスベクタ設定者が解釈を誤り、無用な許可設定を追加してしまう。
* B1-1-1 設計書が定型化されておらず、誤解を生みやすい
* B1-2 アクセスベクタ設定者が誤記により、無用な許可設定を追加してしまう。
* B1-2-1 アクセスベクタ設定者の疲労により集中力が下がり、誤記をしてしまう。
* B1-2-2 アクセスベクタ設定の記述方法が手順化されておらず、誤記をしてしまう。
* B1-3 レビューアが無用な許可設定に気づかない
* B1-3-1 アクセスポリシ記述言語に不慣れなレビューアが無用な許可設定に気づかない。
* B1-3-2 セキュリティポリシファイルのテキスト量(レビュー範囲)が多くなり許可設定に気づかない。
* B1-3-3 レビューあの疲労により集中力が下がり、無用な許可設定に気づかない。
* B1-4 テストによって無用な許可設定が検出されない
* B2 設計変更の段階で無用な許可設定を追加してしまう。
* B2-1 B1-1と同件
* B2-2 B1-2と同件
* B2-3 B1-3と同件
* B2-4 B1-4と同件
* B2-5 アクセスベクタ設定者が削除箇所を認識せず、不要になった許可設定を消し忘れる。
* B2-5-1 B1-1-1と同件
* B2-5-2 削除箇所が変更履歴として管理されていない

## 要因と現状→真因の抽出
* B1-1-1 : 定型化されている
* B1-2-1 : ○　：　
* B1-2-2 : 手順化されている
* B1-3-1 : 専門家がレビューしている
* B1-3-2 : ◎　：　ポリシー記述言語は読みにくい
* B1-3-3 : ○
* B1-4 : ◎
* B2-5-2 : 管理されている

## 対策
* B1-2-1 : マクロによる設定の簡単化
* B1-3-2 : マクロによる設定の可読性向上
* B1-3-3 : マクロによる設定の可読性向上
* B1-4 : テスト手段を考える

# 無用な許可設定を検出するテスト手段を考える
## テストの範囲は？
設計には「許可する箇所」しか記載されていない。
テストしたいのは「拒否する箇所」。
「拒否する箇所」は「許可する箇所」以外のところになるので
設計書から「拒否する箇所」を論理的に導出しテストケースにするのが王道。
```
+--------------------------------------------+
| which access should be denied? (test case) |-----+
+--------------------------------------------+     |
   |             |              |                  |
   |           review         test                test
   |             |              |                  |
+------+     +-------+      +-------+      +-------------+
|spec  | --->|PolFile| ---> |Module | ---> |Mod behavior |
+------+     +-------+      +-------+      +-------------+

```
### Mod behaviorを検証する場合

#### テストケースの量は？
(ObjectClass数×ObjectClass毎のアクセスベクタ数) × タイプ数
※ObjectClass毎のアクセスベクタ数はObjectClassに依存する。例えばfdに対するアクセスベクタはuseだけなのに対し、fileに対するアクセスベクタはread,write,execなど様々。

ObjectClass×ObjectClass毎のアクセスベクタ数=2052

タイプ数は実行環境でseinfoを実行するとわかる。standardの場合4000個以上

#### 課題 
 単純に「拒否する箇所」全部とすると2052×4000個のアクセステストが必要になる。
テストケースを作成するのも無理があるし、実行時間も無理がある。
#### 確認したいこと
#### 絶対使わないObjectClassをneverallowにして2052はどこまで減らせるか

select count(*) from CLASS JOIN ATOC ON CLASS.cname == ATOC.cname WHERE neverallow==0;

472

#### オブジェクトクラスとタイプの関係を知ることは可能か？
オブジェクトクラスとアトリビュートの関係がどうなっているか知る必要がある。
例えば以下を実行するとfile_typeのアトリビュートが付いているタイプ一覧が
得られる
```
seinfo -a file_type -x 
```
#### 472 * 4000は離散値だから境界値分析はできない
全件テストしないと意味がない。
しかし、472 * 4000のテストをやるのは物理的に無理。約200万件？

### Modlueを検証する場合
以下のコマンドでudev_tがfileクラスに属するtypeに対してどのようなアクセスが許可
されているか確認することができる。これを許可リストと付きあわせてみることを自動化
すれば良い。

バイナリ検証の利点：実行環境に近い。改ざんしにくい。SELinuxが扱うデータ。

```
sesearch /etc/selinux/minimum/policy/policy.30 -A -s udev_t -c file
```