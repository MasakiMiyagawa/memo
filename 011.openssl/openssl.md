# openssl
## 参考
- https://www.openssl.org/docs/man1.0.2/crypto/PEM_read_X509.html
- https://stackoverflow.com/questions/5549942/openssl-how-to-read-a-crt-file
- http://d.hatena.ne.jp/ytakano/20130118/1358504084
- https://wiki.openssl.org/index.php/Simple_TLS_Server
- http://blog.sarabande.jp/post/82087204080

## httpsクライアントを作る

### ubuntu 16.04 パッケージ準備

```
$sudo aptitude install libssl-dev
```

## 乱数生成処理の差し替えのやり方

```
https-cli-old/
├── alternate-rand-method.c <== このファイルを参照。
├── https-client.c
└── https_client.mk
```

## サーバ証明書の独自検証処理のやり方

```
https-cli
├── https_client.c <== このファイルを参照
├── https_client.mk
└── setup_crt.sh
```

```

// SSL_set_verify(ssl, SSL_VERIFY_PEER, ssl_sample_verify_callback);で事前にコールバックを登録しておく

static int ssl_sample_verify_callback(int preverify_ok, X509_STORE_CTX *x509)
{
        /*
         * 独自証明書検証関数のsample実装。
         * preverify_ok=0:事前の検証には失敗している
         * preverify_ok=1:事前の検証に成功している
         * この関数のリターン値により検証結果を上書きできる。
         * return 0:検証エラー
         * return 1:検証成功
         * このサンプル実装ではX509証明書を取得し、署名者名を標準出力した後
         * 検証を強制的に成功にする。
         */
        X509 *cert = X509_STORE_CTX_get_current_cert(x509);
        X509_print_fp(stdout, cert);
        return 1;
}

```

## クライアント証明書要求への応答の仕方

```
https-cli
├── https_client.c <== このファイルを参照
├── https_client.mk
└── setup_crt.sh
```

以下のコードは、サーバからクライアント証明書の要求が来た時、証明書ファイル・秘密鍵ファイルを読み込み、X509オブジェクト及びEVP_PKEYオブジェクトとしてサーバに返却する。

```
//SSL_CTX_set_client_cert_cb(ctx, ssl_sample_client_cert_cb);
でコールバックを事前に登録しておく

static int ssl_sample_client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
	X509 *_x509 = NULL;
	EVP_PKEY *_pkey = NULL;
	FILE* f = fopen(crt_file, "r");
    	if (f == NULL) {
		perror("fopen:");
		exit(-1);
	}
        
	_x509 = PEM_read_X509(f, NULL, NULL, NULL);
       	if (_x509 == NULL) {
		fclose(f);
		perror("PEM_read_X509:");
		exit (-1);
	}
	
	fclose(f);
	f = NULL;
	*x509 = _x509;

	f = fopen(key_file, "r");
	if (f == NULL) {
		perror("flopen:");
		X509_free(_x509);
		exit(-1);
	}

	_pkey = PEM_read_PrivateKey(f, NULL, NULL, NULL);
	if (_pkey == NULL) {
		perror("PEM_read_PrivateKey:");
		X509_free(_x509);
		fclose(f);
		exit (-1);
	}

	fclose(f);
	f = NULL;
	*pkey = _pkey;
	return 1;
}
```

##サンプルのhttpsサーバ、httpsクライアントを動かしてみる

以下の2つのディレクトリを使う
```
https-srv/
├── https_server.c
├── https_server.mk
└── setup_crt.sh

https-cli
├── https_client.c
├── https_client.mk
└── setup_crt.sh
```

### ビルド

client

```
$ make -f https_client.mk 
cc -c -Wall -O0 -g -I/home/miyagawa/work/github/memo/011.openssl/https-cli https_client.c
cc -o https_client -Wall -O0 -g https_client.o -lssl -lcrypto -lstdc++
$ ls
https_client  https_client.c  https_client.mk  https_client.o  setup_crt.sh

```
server

```
$ make -f https_server.mk 
cc -c -Wall -O0 -g -I/home/miyagawa/work/github/memo/011.openssl/https-srv https_server.c
cc -o https_server -Wall -O0 -g https_server.o -lssl -lcrypto -lstdc++ -lm
$ ls
https_server  https_server.c  https_server.mk  https_server.o  setup_crt.sh
```

### サーバ証明書・クライアント証明書の作成
実際には検証行わず、標準出力して強制的に検証OKとなる。
公開鍵を相手に渡しておく必要はない。

