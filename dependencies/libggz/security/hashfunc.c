#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#ifdef USE_GCRYPT
# include <gcrypt.h>
#endif
#include "ggz.h"
#include "hashfunc.h"

#ifdef USE_GCRYPT

static hash_t hash_create_private(const char *algo, const char *text, const char *secret)
{
	gcry_md_hd_t handle;
	const unsigned int flags = (secret ? GCRY_MD_FLAG_HMAC : 0);
	int unsigned algos[] = {GCRY_MD_MD5, 0};
	int ret;
	unsigned int i;
	hash_t hash;
	gcry_error_t error;

	hash.hash = NULL;
	hash.hashlen = 0;

	if((!algo) || (!text)) return hash;

	if(!strcmp(algo, "md5")) algos[0] = GCRY_MD_MD5;
	else if(!strcmp(algo, "sha1")) algos[0] = GCRY_MD_SHA1;
	else if(!strcmp(algo, "ripemd160")) algos[0] = GCRY_MD_RMD160;
	else return hash;

	if(!gcry_check_version("1.1.10"))
	{
		fprintf(stderr, "Error: gcrypt version is too old.\n");
		return hash;
	}

	error = gcry_md_open(&handle, 0, flags);
	if(error != GPG_ERR_NO_ERROR)
	{
		fprintf(stderr, "Error: couldn't create handle: %s.\n", gcry_strerror(error));
		return hash;
	}

	if(secret) gcry_md_setkey(handle, secret, strlen(secret));

	for(i = 0; algos[i]; i++)
	{
		ret = gcry_md_enable(handle, algos[i]);
		if(ret)
		{
			fprintf(stderr, "Error: couldn't add algorithm '%s'.\n", gcry_md_algo_name(algos[i]));
			return hash;
		}
	}

	gcry_md_write(handle, text, strlen(text));
	hash.hashlen = gcry_md_get_algo_dlen(algos[0]);
	hash.hash = ggz_malloc(hash.hashlen);
	if(hash.hash){
		hash.hash = memcpy(hash.hash, gcry_md_read(handle, algos[0]), hash.hashlen);
	}else{
		hash.hashlen = 0;
	}

	gcry_md_close(handle);

	return hash;
}

hash_t ggz_hash_create(const char *algo, const char *text)
{
	return hash_create_private(algo, text, NULL);
}

hash_t ggz_hmac_create(const char *algo, const char *text, const char *secret)
{
	return hash_create_private(algo, text, secret);
}

#else

hash_t ggz_hash_create(const char *algo, const char *text)
{
	hash_t hash;

	hash.hash = NULL;
	hash.hashlen = 0;

	return hash;
}

hash_t ggz_hmac_create(const char *algo, const char *text, const char *secret)
{
	hash_t hash;

	hash.hash = NULL;
	hash.hashlen = 0;

	return hash;
}

#endif

