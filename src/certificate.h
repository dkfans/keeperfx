#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#ifdef __cplusplus
extern "C" {
#endif

// Returns 1 if signature is valid, 0 otherwise.
// On non-Windows platforms, always returns 0.
int verify_certificate(const char* path);

#ifdef __cplusplus
}
#endif

#endif // CERTIFICATE_H