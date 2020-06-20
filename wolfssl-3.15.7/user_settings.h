//#define DEBUG_WOLFSSL

#ifndef HAVE_LIBZ
#define HAVE_LIBZ
#endif
#define HAVE_ECC
#define HAVE_AESGCM
//#define HAVE_FIPS
// googleで必要
#define HAVE_TLS_EXTENSIONS
#define HAVE_SNI
#define HAVE_SUPPORTED_CURVES
// AzureのMQTToWSで必要、Open Weather Mapでは害
#define HAVE_SECURE_RENEGOTIATION
#define HAVE_SERVER_RENEGOTIATION_INFO
#define WOLFSSL_SECURE_RENEGOTIATION_ON_BY_DEFAULT

#define ECC_TIMING_RESISTANT
#define TFM_TIMING_RESISTANT

#define WOLFSSL_RIPEMD
#define WOLFSSL_SHA512
#define WOLFSSL_SHA384
#define WOLFSSL_SMALL_STACK
// curlで使用している
#define WOLFSSL_ALLOW_TLSV10

#define WOLFSSL_CERT_EXT
//#define OPENSSL_EXTRA	GR-SAKURAでROMが足らなくなる

#define WC_RSA_BLINDING

#define SINGLE_THREADED

#define CUSTOM_RAND_GENERATE_SEED custom_rand_generate_seed
