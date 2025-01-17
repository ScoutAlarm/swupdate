/*
 * Author: Christian Storm
 * Copyright (C) 2017, Siemens AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <util.h>
#include <sslapi.h>

struct cryptdata {
	unsigned char *key;
	unsigned char *iv;
	unsigned char *crypttext;
};

static void hex2bin(unsigned char *dest, const unsigned char *source)
{
	unsigned int val;
	for (unsigned int i = 0; i < strlen((const char *)source); i += 2) {
		val = from_ascii((const char *)&source[i], 2, LG_16);
		dest[i / 2] = val;
	}
}

static void do_crypt(struct cryptdata *crypt, unsigned char *CRYPTTEXT, unsigned char *PLAINTEXT)
{
	int len;
	void *dcrypt = swupdate_DECRYPT_init(crypt->key, crypt->iv);
	assert_non_null(dcrypt);

	unsigned char *buffer = calloc(1, strlen((const char *)CRYPTTEXT) + EVP_MAX_BLOCK_LENGTH);
	int ret = swupdate_DECRYPT_update(dcrypt, buffer, &len, crypt->crypttext, strlen((const char *)CRYPTTEXT) / 2);
	assert_true(ret >= 0);
	assert_true(len == 0);

	ret = swupdate_DECRYPT_final(dcrypt, crypt->crypttext, &len);
	assert_true(ret == 0);
	assert_true(len == (int)strlen((const char *)PLAINTEXT));
	assert_true(strncmp((const char *)buffer, (const char *)PLAINTEXT, len) == 0);
	free(buffer);
}

static void test_crypt_1(void **state)
{
	(void)state;

	unsigned char KEY[] = "E5E9FA1BA31ECD1AE84F75CAAA474F3A663F05F412028F81DA65D26EE56424B2";
	unsigned char IV[] = "E93DA465B309C53FEC5FF93C9637DA58";
	unsigned char CRYPTTEXT[] = "E4B7745CA14039555CECD548BB33E0C3";
	unsigned char PLAINTEXT[] = "CRYPTTEST";

	struct cryptdata crypt;
	hex2bin((crypt.key = calloc(1, strlen((const char *)KEY))), KEY);
	hex2bin((crypt.iv = calloc(1, strlen((const char *)IV))), IV);
	hex2bin((crypt.crypttext = calloc(1, strlen((const char *)CRYPTTEXT))), CRYPTTEXT);

	do_crypt(&crypt, &CRYPTTEXT[0], &PLAINTEXT[0]);

	free(crypt.key);
	free(crypt.iv);
	free(crypt.crypttext);
}

static void test_crypt_2(void **state)
{
	(void)state;

	unsigned char KEY[] = "69D54287F856D30B51B812FDF714556778CF31E1B104D9C68BD90C669C37D1AB";
	unsigned char IV[] = "E7039ABFCA63EB8EB1D320F7918275B2";
	unsigned char CRYPTTEXT[] = "A17EBBB1A28459352FE3A994404E35AA";
	unsigned char PLAINTEXT[] = "CRYPTTEST";

	struct cryptdata crypt;
	hex2bin((crypt.key = calloc(1, strlen((const char *)KEY))), KEY);
	hex2bin((crypt.iv = calloc(1, strlen((const char *)IV))), IV);
	hex2bin((crypt.crypttext = calloc(1, strlen((const char *)CRYPTTEXT))), CRYPTTEXT);

	do_crypt(&crypt, &CRYPTTEXT[0], &PLAINTEXT[0]);

	free(crypt.key);
	free(crypt.iv);
	free(crypt.crypttext);
}

static void test_crypt_failure(void **state)
{
	(void)state;

	unsigned char KEY[] = "E5E9FA1BA31ECD1AE84F75CAAA474F3A663F05F412028F81DA65D26EE56424B2";
	unsigned char IV[] = "E93DA465B309C53FEC5FF93C9637DA58";
	unsigned char CRYPTTEXT[] = "CAFECAFECAFECAFECAFECAFECAFECAFE";

	struct cryptdata crypt;
	hex2bin((crypt.key = calloc(1, strlen((const char *)KEY))), KEY);
	hex2bin((crypt.iv = calloc(1, strlen((const char *)IV))), IV);
	hex2bin((crypt.crypttext = calloc(1, strlen((const char *)CRYPTTEXT))), CRYPTTEXT);

	int len;
	void *dcrypt = swupdate_DECRYPT_init(crypt.key, crypt.iv);
	assert_non_null(dcrypt);

	unsigned char *buffer = calloc(1, strlen((const char *)CRYPTTEXT) + EVP_MAX_BLOCK_LENGTH);
	int ret = swupdate_DECRYPT_update(dcrypt, buffer, &len, crypt.crypttext, strlen((const char *)CRYPTTEXT) / 2);
	ret = swupdate_DECRYPT_final(dcrypt, crypt.crypttext, &len);
	assert_true(ret != 0);
	free(buffer);

	free(crypt.key);
	free(crypt.iv);
	free(crypt.crypttext);
}

int main(void)
{
	int error_count = 0;
	const struct CMUnitTest crypt_tests[] = {
		cmocka_unit_test(test_crypt_1),
		cmocka_unit_test(test_crypt_failure),
		cmocka_unit_test(test_crypt_2)
	};
	error_count += cmocka_run_group_tests_name("crypt", crypt_tests, NULL, NULL);
	return error_count;
}
