#ifndef STITCHENGINE_EXPORT_H
#define STITCHENGINE_EXPORT_H

#ifdef WIN32
#  if defined(STITCHENGINE_CPP)
#    define STITCHENGINE_EXPORT
#  elif defined(STITCHENGINE_SHARED_LIBRARY )
#    define STITCHENGINE_EXPORT __declspec(dllexport)
#  else
#    define STITCHENGINE_EXPORT __declspec(dllimport)
#  endif
#else
#  define STITCHENGINE_EXPORT
#endif

#ifdef __cplusplus
#define STITCHENGINE_EXTERN extern "C" STITCHENGINE_EXPORT
#else
#define STITCHENGINE_EXTERN STITCHENGINE_EXPORT
#endif

#endif // STITCHENGINE_EXPORT_H
