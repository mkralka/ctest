#ifndef PRIVATE__SERIALIZATION_H__INCLUDED__
#define PRIVATE__SERIALIZATION_H__INCLUDED__

#include <stdbool.h>
#include <stddef.h>

/**
 * Pad the size of a buffer so that it is rounded up to the nearest whole
 * object of the specified alignment.
 *
 * This can be used to add padding bytes to a buffer so that the next item in
 * the buffer is on an aligned boundary. This assumes that the beginning of the
 * buffer is already aligned to the specified alignment.
 *
 * For example, if the buffer is currently 11 bytes and the alignment is 8, then
 * the result will be 16.
 *
 * @param size      The current size of the buffer.
 * @param alignment The alignment out to which the buffer should be padded.
 *
 * @return <code>size</code> padded to <code>alignment</code>.
 */
static inline size_t serialize_pad_size(size_t size, size_t alignment)
{
	return ((size + alignment - 1) / alignment) * alignment;
}

/**
 * Create a relative pointer from an absolute pointer.
 *
 * A relative pointer one whose value adjusted if <code>base</code> had a value
 * of 0 (meaning, it indicates the number of bytes from <code>base</code> that
 * the memory location referenced by <code>ptr</code> can be found.
 *
 * This is useful for serializing objects with pointers to internal fields
 * within the object.
 *
 * The result of this can be undone using
 * <code>seriallize_abs_ptr_from_rel</code>.
 *
 * @param ptr  The absolute pointer to make relative.
 * @param base The base address to which the returned pointer is relative.
 *
 * @return A pointer whose value is equivalent to <code>ptr</code>, but relative
 *         to <code>base</code>.
 */
static inline void *serialize_rel_ptr_from_abs(const void *ptr, const void *base)
{
	return (void *)(ptr - base);
}

/**
 * Create an absolute pointer from a relative pointer.
 *
 * This undoes the transformation performed by
 * <code>serialize_rel_ptr_from_abs</code>.
 *
 * @param ptr  The relative pointer to make absolute.
 * @param base The base address to which <code>ptr</code> is relative.
 *
 * @return A pointer whose value is equivalent to <code>ptr</code> (relative
 *         to <code>base</code>), but absolute.
 */
static inline void *serialize_abs_ptr_from_rel(const void *ptr, const void *base)
{
	return ((void *)base) + (ptrdiff_t)ptr;
}

/**
 * Determine if the memory <code>ptr_len</code> bytes long starting at
 * <code>ptr</code> is completely contained within the memory <code>len</code>
 * byte long starting at <code>base</code>.
 *
 * @param ptr     The base address of the memory block being checked if it is
 *                contained within the memory block beginning at
 *                <code>base</code>.
 * @param ptr_len The size of the memory block beginning at <code>ptr</code>.
 * @param base    The base address of the memory block in which the memory block
 *                beginning at <code>ptr</code> is being checked.
 * @param len     The size of the memory block beginning at <code>base</code>.
 *
 * @return <code>true</code> if the memory block denoted by <code>ptr</code> and
 *         <code>ptr_len</code> are completely contained within the memory block
 *         denoted by <code>base</code> and <code>len</code>.
 */
static inline bool serialize_ptr_in_range(const void *ptr, size_t ptr_len, const void *base, size_t len)
{
	return ptr >= base && (ptr + ptr_len) <= (base + len);
}

#endif /* PRIVATE__SERIALIZATION_H__INCLUDED__ */
