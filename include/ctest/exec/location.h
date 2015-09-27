#ifndef CTEST__EXEC__LOCATION_H__INCLUDED__
#define CTEST__EXEC__LOCATION_H__INCLUDED__

/**
 * The location with a test.
 */
typedef struct ctest_location ctest_location_t;
struct ctest_location {

	/**
	 * The name of the (source) file.
	 */
	const char *filename;

	/**
	 * The line number, within <code>filename</code>.
	 */
	int line;
};

#endif /* CTEST__EXEC__LOCATION_H__INCLUDED__ */