client　※serverも同じ
```
$ ./setup_crt.sh 
Generating RSA private key, 2048 bit long modulus
....................................+++
..........................................+++
e is 65537 (0x10001)
Enter pass phrase:<秘密鍵フレーズ>
Verifying - Enter pass phrase:<秘密鍵フレーズ>
Enter pass phrase for client.key:<秘密鍵フレーズ>
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:<任意>
State or Province Name (full name) [Some-State]:<任意>
Locality Name (eg, city) []:<任意>
Organization Name (eg, company) [Internet Widgits Pty Ltd]:<任意>
Organizational Unit Name (eg, section) []:<任意>
Common Name (e.g. server FQDN or YOUR name) []:<任意>
Email Address []:<任意>

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:<任意>
An optional company name []:<任意>
Signature ok
subject=/C=JP/ST=Tokyo/L=Shinjuku/O=OIOI
Getting Private key
Enter pass phrase for client.key:<秘密鍵フレーズ>
```

### 動作確認
1.サーバ起動
sudo権限が必要443ポートを使うから
```
$ sudo ./https_server 
Enter PEM pass phrase:<秘密鍵フレーズ>
```
2.クライアント起動
現状localhostで動いているサーバのみアクセス可
```
$ ./https_client 
※まずサーバの証明書を標準出力する
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number: 14900239281640139655 (0xcec848ae8a7b6b87)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=JP, ST=Tokyo, L=Shibuya, O=109
        Validity
            Not Before: Jun 23 15:33:08 2018 GMT
            Not After : Oct 24 15:33:08 3017 GMT
        Subject: C=JP, ST=Tokyo, L=Shibuya, O=109
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:d2:5e:6d:94:68:b8:2b:f6:7b:f8:08:43:00:22:
                    f4:33:f6:37:45:49:1a:e2:a6:40:81:03:65:70:f6:
                    df:45:ea:90:39:35:19:35:7f:36:cf:8a:9a:47:42:
                    ec:fc:a4:fc:8e:16:3b:38:f9:cf:05:c7:2d:4c:83:
                    39:c5:7c:27:4c:37:55:f9:de:a3:74:c1:21:ed:85:
                    3d:03:d4:02:14:b2:88:45:e5:bd:c3:97:bc:4b:77:
                    41:48:97:da:b9:4d:c2:13:00:7f:64:f9:5b:97:5e:
                    2c:1f:8c:83:3f:74:14:c1:59:13:a2:13:d6:63:c2:
                    6b:a1:a9:79:b1:7b:59:20:c0:ed:31:d0:7a:aa:e5:
                    bc:f1:8d:52:4a:b8:e9:91:de:6b:11:ec:eb:2a:1e:
                    0f:4a:4e:9a:58:1e:62:d1:19:7b:98:d0:9c:d9:e7:
                    28:77:87:b4:60:e8:cb:0b:21:1d:63:29:ac:ef:71:
                    8a:22:0e:bd:40:df:89:16:57:e6:91:40:8c:c6:c2:
                    12:d0:db:11:d5:d5:a2:2d:9b:82:54:a7:5d:91:d5:
                    1a:cd:f5:ac:87:cb:a1:2d:e5:dd:0e:ab:3d:c7:b8:
                    20:5a:2b:d1:2b:eb:6a:37:e0:29:49:28:86:1a:2e:
                    10:b0:eb:63:96:2e:4d:b3:ea:a6:44:b0:06:f2:ce:
                    1d:3f
                Exponent: 65537 (0x10001)
    Signature Algorithm: sha256WithRSAEncryption
         a1:59:8c:2f:80:0b:c1:6b:08:42:2a:fc:8f:d2:f0:a7:8a:47:
         0e:ab:a8:b6:1f:91:7d:37:c2:8f:a0:6a:47:f1:57:cf:11:47:
         1d:db:63:02:c6:22:72:d7:73:fe:96:92:43:97:4a:28:b7:88:
         c5:97:b9:61:db:bf:42:8f:77:b5:6c:97:e5:34:85:39:4d:ed:
         93:d5:a7:e5:52:e0:40:fd:2a:4b:f9:03:1c:a4:41:4a:a9:a6:
         3b:26:8b:ed:fc:a0:45:dc:48:b0:1f:7b:26:84:b4:d8:36:93:
         0f:59:08:e6:cd:cd:52:50:35:79:59:55:f0:25:29:c2:38:fd:
         6c:f8:3a:4d:36:35:e4:8d:af:37:e6:72:98:be:e1:50:a8:1b:
         e2:83:8e:cb:ab:9b:fe:10:64:f5:95:46:ab:4d:94:38:ef:6b:
         17:ea:22:c7:19:d3:09:8d:af:12:b9:ee:de:99:2f:d2:3b:ab:
         c3:73:e6:a9:bf:35:7a:aa:b4:d3:76:90:bf:18:6a:75:1e:2b:
         e1:b2:14:bb:6e:84:01:69:77:1f:1c:f8:c1:54:77:cc:99:04:
         5b:b3:4e:51:80:bb:30:ac:00:e3:d1:88:00:d9:9c:86:5b:f2:
         cd:58:75:ea:23:1d:91:dc:37:b8:6f:e4:50:ac:77:ee:9d:dd:
         b3:1e:b5:82
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number: 14900239281640139655 (0xcec848ae8a7b6b87)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=JP, ST=Tokyo, L=Shibuya, O=109
        Validity
            Not Before: Jun 23 15:33:08 2018 GMT
            Not After : Oct 24 15:33:08 3017 GMT
        Subject: C=JP, ST=Tokyo, L=Shibuya, O=109
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:d2:5e:6d:94:68:b8:2b:f6:7b:f8:08:43:00:22:
                    f4:33:f6:37:45:49:1a:e2:a6:40:81:03:65:70:f6:
                    df:45:ea:90:39:35:19:35:7f:36:cf:8a:9a:47:42:
                    ec:fc:a4:fc:8e:16:3b:38:f9:cf:05:c7:2d:4c:83:
                    39:c5:7c:27:4c:37:55:f9:de:a3:74:c1:21:ed:85:
                    3d:03:d4:02:14:b2:88:45:e5:bd:c3:97:bc:4b:77:
                    41:48:97:da:b9:4d:c2:13:00:7f:64:f9:5b:97:5e:
                    2c:1f:8c:83:3f:74:14:c1:59:13:a2:13:d6:63:c2:
                    6b:a1:a9:79:b1:7b:59:20:c0:ed:31:d0:7a:aa:e5:
                    bc:f1:8d:52:4a:b8:e9:91:de:6b:11:ec:eb:2a:1e:
                    0f:4a:4e:9a:58:1e:62:d1:19:7b:98:d0:9c:d9:e7:
                    28:77:87:b4:60:e8:cb:0b:21:1d:63:29:ac:ef:71:
                    8a:22:0e:bd:40:df:89:16:57:e6:91:40:8c:c6:c2:
                    12:d0:db:11:d5:d5:a2:2d:9b:82:54:a7:5d:91:d5:
                    1a:cd:f5:ac:87:cb:a1:2d:e5:dd:0e:ab:3d:c7:b8:
                    20:5a:2b:d1:2b:eb:6a:37:e0:29:49:28:86:1a:2e:
                    10:b0:eb:63:96:2e:4d:b3:ea:a6:44:b0:06:f2:ce:
                    1d:3f
                Exponent: 65537 (0x10001)
    Signature Algorithm: sha256WithRSAEncryption
         a1:59:8c:2f:80:0b:c1:6b:08:42:2a:fc:8f:d2:f0:a7:8a:47:
         0e:ab:a8:b6:1f:91:7d:37:c2:8f:a0:6a:47:f1:57:cf:11:47:
         1d:db:63:02:c6:22:72:d7:73:fe:96:92:43:97:4a:28:b7:88:
         c5:97:b9:61:db:bf:42:8f:77:b5:6c:97:e5:34:85:39:4d:ed:
         93:d5:a7:e5:52:e0:40:fd:2a:4b:f9:03:1c:a4:41:4a:a9:a6:
         3b:26:8b:ed:fc:a0:45:dc:48:b0:1f:7b:26:84:b4:d8:36:93:
         0f:59:08:e6:cd:cd:52:50:35:79:59:55:f0:25:29:c2:38:fd:
         6c:f8:3a:4d:36:35:e4:8d:af:37:e6:72:98:be:e1:50:a8:1b:
         e2:83:8e:cb:ab:9b:fe:10:64:f5:95:46:ab:4d:94:38:ef:6b:
         17:ea:22:c7:19:d3:09:8d:af:12:b9:ee:de:99:2f:d2:3b:ab:
         c3:73:e6:a9:bf:35:7a:aa:b4:d3:76:90:bf:18:6a:75:1e:2b:
         e1:b2:14:bb:6e:84:01:69:77:1f:1c:f8:c1:54:77:cc:99:04:
         5b:b3:4e:51:80:bb:30:ac:00:e3:d1:88:00:d9:9c:86:5b:f2:
         cd:58:75:ea:23:1d:91:dc:37:b8:6f:e4:50:ac:77:ee:9d:dd:
         b3:1e:b5:82
※サーバから証明書要求が来るので秘密鍵フレーズ入力
Enter PEM pass phrase:
※以下はサーバからの応答メッセージ
Conntect to localhost
GET  HTTP/1.0
Host: localhost

HTTP/1.1 200 OK
Content-Type:	 text/html
Content-Length: 11
Connection: Close

hello world
```

