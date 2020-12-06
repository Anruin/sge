/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Evan Teran
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <assert.h> /* for assert */
#include <stdlib.h> /* for malloc/realloc/free */

/**
 * @brief FVector_Declare - The vector type used in this library
 */
#define FVector(type) type *

/**
 * @brief FVector_SetCapacity - For internal use, sets the capacity variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define FVector_SetCapacity(vec, size)     \
	do {                                    \
		if (vec) {                          \
			((size_t *)(vec))[-1] = (size); \
		}                                   \
	} while (0)

/**
 * @brief FVector_SetSize - For internal use, sets the size variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define FVector_SetSize(vec, size)         \
	do {                                    \
		if (vec) {                          \
			((size_t *)(vec))[-2] = (size); \
		}                                   \
	} while (0)

/**
 * @brief FVector_GetCapacity - gets the current capacity of the vector
 * @param vec - the vector
 * @return the capacity as a size_t
 */
#define FVector_GetCapacity(vec) \
	((vec) ? ((size_t *)(vec))[-1] : (size_t)0)

/**
 * @brief FVector_GetSize - gets the current size of the vector
 * @param vec - the vector
 * @return the size as a size_t
 */
#define FVector_GetSize(vec) \
	((vec) ? ((size_t *)(vec))[-2] : (size_t)0)

/**
 * @brief FVector_IsEmpty - returns non-zero if the vector is empty
 * @param vec - the vector
 * @return non-zero if empty, zero if non-empty
 */
#define FVector_IsEmpty(vec) \
	(FVector_GetSize(vec) == 0)

/**
 * @brief FVector_Grow - For internal use, ensures that the vector is at least <count> elements big
 * @param vec - the vector
 * @param count - the new capacity to set
 * @return void
 */
#define FVector_Grow(vec, count)                                              \
	do {                                                                      \
		const size_t cv_sz = (count) * sizeof(*(vec)) + (sizeof(size_t) * 2); \
		if (!(vec)) {                                                         \
			size_t *cv_p = malloc(cv_sz);                                     \
			assert(cv_p);                                                     \
			(vec) = (void *)(&cv_p[2]);                                       \
			FVector_SetCapacity((vec), (count));                             \
			FVector_SetSize((vec), 0);                                       \
		} else {                                                              \
			size_t *cv_p1 = &((size_t *)(vec))[-2];                           \
			size_t *cv_p2 = realloc(cv_p1, (cv_sz));                          \
			assert(cv_p2);                                                    \
			(vec) = (void *)(&cv_p2[2]);                                      \
			FVector_SetCapacity((vec), (count));                             \
		}                                                                     \
	} while (0)

/**
 * @brief FVector_PopBack - removes the last element from the vector
 * @param vec - the vector
 * @return void
 */
#define FVector_PopBack(vec)                           \
	do {                                                \
		FVector_SetSize((vec), FVector_GetSize(vec) - 1); \
	} while (0)

/**
 * @brief FVector_erase - removes the element at index i from the vector
 * @param vec - the vector
 * @param i - index of element to remove
 * @return void
 */
#define FVector_RemoveAt(vec, i)                                  \
	do {                                                       \
		if (vec) {                                             \
			const size_t cv_sz = FVector_GetSize(vec);            \
			if ((i) < cv_sz) {                                 \
				FVector_SetSize((vec), cv_sz - 1);            \
				size_t cv_x;                                   \
				for (cv_x = (i); cv_x < (cv_sz - 1); ++cv_x) { \
					(vec)[cv_x] = (vec)[cv_x + 1];             \
				}                                              \
			}                                                  \
		}                                                      \
	} while (0)

/**
 * @brief FVector_Free - frees all memory associated with the vector
 * @param vec - the vector
 * @return void
 */
#define FVector_Free(vec)                        \
	do {                                         \
		if (vec) {                               \
			size_t *p1 = &((size_t *)(vec))[-2]; \
			free(p1);                            \
		}                                        \
	} while (0)

/**
 * @brief FVector_Begin - returns an iterator to first element of the vector
 * @param vec - the vector
 * @return a pointer to the first element (or NULL)
 */
#define FVector_Begin(vec) \
	(vec)

/**
 * @brief FVector_End - returns an iterator to one past the last element of the vector
 * @param vec - the vector
 * @return a pointer to one past the last element (or NULL)
 */
#define FVector_End(vec) \
	((vec) ? &((vec)[FVector_GetSize(vec)]) : NULL)

/* user request to use logarithmic growth algorithm */
#ifdef FVector_LOGARITHMICGROWTH

/**
 * @brief FVector_Add - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#define FVector_Add(vec, value)                               \
	do {                                                            \
		size_t cv_cap = FVector_GetCapacity(vec);                      \
		if (cv_cap <= FVector_GetSize(vec)) {                          \
			FVector_Grow((vec), !cv_cap ? cv_cap + 1 : cv_cap * 2); \
		}                                                           \
		vec[FVector_GetSize(vec)] = (value);                           \
		FVector_SetSize((vec), FVector_GetSize(vec) + 1);             \
	} while (0)

#else

/**
 * @brief FVector_Add - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#define FVector_Add(vec, value)                   \
	do {                                                \
		size_t cv_cap = FVector_GetCapacity(vec);          \
		if (cv_cap <= FVector_GetSize(vec)) {              \
			FVector_Grow((vec), cv_cap + 1);            \
		}                                               \
		vec[FVector_GetSize(vec)] = (value);               \
		FVector_SetSize((vec), FVector_GetSize(vec) + 1); \
	} while (0)

#endif /* FVector_LOGARITHMICGROWTH */

/**
 * @brief FVector_Copy - copy a vector
 * @param from - the original vector
 * @param to - destination to which the function copy to
 * @return void
 */
#define FVector_Copy(from, to)									\
	do {														\
		for(size_t i = 0; i < FVector_GetSize(from); i++) {		\
			FVector_Add(to, from[i]);						\
		}														\
	} while (0)
