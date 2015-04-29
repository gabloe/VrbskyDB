

#ifndef CONFIG_H_
#define CONFIG_H_

#if defined(_WIN32) || defined(_WINNT)
#else
#define RAPIDJSON_SSE2
#endif

#define THREADING 0
#define NUM_THREADS 0
#define EXPERIMENTAL 1

#endif
