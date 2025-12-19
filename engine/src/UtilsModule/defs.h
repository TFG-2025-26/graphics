#pragma once

#ifndef DEFS_H_
#define DEFS_H_

#ifdef FLUX_EXPORTS
#define FLUX_API __declspec(dllexport)
#else
#define FLUX_API __declspec(dllimport)
#endif

#endif // DEFS_H_
