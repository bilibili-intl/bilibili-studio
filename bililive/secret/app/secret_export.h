#ifndef BILILIVE_SECRET_APP_SECRET_EXPORT_H_
#define BILILIVE_SECRET_APP_SECRET_EXPORT_H_

#if defined(BILILIVE_SECRET_IMPLEMENTATION)
#define SECRET_EXPORT __declspec(dllexport)
#else
#define SECRET_EXPORT __declspec(dllimport)
#endif

#endif  // BILILIVE_SECRET_APP_SECRET_EXPORT_H_