3.サーバ動作確認

クライアントの証明書が標準出力される

```
$ sudo ./https_server 
Enter PEM pass phrase:
Connected from: 127.0.0.1:44134
simple_https_verify_callback
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number: 17100695306109529265 (0xed51dfc70cd92cb1)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=JP, ST=Tokyo, L=Shinjuku, O=OIOI
        Validity
            Not Before: Jun 23 15:29:31 2018 GMT
            Not After : Oct 24 15:29:31 3017 GMT
        Subject: C=JP, ST=Tokyo, L=Shinjuku, O=OIOI
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:df:82:51:d1:9a:ea:9a:65:57:8a:e4:14:3a:55:
                    b5:9b:f0:56:0b:5d:90:46:1b:86:87:9f:90:ad:50:
                    9a:80:e2:63:6c:44:b1:23:f0:c5:c3:82:cd:07:91:
                    5a:e4:c4:d7:5e:56:7a:ce:1c:5c:14:28:b3:8c:71:
                    f7:9f:bc:37:11:e2:f2:98:81:96:e7:13:1f:38:18:
                    44:3f:bc:3c:b9:0e:cd:9b:c5:38:06:a1:46:43:06:
                    1e:a9:14:65:10:4c:cc:71:d8:da:82:eb:bf:de:33:
                    62:ef:9b:93:cf:38:81:71:cf:57:fd:70:ee:8c:d5:
                    6a:e5:57:68:9a:b9:88:6e:1f:87:0f:59:48:ed:44:
                    92:61:e5:8b:60:d1:e9:13:76:0b:81:ef:e3:0a:29:
                    5a:16:0e:0b:6a:0f:c0:50:81:39:c2:c5:33:ce:54:
                    45:b0:f4:79:f8:f2:29:cb:fc:6a:03:16:56:a7:46:
                    e1:21:6e:b6:d3:17:f4:85:67:a6:eb:a7:b3:f2:74:
                    9a:35:45:f2:46:b7:04:68:f8:93:89:95:e6:32:53:
                    0c:f3:66:16:c8:de:3c:03:91:07:df:28:d7:02:32:
                    4b:5b:e6:9a:4d:95:fc:d1:69:a0:54:18:47:c2:91:
                    40:8f:b0:f4:80:34:77:bd:ce:1e:12:ce:b2:c6:db:
                    ac:a7
                Exponent: 65537 (0x10001)
    Signature Algorithm: sha256WithRSAEncryption
         6c:47:4e:b7:2a:4b:8e:9c:0e:c7:38:9e:39:cb:06:6f:9d:13:
         2e:1a:3d:21:81:6d:c1:fd:d5:35:64:ee:c1:9f:58:88:0b:6b:
         4f:03:c7:94:fc:a0:17:2d:09:99:fb:1d:d2:96:73:93:8a:aa:
         10:49:a9:41:6e:0b:cf:dc:67:9b:c9:1c:be:09:02:0f:be:8b:
         7a:d8:c2:87:bc:d8:93:01:4f:db:f0:84:06:4c:48:ac:f1:f0:
         ed:e5:f8:a1:96:14:04:72:18:fb:6f:95:0c:ff:b5:3e:3d:4a:
         ab:c5:3e:07:be:8c:ac:4a:36:05:62:e8:20:43:7f:4b:8f:73:
         6a:c7:56:d1:6a:52:a8:a2:50:f5:30:96:97:5c:33:08:a4:be:
         ce:e3:2e:a8:85:e8:e5:64:3a:f0:ee:ef:3a:86:c0:a6:25:03:
         2d:d7:65:f6:41:cf:02:e5:ba:73:46:84:7b:45:3a:9b:8c:ca:
         a4:66:dd:83:63:ca:f1:54:dc:6a:9e:0b:25:3b:27:36:04:6d:
         81:2c:01:9f:1a:f1:39:f6:9c:89:2b:ac:27:a8:19:79:72:0a:
         f6:d6:69:c7:36:bc:8e:f1:17:7d:d8:61:b5:11:b6:f9:28:b6:
         1d:47:15:8a:23:89:db:ef:f6:90:9b:82:e2:ca:09:fd:3a:92:
         bf:d6:fd:f6
simple_https_verify_callback
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number: 17100695306109529265 (0xed51dfc70cd92cb1)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=JP, ST=Tokyo, L=Shinjuku, O=OIOI
        Validity
            Not Before: Jun 23 15:29:31 2018 GMT
            Not After : Oct 24 15:29:31 3017 GMT
        Subject: C=JP, ST=Tokyo, L=Shinjuku, O=OIOI
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:df:82:51:d1:9a:ea:9a:65:57:8a:e4:14:3a:55:
                    b5:9b:f0:56:0b:5d:90:46:1b:86:87:9f:90:ad:50:
                    9a:80:e2:63:6c:44:b1:23:f0:c5:c3:82:cd:07:91:
                    5a:e4:c4:d7:5e:56:7a:ce:1c:5c:14:28:b3:8c:71:
                    f7:9f:bc:37:11:e2:f2:98:81:96:e7:13:1f:38:18:
                    44:3f:bc:3c:b9:0e:cd:9b:c5:38:06:a1:46:43:06:
                    1e:a9:14:65:10:4c:cc:71:d8:da:82:eb:bf:de:33:
                    62:ef:9b:93:cf:38:81:71:cf:57:fd:70:ee:8c:d5:
                    6a:e5:57:68:9a:b9:88:6e:1f:87:0f:59:48:ed:44:
                    92:61:e5:8b:60:d1:e9:13:76:0b:81:ef:e3:0a:29:
                    5a:16:0e:0b:6a:0f:c0:50:81:39:c2:c5:33:ce:54:
                    45:b0:f4:79:f8:f2:29:cb:fc:6a:03:16:56:a7:46:
                    e1:21:6e:b6:d3:17:f4:85:67:a6:eb:a7:b3:f2:74:
                    9a:35:45:f2:46:b7:04:68:f8:93:89:95:e6:32:53:
                    0c:f3:66:16:c8:de:3c:03:91:07:df:28:d7:02:32:
                    4b:5b:e6:9a:4d:95:fc:d1:69:a0:54:18:47:c2:91:
                    40:8f:b0:f4:80:34:77:bd:ce:1e:12:ce:b2:c6:db:
                    ac:a7
                Exponent: 65537 (0x10001)
    Signature Algorithm: sha256WithRSAEncryption
         6c:47:4e:b7:2a:4b:8e:9c:0e:c7:38:9e:39:cb:06:6f:9d:13:
         2e:1a:3d:21:81:6d:c1:fd:d5:35:64:ee:c1:9f:58:88:0b:6b:
         4f:03:c7:94:fc:a0:17:2d:09:99:fb:1d:d2:96:73:93:8a:aa:
         10:49:a9:41:6e:0b:cf:dc:67:9b:c9:1c:be:09:02:0f:be:8b:
         7a:d8:c2:87:bc:d8:93:01:4f:db:f0:84:06:4c:48:ac:f1:f0:
         ed:e5:f8:a1:96:14:04:72:18:fb:6f:95:0c:ff:b5:3e:3d:4a:
         ab:c5:3e:07:be:8c:ac:4a:36:05:62:e8:20:43:7f:4b:8f:73:
         6a:c7:56:d1:6a:52:a8:a2:50:f5:30:96:97:5c:33:08:a4:be:
         ce:e3:2e:a8:85:e8:e5:64:3a:f0:ee:ef:3a:86:c0:a6:25:03:
         2d:d7:65:f6:41:cf:02:e5:ba:73:46:84:7b:45:3a:9b:8c:ca:
         a4:66:dd:83:63:ca:f1:54:dc:6a:9e:0b:25:3b:27:36:04:6d:
         81:2c:01:9f:1a:f1:39:f6:9c:89:2b:ac:27:a8:19:79:72:0a:
         f6:d6:69:c7:36:bc:8e:f1:17:7d:d8:61:b5:11:b6:f9:28:b6:
         1d:47:15:8a:23:89:db:ef:f6:90:9b:82:e2:ca:09:fd:3a:92:
         bf:d6:fd:f6
※以下はクライアントからのhttpリクエスト
GET  HTTP/1.0
Host: localhost

```
