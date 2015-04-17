#ifndef YYSSL_H
#define YYSSL_H

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

#include <string>

class YYSSL
{
public:
	static void SSLInit()
	{
		SSL_library_init();
		SSL_load_error_strings();
	}
public:
	class YYRSA
	{
		static const int kBits = 1024;
		static const int kBytes = 1024 / 8;
		RSA * _publicKey;
		RSA * _privateKey;
	public:
		/*
		 *	RSA
		 */
		static bool GenRSAKey(RSA ** publickey, RSA ** privatekey)
		{
			RSA *rsa = RSA_generate_key(kBits, RSA_F4, 0, 0);
			if (!rsa)
			{
				// ERR_get_error();
				return false;
			}
			if (RSA_check_key(rsa) <= 0)
				return false;

			*privatekey = RSAPrivateKey_dup(rsa);
			*publickey = RSAPublicKey_dup(rsa);

			RSA_free(rsa);
			return true;
		}
		static RSA * RSAStringToPubKey(std::string & buf)
		{
			const char * p = buf.c_str();
			return d2i_RSAPublicKey(NULL, (const unsigned char**)&p, (long)buf.length());
		}
		static RSA * RSAStringToPrivKey(std::string & buf)
		{
			const char * p = buf.c_str();
			return d2i_RSAPrivateKey(NULL, (const unsigned char**)&p, (long)buf.length());
		}
		static bool RSAPubKeyToString(RSA * pubickey, std::string & buf)
		{
			if (!pubickey)
				return false;
			unsigned char temp[kBits],*p;
			memset(temp, 0 ,sizeof(temp));
			p = temp;
			int keylen = i2d_RSAPublicKey(pubickey, &p);
			if (keylen <= 0)
				return false;

			buf.append((const char*)temp, keylen);
			return true;
		}
		static bool RSAPrivKeyToString(RSA * privatekey, std::string & buf)
		{
			if (!privatekey)
				return false;
			unsigned char temp[kBits],*p;
			memset(temp, 0 ,sizeof(temp));
			p = temp;
			int keylen = i2d_RSAPrivateKey(privatekey, &p);
			if (keylen <= 0)
				return false;

			buf.append((const char*)temp, keylen);
			return true;
		}
	public:
		YYRSA()
		{
			_publicKey = _privateKey = NULL;
		}
		~YYRSA()
		{
			if (_publicKey)
				RSA_free(_publicKey);
			if (_privateKey)
				RSA_free(_privateKey);
			_publicKey = _privateKey = NULL;
		}
		void SetRSAKey(RSA * publickey, RSA * privatekey)
		{
			if (publickey)
			{
				if (_publicKey) RSA_free(_publicKey);
				_publicKey = publickey;
			}
			if (privatekey)
			{
				if (_privateKey) RSA_free(_privateKey);
				_privateKey = privatekey;
			}
		}
		void SetRSAKey(std::string & publickey, std::string & privatekey)
		{
			RSA * pubkey = NULL;
			RSA * prikey = NULL;
			if (!publickey.empty())
				pubkey = RSAStringToPubKey(publickey);
			if (!privatekey.empty())
				prikey = RSAStringToPrivKey(privatekey);
			SetRSAKey(pubkey, prikey);
		}
		bool RSAPublicEncrypt(std::string & inbuf, std::string & outbuf)
		{
			if (!_publicKey) return false;
			char from[kBytes];
			char to[kBytes];

			char offsetLen = inbuf.size() % kBytes;
			outbuf.push_back(offsetLen);

			std::string::size_type offset = 0;
			while (offset < inbuf.size())
			{
				memset(from, 0, sizeof(from));
				int len = inbuf.copy(from, kBytes, offset);
				int r = RSA_public_encrypt(kBytes, (const unsigned char*)from, (unsigned char*)to, _publicKey, RSA_NO_PADDING);
				if (r < 0)
				{
					// unsigned long err = ERR_get_error();
					return false;
				}
				outbuf.append(to, r);
				offset += len;
			}
			return true;
		}
		bool RSAPrivateDecrypt(std::string & inbuf, std::string & outbuf)
		{
			if (!_privateKey) return false;
			char from[kBytes];
			char to[kBytes];

			int inbufsize = inbuf.size();
			if (inbufsize % kBytes != 1)
				return false;

			std::string::size_type offset = 0;
			char offsetLen = *inbuf.c_str();
			offset++;

			while (offset < inbufsize)
			{
				int len = inbuf.copy(from, kBytes, offset);
				int r = RSA_private_decrypt(kBytes, (const unsigned char*)from, (unsigned char*)to, _privateKey, RSA_NO_PADDING);
				if (r < 0)
				{
					// unsigned long err = ERR_get_error();
					return false;
				}

				offset += len;
				if (offsetLen != 0 && offset >= inbufsize)
					outbuf.append(to, offsetLen);
				else
					outbuf.append(to, r);
			}
			return true;
		}
	};
	/*
	 *	Base64
	 */
	class YYBase64
	{
	public:
		static bool base64encode(std::string & inbuf, std::string & outbuf)
		{
			int base64Len = (((inbuf.size() + 2) / 3) * 4) + 1;	//Base 64 text length
			int pemLen = base64Len + base64Len / 64;			// PEM adds a newline every 64 bytes

			outbuf.resize(pemLen);

			EVP_ENCODE_CTX ctx;
			EVP_EncodeInit(&ctx);

			int len, len2;
			EVP_EncodeUpdate(&ctx, (unsigned char*)outbuf.data(), &len, (const unsigned char*)inbuf.c_str(), inbuf.length());
			EVP_EncodeFinal(&ctx, (unsigned char*)outbuf.data() + len, &len2);

			if (outbuf.size() > len + len2)
				outbuf.erase(len + len2, outbuf.size() - len - len2);

			return true;
		}
		static bool base64decode(std::string & inbuf, std::string & outbuf)
		{
			int orgLen = (((inbuf.size() + 2) / 4) * 3) + 1;

			outbuf.resize(orgLen);

			int len, len2;
			EVP_ENCODE_CTX ctx;
			EVP_DecodeInit(&ctx);
			EVP_DecodeUpdate(&ctx,(unsigned char*)outbuf.data(), &len,(unsigned char*)inbuf.c_str(), inbuf.size());
			EVP_DecodeFinal(&ctx, (unsigned char*)outbuf.data() + len, &len2);

			if (outbuf.size() > len + len2)
				outbuf.erase(len + len2, outbuf.size() - len - len2);
			return true;
		}
	};
	/*
	 *	AES
	 */
	class YYAES
	{
	public:
		static void GenAESKey(unsigned char * aes_keybuf, int bytes)
		{
			RAND_bytes(aes_keybuf, bytes);
		}
	public:
		YYAES()
		{
			memset(&_encryptkey, 0, sizeof(AES_KEY));
			memset(&_decryptkey, 0, sizeof(AES_KEY));
		}
		void AESSetKey(unsigned char * aesKkey, int bytes)
		{
			if (!aesKkey)
				return;
			AES_set_encrypt_key(aesKkey, bytes * 8, &_encryptkey);
			AES_set_decrypt_key(aesKkey, bytes * 8, &_decryptkey);
		}
		bool AESEncrypt(std::string & inbuf, std::string & outbuf)
		{
			char from[AES_BLOCK_SIZE];
			char to[AES_BLOCK_SIZE];
			char offsetLen = inbuf.size() % AES_BLOCK_SIZE;
			outbuf.push_back(offsetLen);

			std::string::size_type offset = 0;
			while (offset < inbuf.size())
			{
				inbuf.copy(from, AES_BLOCK_SIZE, offset);
				AES_encrypt((unsigned char*)from, (unsigned char*)to, &_encryptkey);
				outbuf.append(to, AES_BLOCK_SIZE);
				offset += AES_BLOCK_SIZE;
			}
			return true;
		}
		bool AESDecrypt(std::string & inbuf, std::string & outbuf)
		{
			if (inbuf.size() < 1 || inbuf.size() % AES_BLOCK_SIZE != 1)
				return false;

			char from[AES_BLOCK_SIZE];
			char to[AES_BLOCK_SIZE];
			char offsetLen = inbuf[0];

			std::string::size_type offset = 1;
			while (offset < inbuf.size())
			{
				inbuf.copy(from, AES_BLOCK_SIZE, offset);
				AES_decrypt((unsigned char*)from, (unsigned char*)to, &_decryptkey);

				offset += AES_BLOCK_SIZE;
				if (offsetLen != 0 && offset >= inbuf.size())
					outbuf.append(to, offsetLen);
				else
					outbuf.append(to, AES_BLOCK_SIZE);
			}
			return true;
		}
	private:
		AES_KEY		_encryptkey;
		AES_KEY		_decryptkey;
	};
};

#endif // YYSSL_H
