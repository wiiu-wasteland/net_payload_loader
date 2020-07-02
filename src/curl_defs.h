#ifndef CURL_DEFS_H
#define	CURL_DEFS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CURLOPT_WRITEDATA       10001
#define CURLOPT_URL             10002
#define CURLOPT_WRITEFUNCTION   20011

#define CURLINFO_RESPONSE_CODE  0x200002

#ifdef __cplusplus
}
#endif

#endif	/* CURL_DEFS_H */
