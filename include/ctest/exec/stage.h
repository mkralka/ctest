#ifndef CTEST__EXEC__STAGE_H__INCLUDED__
#define CTEST__EXEC__STAGE_H__INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The stages of execution.
 */
typedef enum ctest_stage ctest_stage_t;
enum ctest_stage {
	CTEST_STAGE_SETUP,
	CTEST_STAGE_EXECUTION,
	CTEST_STAGE_TEARDOWN,
};

#ifdef __cplusplus
}
#endif

#endif /* CTEST__EXEC__STAGE_H__INCLUDED__ */
