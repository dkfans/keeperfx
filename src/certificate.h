#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char verify_certificates;

// Returns 1 if signature is valid, -1 if certificate verification is disabled, and 0 otherwise.
// On non-Windows platforms, always returns -1.
int verify_certificate(const char* path);

#ifdef __cplusplus
}
#endif

#endif // CERTIFICATE_H